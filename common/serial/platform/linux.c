#include <ctype.h>
#include "common/config.h"
#include "common/platform.h"
#include "common/serial.h"
#include "common/debug.h"

PRIVATE int serial_details(serial_t *port)
{
    char description[128];
	int bus, address;
	unsigned int vid, pid;
	char manufacturer[128], product[128], serial[128];
	char baddr[32];
	const char dir_name[] = "/sys/class/tty/%s/device/%s%s";
	char sub_dir[32] = "", file_name[PATH_MAX];
	char *ptr, *dev = port->name + 5;
	FILE *file;
	int i, j, count, len;
	struct stat statbuf;

    if (strncmp(port->name, "/dev/", 5)) {
		errno = EINVAL;
        return -1;
    }

    snprintf(file_name, sizeof(file_name), "/sys/class/tty/%s", dev);
	if (lstat(file_name, &statbuf) == -1) {
        errno = EADDRNOTAVAIL;
        return -1;
    }

    if (!S_ISLNK(statbuf.st_mode)) {
        snprintf(file_name, sizeof(file_name), "/sys/class/tty/%s/device", dev);
    }
    count = readlink(file_name, file_name, sizeof(file_name));
    if (count <= 0 || count >= (int)(sizeof(file_name) - 1)) {
        errno = EADDRNOTAVAIL;
        return -1;
    }

    file_name[count] = 0;
	if (strstr(file_name, "bluetooth")) {
		port->transport = SERIAL_TRANSPORT_BLUETOOTH;
	} else if (strstr(file_name, "usb")) {
		port->transport = SERIAL_TRANSPORT_USB;
    }

	if (port->transport == SERIAL_TRANSPORT_USB) {
		for (i = 0; i < 5; i++) {
			strcat(sub_dir, "../");

			snprintf(file_name, sizeof(file_name), dir_name, dev, sub_dir, "busnum");
			if (!(file = fopen(file_name, "r")))
				continue;
			count = fscanf(file, "%d", &bus);
			fclose(file);
			if (count != 1)
				continue;

			snprintf(file_name, sizeof(file_name), dir_name, dev, sub_dir, "devnum");
			if (!(file = fopen(file_name, "r")))
				continue;
			count = fscanf(file, "%d", &address);
			fclose(file);
			if (count != 1)
				continue;

			snprintf(file_name, sizeof(file_name), dir_name, dev, sub_dir, "idVendor");
			if (!(file = fopen(file_name, "r")))
				continue;
			count = fscanf(file, "%4x", &vid);
			fclose(file);
			if (count != 1)
				continue;

			snprintf(file_name, sizeof(file_name), dir_name, dev, sub_dir, "idProduct");
			if (!(file = fopen(file_name, "r")))
				continue;
			count = fscanf(file, "%4x", &pid);
			fclose(file);
			if (count != 1)
				continue;

			port->usb_bus = bus;
			port->usb_address = address;
			port->usb_vid = vid;
			port->usb_pid = pid;

			snprintf(file_name, sizeof(file_name), dir_name, dev, sub_dir, "product");
			if ((file = fopen(file_name, "r"))) {
				if ((ptr = fgets(description, sizeof(description), file))) {
					ptr = description + strlen(description) - 1;
					if (ptr >= description && *ptr == '\n')
						*ptr = 0;
					port->description = strdup(description);
				}
				fclose(file);
			}
			if (!file || !ptr)
				port->description = strdup(dev);

			snprintf(file_name, sizeof(file_name), dir_name, dev, sub_dir, "manufacturer");
			if ((file = fopen(file_name, "r"))) {
				if ((ptr = fgets(manufacturer, sizeof(manufacturer), file))) {
					ptr = manufacturer + strlen(manufacturer) - 1;
					if (ptr >= manufacturer && *ptr == '\n')
						*ptr = 0;
					port->usb_manufacturer = strdup(manufacturer);
				}
				fclose(file);
			}

			snprintf(file_name, sizeof(file_name), dir_name, dev, sub_dir, "product");
			if ((file = fopen(file_name, "r"))) {
				if ((ptr = fgets(product, sizeof(product), file))) {
					ptr = product + strlen(product) - 1;
					if (ptr >= product && *ptr == '\n')
						*ptr = 0;
					port->usb_product = strdup(product);
				}
				fclose(file);
			}

			snprintf(file_name, sizeof(file_name), dir_name, dev, sub_dir, "serial");
			if ((file = fopen(file_name, "r"))) {
				if ((ptr = fgets(serial, sizeof(serial), file))) {
					ptr = serial + strlen(serial) - 1;
					if (ptr >= serial && *ptr == '\n')
						*ptr = 0;
					port->usb_serial = strdup(serial);
                    len = strlen(port->usb_serial);
                    for (i = 0; i < len; i++) {
                        port->usb_serial[i] = tolower(port->usb_serial[i]);
                    }
				}
				fclose(file);
			}

			/* If present, add serial to description for better identification. */
			if (port->usb_serial && strlen(port->usb_serial)) {
				snprintf(description, sizeof(description),
					"%s - %s", port->description, port->usb_serial);
				if (port->description)
					free(port->description);
				port->description = strdup(description);
			}

			break;
		}
	} else {
		port->description = strdup(dev);

		if (port->transport == SERIAL_TRANSPORT_BLUETOOTH) {
			snprintf(file_name, sizeof(file_name), dir_name, dev, "", "address");
			if ((file = fopen(file_name, "r"))) {
				if ((ptr = fgets(baddr, sizeof(baddr), file))) {
					ptr = baddr + strlen(baddr) - 1;
					if (ptr >= baddr && *ptr == '\n')
						*ptr = 0;
					port->bluetooth_address = strdup(baddr);
				}
				fclose(file);
			}
		}
	}

    return 0;
}

