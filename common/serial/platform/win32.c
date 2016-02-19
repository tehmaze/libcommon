#include "common/platform.h"
#include "common/debug.h"
#include "common/serial.h"

#include <usbioctl.h>
#include <winnls.h>
#include <talloc.h>
#include <string.h>

#if !defined(WC_NO_BEST_FIT_CHARS)
#define WC_NO_BEST_FIT_CHARS      0x00000400  // do not use best fit chars
#endif

/* The stuff below is not yet available in MinGW apparently. Define it here. */

#ifndef USB_NODE_CONNECTION_INFORMATION_EX
PRIVATE typedef struct _USB_NODE_CONNECTION_INFORMATION_EX {
       ULONG ConnectionIndex;
       USB_DEVICE_DESCRIPTOR DeviceDescriptor;
       UCHAR CurrentConfigurationValue;
       UCHAR Speed;
       BOOLEAN DeviceIsHub;
       USHORT DeviceAddress;
       ULONG NumberOfOpenPipes;
       USB_CONNECTION_STATUS ConnectionStatus;
       USB_PIPE_INFO PipeList[];
} USB_NODE_CONNECTION_INFORMATION_EX, *PUSB_NODE_CONNECTION_INFORMATION_EX;
#endif

#ifndef USB_GET_NODE_CONNECTION_INFORMATION_EX
#define USB_GET_NODE_CONNECTION_INFORMATION_EX 274
#endif

#ifndef IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX
#define IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX \
       CTL_CODE(FILE_DEVICE_USB, USB_GET_NODE_CONNECTION_INFORMATION_EX, \
       METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

#ifndef CM_DRP_COMPATIBLEIDS
#define CM_DRP_COMPATIBLEIDS 0x03
#endif
#ifndef CM_DRP_CLASS
#define CM_DRP_CLASS 0x08
#endif
#ifndef CM_DRP_FRIENDLYNAME
#define CM_DRP_FRIENDLYNAME 0x0d
#endif
#ifndef CM_DRP_ADDRESS
#define CM_DRP_ADDRESS 0x1d
#endif

#ifndef CM_Get_DevNode_Registry_PropertyA
CMAPI CONFIGRET WINAPI CM_Get_DevNode_Registry_PropertyA(DEVINST dnDevInst, \
       ULONG ulProperty, PULONG pulRegDataType, PVOID Buffer, \
       PULONG pulLength, ULONG ulFlags);
#endif


/* USB path is a string of at most 8 decimal numbers < 128 separated by dots. */
#define MAX_USB_PATH ((8 * 3) + (7 * 1) + 1)

PRIVATE static void enumerate_hub(serial_t *port, const char *hub_name,
                          const char *parent_path, DEVINST dev_inst);

PRIVATE static char *wc_to_utf8(PWCHAR wc_buffer, ULONG size)
{
	WCHAR wc_str[(size / sizeof(WCHAR)) + 1];
	char *utf8_str;

	/* Zero-terminate the wide char string. */
	memcpy(wc_str, wc_buffer, size);
	wc_str[sizeof(wc_str) - 1] = 0;

	/* Compute the size of the UTF-8 converted string. */
	if (!(size = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wc_str, -1,
	                                 NULL, 0, NULL, NULL)))
		return NULL;

	/* Allocate UTF-8 output buffer. */
	if (!(utf8_str = talloc_size(NULL, size)))
		return NULL;

	/* Actually converted to UTF-8. */
	if (!WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wc_str, -1,
	                         utf8_str, size, NULL, NULL)) {
		TALLOC_FREE(utf8_str);
		return NULL;
	}

	return utf8_str;
}

PRIVATE static char *get_root_hub_name(HANDLE host_controller)
{
	USB_ROOT_HUB_NAME root_hub_name;
	PUSB_ROOT_HUB_NAME root_hub_name_wc;
	char *root_hub_name_utf8;
	ULONG size = 0;

	/* Compute the size of the root hub name string. */
	if (!DeviceIoControl(host_controller, IOCTL_USB_GET_ROOT_HUB_NAME, 0, 0,
	                     &root_hub_name, sizeof(root_hub_name), &size, NULL))
		return NULL;

	/* Allocate wide char root hub name string. */
	size = root_hub_name.ActualLength;
	if (!(root_hub_name_wc = talloc_size(NULL, size)))
		return NULL;

	/* Actually get the root hub name string. */
	if (!DeviceIoControl(host_controller, IOCTL_USB_GET_ROOT_HUB_NAME,
	                     NULL, 0, root_hub_name_wc, size, &size, NULL)) {
		TALLOC_FREE(root_hub_name_wc);
		return NULL;
	}

	/* Convert the root hub name string to UTF-8. */
	root_hub_name_utf8 = wc_to_utf8(root_hub_name_wc->RootHubName, size);
	TALLOC_FREE(root_hub_name_wc);
	return root_hub_name_utf8;
}

PRIVATE static char *get_external_hub_name(HANDLE hub, ULONG connection_index)
{
	USB_NODE_CONNECTION_NAME ext_hub_name;
	PUSB_NODE_CONNECTION_NAME ext_hub_name_wc;
	char *ext_hub_name_utf8;
	ULONG size;

	/* Compute the size of the external hub name string. */
	ext_hub_name.ConnectionIndex = connection_index;
	if (!DeviceIoControl(hub, IOCTL_USB_GET_NODE_CONNECTION_NAME,
	                     &ext_hub_name, sizeof(ext_hub_name),
	                     &ext_hub_name, sizeof(ext_hub_name), &size, NULL))
		return NULL;

	/* Allocate wide char external hub name string. */
	size = ext_hub_name.ActualLength;
	if (size <= sizeof(ext_hub_name)
	    || !(ext_hub_name_wc = talloc_size(NULL, size)))
		return NULL;

	/* Get the name of the external hub attached to the specified port. */
	ext_hub_name_wc->ConnectionIndex = connection_index;
	if (!DeviceIoControl(hub, IOCTL_USB_GET_NODE_CONNECTION_NAME,
	                     ext_hub_name_wc, size,
	                     ext_hub_name_wc, size, &size, NULL)) {
		TALLOC_FREE(ext_hub_name_wc);
		return NULL;
	}

	/* Convert the external hub name string to UTF-8. */
	ext_hub_name_utf8 = wc_to_utf8(ext_hub_name_wc->NodeName, size);
	TALLOC_FREE(ext_hub_name_wc);
	return ext_hub_name_utf8;
}

PRIVATE static char *get_string_descriptor(HANDLE hub_device, ULONG connection_index,
                                   UCHAR descriptor_index)
{
	char desc_req_buf[sizeof(USB_DESCRIPTOR_REQUEST) +
	                  MAXIMUM_USB_STRING_LENGTH] = { 0 };
	PUSB_DESCRIPTOR_REQUEST desc_req = (void *)desc_req_buf;
	PUSB_STRING_DESCRIPTOR desc = (void *)(desc_req + 1);
	ULONG size = sizeof(desc_req_buf);

	desc_req->ConnectionIndex = connection_index;
	desc_req->SetupPacket.wValue = (USB_STRING_DESCRIPTOR_TYPE << 8)
	                               | descriptor_index;
	desc_req->SetupPacket.wIndex = 0;
	desc_req->SetupPacket.wLength = size - sizeof(*desc_req);

	if (!DeviceIoControl(hub_device,
	                     IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
	                     desc_req, size, desc_req, size, &size, NULL)
	    || size < 2
	    || desc->bDescriptorType != USB_STRING_DESCRIPTOR_TYPE
	    || desc->bLength != size - sizeof(*desc_req)
	    || desc->bLength % 2)
		return NULL;

	return wc_to_utf8(desc->bString, desc->bLength);
}