PRIVATE int serial_list(serial_t ***list)
{
	char name[PATH_MAX], target[PATH_MAX];
	struct dirent entry, *result;
#ifdef HAVE_STRUCT_SERIAL_STRUCT
	struct serial_struct serial_info;
	int ioctl_result;
#endif
	char buf[sizeof(entry.d_name) + 23];
	int len, fd;
	DIR *dir;
	int ret = 0;
	struct stat statbuf;

	DEBUG("Enumerating tty devices");
	if (!(dir = opendir("/sys/class/tty"))) {
		return -1;
    }

	DEBUG("Iterating over results");
	while (!readdir_r(dir, &entry, &result) && result) {
		snprintf(buf, sizeof(buf), "/sys/class/tty/%s", entry.d_name);
		if (lstat(buf, &statbuf) == -1)
			continue;
		if (!S_ISLNK(statbuf.st_mode))
			snprintf(buf, sizeof(buf), "/sys/class/tty/%s/device", entry.d_name);
		len = readlink(buf, target, sizeof(target));
		if (len <= 0 || len >= (int)(sizeof(target) - 1))
			continue;
		target[len] = 0;
		if (strstr(target, "virtual"))
			continue;
		snprintf(name, sizeof(name), "/dev/%s", entry.d_name);
		DEBUGF("found device %s", name);
		if (strstr(target, "serial8250")) {
			/*
			 * The serial8250 driver has a hardcoded number of ports.
			 * The only way to tell which actually exist on a given system
			 * is to try to open them and make an ioctl call.
			 */
			DEBUG("serial8250 device, attempting to open");
			if ((fd = open(name, O_RDWR | O_NONBLOCK | O_NOCTTY)) < 0) {
				DEBUG("Open failed, skipping");
				continue;
			}
#ifdef HAVE_STRUCT_SERIAL_STRUCT
			ioctl_result = ioctl(fd, TIOCGSERIAL, &serial_info);
#endif
			close(fd);
#ifdef HAVE_STRUCT_SERIAL_STRUCT
			if (ioctl_result != 0) {
				DEBUG("ioctl failed, skipping");
				continue;
			}
			if (serial_info.type == PORT_UNKNOWN) {
				DEBUG("Port type is unknown, skipping");
				continue;
			}
#endif
		}
        DEBUGF("found port %s", name);
        if (!(*list = serial_list_append(*list, name))) {
            SET_ERROR(ret, ENOMEM, "list append failed");
            goto out;
        }
	}
	closedir(dir);
out:
	return ret;
}