PRIVATE static void enumerate_hub_ports(serial_t *port, HANDLE hub_device,
                                ULONG nb_ports, const char *parent_path, DEVINST dev_inst)
{
	char path[MAX_USB_PATH];
	ULONG index = 0;

	for (index = 1; index <= nb_ports; index++) {
		PUSB_NODE_CONNECTION_INFORMATION_EX connection_info_ex;
		ULONG size = sizeof(*connection_info_ex) + (30 * sizeof(USB_PIPE_INFO));

		if (!(connection_info_ex = talloc_size(port, size)))
			break;

		connection_info_ex->ConnectionIndex = index;
		if (!DeviceIoControl(hub_device,
		                     IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX,
		                     connection_info_ex, size,
		                     connection_info_ex, size, &size, NULL)) {
			/*
			 * Try to get CONNECTION_INFORMATION if
			 * CONNECTION_INFORMATION_EX did not work.
			 */
			PUSB_NODE_CONNECTION_INFORMATION connection_info;

			size = sizeof(*connection_info) + (30 * sizeof(USB_PIPE_INFO));
			if (!(connection_info = talloc_size(port, size))) {
				TALLOC_FREE(connection_info_ex);
				continue;
			}
			connection_info->ConnectionIndex = index;
			if (!DeviceIoControl(hub_device,
			                     IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
			                     connection_info, size,
			                     connection_info, size, &size, NULL)) {
				TALLOC_FREE(connection_info);
				TALLOC_FREE(connection_info_ex);
				continue;
			}

			connection_info_ex->ConnectionIndex = connection_info->ConnectionIndex;
			connection_info_ex->DeviceDescriptor = connection_info->DeviceDescriptor;
			connection_info_ex->DeviceIsHub = connection_info->DeviceIsHub;
			connection_info_ex->DeviceAddress = connection_info->DeviceAddress;
			TALLOC_FREE(connection_info);
		}

		if (connection_info_ex->DeviceIsHub) {
			/* Recursively enumerate external hub. */
			PCHAR ext_hub_name;
			if ((ext_hub_name = get_external_hub_name(hub_device, index))) {
				snprintf(path, sizeof(path), "%s%ld.",
				         parent_path, connection_info_ex->ConnectionIndex);
				enumerate_hub(port, ext_hub_name, path, dev_inst);
			}
			free(connection_info_ex);
		} else {
			snprintf(path, sizeof(path), "%s%ld",
			         parent_path, connection_info_ex->ConnectionIndex);

			/* Check if this device is the one we search for. */
			if (strncmp(path, port->usb_path, MAX_USB_PATH)) {
				TALLOC_FREE(connection_info_ex);
				continue;
			}

			/* Finally grab detailed information regarding the device. */
			port->usb_address = connection_info_ex->DeviceAddress + 1;
			port->usb_vid = connection_info_ex->DeviceDescriptor.idVendor;
			port->usb_pid = connection_info_ex->DeviceDescriptor.idProduct;

			if (connection_info_ex->DeviceDescriptor.iManufacturer)
				port->usb_manufacturer = get_string_descriptor(hub_device,index,
				           connection_info_ex->DeviceDescriptor.iManufacturer);
			if (connection_info_ex->DeviceDescriptor.iProduct)
				port->usb_product = get_string_descriptor(hub_device, index,
				           connection_info_ex->DeviceDescriptor.iProduct);
			if (connection_info_ex->DeviceDescriptor.iSerialNumber) {
				port->usb_serial = get_string_descriptor(hub_device, index,
				           connection_info_ex->DeviceDescriptor.iSerialNumber);
				if (port->usb_serial == NULL) {
					//composite device, get the parent's serial number
					char device_id[MAX_DEVICE_ID_LEN];
					if (CM_Get_Parent(&dev_inst, dev_inst, 0) == CR_SUCCESS) {
						if (CM_Get_Device_IDA(dev_inst, device_id, sizeof(device_id), 0) == CR_SUCCESS)
							port->usb_serial = strdup(strrchr(device_id, '\\')+1);
					}
				}
			}

			TALLOC_FREE(connection_info_ex);
			break;
		}
	}
}

PRIVATE static void enumerate_hub(serial_t *port, const char *hub_name,
                          const char *parent_path, DEVINST dev_inst)
{
	USB_NODE_INFORMATION hub_info;
	HANDLE hub_device;
	ULONG size = sizeof(hub_info);
	char *device_name;

	/* Open the hub with its full name. */
	if (!(device_name = talloc_size(port, strlen("\\\\.\\") + strlen(hub_name) + 1)))
		return;
	strcpy(device_name, "\\\\.\\");
	strcat(device_name, hub_name);
	hub_device = CreateFile(device_name, GENERIC_WRITE, FILE_SHARE_WRITE,
	                        NULL, OPEN_EXISTING, 0, NULL);
	TALLOC_FREE(device_name);
	if (hub_device == INVALID_HANDLE_VALUE)
		return;

	/* Get the number of ports of the hub. */
	if (DeviceIoControl(hub_device, IOCTL_USB_GET_NODE_INFORMATION,
	                    &hub_info, size, &hub_info, size, &size, NULL))
		/* Enumerate the ports of the hub. */
		enumerate_hub_ports(port, hub_device,
		   hub_info.u.HubInformation.HubDescriptor.bNumberOfPorts, parent_path, dev_inst);

	CloseHandle(hub_device);
}

PRIVATE static void enumerate_host_controller(serial_t *port,
                                      HANDLE host_controller_device,
                                      DEVINST dev_inst)
{
	char *root_hub_name;

	if ((root_hub_name = get_root_hub_name(host_controller_device))) {
		enumerate_hub(port, root_hub_name, "", dev_inst);
		TALLOC_FREE(root_hub_name);
	}
}

PRIVATE static void get_usb_details(serial_t *port, DEVINST dev_inst_match)
{
	HDEVINFO device_info;
	SP_DEVINFO_DATA device_info_data;
	ULONG i, size = 0;

	device_info = SetupDiGetClassDevs(&GUID_CLASS_USB_HOST_CONTROLLER, NULL, NULL,
	                                  DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	device_info_data.cbSize = sizeof(device_info_data);

	for (i = 0; SetupDiEnumDeviceInfo(device_info, i, &device_info_data); i++) {
		SP_DEVICE_INTERFACE_DATA device_interface_data;
		PSP_DEVICE_INTERFACE_DETAIL_DATA device_detail_data;
		DEVINST dev_inst = dev_inst_match;
		HANDLE host_controller_device;

		device_interface_data.cbSize = sizeof(device_interface_data);
		if (!SetupDiEnumDeviceInterfaces(device_info, 0,
		                                 &GUID_CLASS_USB_HOST_CONTROLLER,
		                                 i, &device_interface_data))
			continue;

		if (!SetupDiGetDeviceInterfaceDetail(device_info,&device_interface_data,
		                                     NULL, 0, &size, NULL)
		    && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			continue;

		if (!(device_detail_data = talloc_size(port, size)))
			continue;
		device_detail_data->cbSize = sizeof(*device_detail_data);
		if (!SetupDiGetDeviceInterfaceDetail(device_info,&device_interface_data,
		                                     device_detail_data, size, &size,
		                                     NULL)) {
			TALLOC_FREE(device_detail_data);
			continue;
		}

		while (CM_Get_Parent(&dev_inst, dev_inst, 0) == CR_SUCCESS
		       && dev_inst != device_info_data.DevInst) { }
		if (dev_inst != device_info_data.DevInst) {
			TALLOC_FREE(device_detail_data);
			continue;
		}

		port->usb_bus = i + 1;

		host_controller_device = CreateFile(device_detail_data->DevicePath,
		                                    GENERIC_WRITE, FILE_SHARE_WRITE,
		                                    NULL, OPEN_EXISTING, 0, NULL);
		if (host_controller_device != INVALID_HANDLE_VALUE) {
			enumerate_host_controller(port, host_controller_device, dev_inst_match);
			CloseHandle(host_controller_device);
		}
		TALLOC_FREE(device_detail_data);
	}

	SetupDiDestroyDeviceInfoList(device_info);
	return;
}

PRIVATE int serial_details(serial_t *port)
{
	/*
	 * Description limited to 127 char, anything longer
	 * would not be user friendly anyway.
	 */
	char description[128];
	SP_DEVINFO_DATA device_info_data = { .cbSize = sizeof(device_info_data) };
	HDEVINFO device_info;
	int i;

	device_info = SetupDiGetClassDevs(NULL, 0, 0,
	                                  DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if (device_info == INVALID_HANDLE_VALUE)
		RETURN_FAIL("SetupDiGetClassDevs() failed");

	for (i = 0; SetupDiEnumDeviceInfo(device_info, i, &device_info_data); i++) {
		HKEY device_key;
		DEVINST dev_inst;
		char value[8], class[16];
		DWORD size, type;
		CONFIGRET cr;

		/* Check if this is the device we are looking for. */
		device_key = SetupDiOpenDevRegKey(device_info, &device_info_data,
		                                  DICS_FLAG_GLOBAL, 0,
		                                  DIREG_DEV, KEY_QUERY_VALUE);
		if (device_key == INVALID_HANDLE_VALUE)
			continue;
		size = sizeof(value);
		if (RegQueryValueExA(device_key, "PortName", NULL, &type, (LPBYTE)value,
		                     &size) != ERROR_SUCCESS || type != REG_SZ) {
			RegCloseKey(device_key);
			continue;
		}
		RegCloseKey(device_key);
		value[sizeof(value)-1] = 0;
		if (strncmp(value, port->name, 8))
			continue;

		/* Check port transport type. */
		dev_inst = device_info_data.DevInst;
		size = sizeof(class);
		cr = CR_FAILURE;
		while (CM_Get_Parent(&dev_inst, dev_inst, 0) == CR_SUCCESS &&
		       (cr = CM_Get_DevNode_Registry_PropertyA(dev_inst,
		                 CM_DRP_CLASS, 0, class, &size, 0)) != CR_SUCCESS) { }
		if (cr == CR_SUCCESS) {
			if (!strncmp(class, "USB", 3))
				port->transport = SERIAL_TRANSPORT_USB;
		}

		/* Get port description (friendly name). */
		dev_inst = device_info_data.DevInst;
		size = sizeof(description);
		while ((cr = CM_Get_DevNode_Registry_PropertyA(dev_inst,
		          CM_DRP_FRIENDLYNAME, 0, description, &size, 0)) != CR_SUCCESS
		       && CM_Get_Parent(&dev_inst, dev_inst, 0) == CR_SUCCESS) { }
		if (cr == CR_SUCCESS)
			port->description = talloc_strdup(port, description);

		/* Get more informations for USB connected ports. */
		if (port->transport == SERIAL_TRANSPORT_USB) {
			char usb_path[MAX_USB_PATH] = "", tmp[MAX_USB_PATH];
			char device_id[MAX_DEVICE_ID_LEN];

			/* Recurse over parents to build the USB device path. */
			dev_inst = device_info_data.DevInst;
			do {
				/* Verify that this layer of the tree is USB related. */
				if (CM_Get_Device_IDA(dev_inst, device_id,
				                      sizeof(device_id), 0) != CR_SUCCESS
				    || strncmp(device_id, "USB\\", 4))
					continue;

				/* Discard one layer for composite devices. */
				char compat_ids[512], *p = compat_ids;
				size = sizeof(compat_ids);
				if (CM_Get_DevNode_Registry_PropertyA(dev_inst,
				                                      CM_DRP_COMPATIBLEIDS, 0,
				                                      &compat_ids,
				                                      &size, 0) == CR_SUCCESS) {
					while (*p) {
						if (!strncmp(p, "USB\\COMPOSITE", 13))
							break;
						p += strlen(p) + 1;
					}
					if (*p)
						continue;
				}

				/* Stop the recursion when reaching the USB root. */
				if (!strncmp(device_id, "USB\\ROOT", 8))
					break;

				/* Prepend the address of current USB layer to the USB path. */
				DWORD address;
				size = sizeof(address);
				if (CM_Get_DevNode_Registry_PropertyA(dev_inst, CM_DRP_ADDRESS,
				                        0, &address, &size, 0) == CR_SUCCESS) {
					strncpy(tmp, usb_path, MAX_USB_PATH);
					snprintf(usb_path, sizeof(usb_path), "%d%s%s",
					         (int)address, *tmp ? "." : "", tmp);
				}
			} while (CM_Get_Parent(&dev_inst, dev_inst, 0) == CR_SUCCESS);

			port->usb_path = talloc_strdup(port, usb_path);

			/* Wake up the USB device to be able to read string descriptor. */
			char *escaped_port_name;
			HANDLE handle;
			if (!(escaped_port_name = talloc_size(NULL, strlen(port->name) + 5)))
				RETURN_ERROR(ENOMEM, "escaped port name malloc failed");
			sprintf(escaped_port_name, "\\\\.\\%s", port->name);
			handle = CreateFile(escaped_port_name, GENERIC_READ, 0, 0,
			                    OPEN_EXISTING,
			                    FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, 0);
			TALLOC_FREE(escaped_port_name);
			CloseHandle(handle);

			/* Retrieve USB device details from the device descriptor. */
			get_usb_details(port, device_info_data.DevInst);
		}
		break;
	}

	SetupDiDestroyDeviceInfoList(device_info);

	RETURN_OK();
}

PRIVATE int serial_list(serial_t ***list)
{
	HKEY key;
	TCHAR *value, *data;
	DWORD max_value_len, max_data_size, max_data_len;
	DWORD value_len, data_size, data_len;
	DWORD type, index = 0;
	char *name;
	int name_len;
	int ret = 0;

	DEBUG("opening registry key");
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
			0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
		SET_FAIL(ret, "RegOpenKeyEx() failed");
		goto out_done;
	}
	DEBUG("querying registry key value and data sizes");
	if (RegQueryInfoKey(key, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				&max_value_len, &max_data_size, NULL, NULL) != ERROR_SUCCESS) {
		SET_FAIL(ret, "RegQueryInfoKey() failed");
		goto out_close;
	}
	max_data_len = max_data_size / sizeof(TCHAR);
	if (!(value = talloc_size(NULL, (max_value_len + 1) * sizeof(TCHAR)))) {
		SET_ERROR(ret, ENOMEM, "registry value malloc failed");
		goto out_close;
	}
	if (!(data = talloc_size(NULL, (max_data_len + 1) * sizeof(TCHAR)))) {
		SET_ERROR(ret, ENOMEM, "registry data malloc failed");
		goto out_free_value;
	}
	DEBUG("iterating over values");
	while (
		value_len = max_value_len + 1,
		data_size = max_data_size,
		RegEnumValue(key, index, value, &value_len,
			NULL, &type, (LPBYTE)data, &data_size) == ERROR_SUCCESS)
	{
		if (type == REG_SZ) {
			data_len = data_size / sizeof(TCHAR);
			data[data_len] = '\0';
#ifdef UNICODE
			name_len = WideCharToMultiByte(CP_ACP, 0, data, -1, NULL, 0, NULL, NULL);
#else
			name_len = data_len + 1;
#endif
			if (!(name = talloc_size(NULL, name_len))) {
				SET_ERROR(ret, ENOMEM, "registry port name malloc failed");
				goto out;
			}
#ifdef UNICODE
			WideCharToMultiByte(CP_ACP, 0, data, -1, name, name_len, NULL, NULL);
#else
			strcpy(name, data);
#endif
			DEBUGF("found port %s", name);
			if (!(*list = serial_list_append(*list, name))) {
				SET_ERROR(ret, ENOMEM, "list append failed");
				TALLOC_FREE(name);
				goto out;
			}
			TALLOC_FREE(name);
		}
		index++;
	}
out:
	TALLOC_FREE(data);
out_free_value:
	TALLOC_FREE(value);
out_close:
	RegCloseKey(key);
out_done:

	return ret;
}

/*
 * public domain strtok_r() by Charlie Gordon
 *
 *   from comp.lang.c  9/14/2007
 *
 *      http://groups.google.com/group/comp.lang.c/msg/2ab1ecbb86646684
 *
 *     (Declaration that it's public domain):
 *      http://groups.google.com/group/comp.lang.c/msg/7c7b39328fefab9c
 */

char* strtok_r(char *str, const char *delim, char **nextp)
{
    char *ret;

    if (str == NULL)
    {
        str = *nextp;
    }

    str += strspn(str, delim);

    if (*str == '\0')
    {
        return NULL;
    }

    ret = str;

    str += strcspn(str, delim);

    if (*str)
    {
        *str++ = '\0';
    }

    *nextp = str;

    return ret;
}
