#include "common/config.h"
#include "common/platform.h"
#include "common/serial.h"
#include "common/debug.h"

PRIVATE int serial_details(serial_t *port)
{
	/*
	 * Description limited to 127 char, anything longer
	 * would not be user friendly anyway.
	 */
	char description[128];
	int bus, address, vid, pid = -1;
	char manufacturer[128], product[128], serial[128];
	CFMutableDictionaryRef classes;
	io_iterator_t iter;
	io_object_t ioport, ioparent;
	CFTypeRef cf_property, cf_bus, cf_address, cf_vendor, cf_product;
	Boolean result;
	char path[PATH_MAX], class[16];

	DEBUG("getting serial port details");
	if (!(classes = IOServiceMatching(kIOSerialBSDServiceValue)))
		RETURN_ERROR(errno, "IOServiceMatching() failed");

	if (IOServiceGetMatchingServices(kIOMasterPortDefault, classes,
	                                 &iter) != KERN_SUCCESS)
		RETURN_ERROR(errno, "IOServiceGetMatchingServices() failed");

	DEBUG("iterating over results");
	while ((ioport = IOIteratorNext(iter))) {
		if (!(cf_property = IORegistryEntryCreateCFProperty(ioport,
		            CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0))) {
			IOObjectRelease(ioport);
			continue;
		}
		result = CFStringGetCString(cf_property, path, sizeof(path),
		                            kCFStringEncodingASCII);
		CFRelease(cf_property);
		if (!result || strcmp(path, port->name)) {
			IOObjectRelease(ioport);
			continue;
		}
		DEBUGF("found port %s", path);

		IORegistryEntryGetParentEntry(ioport, kIOServicePlane, &ioparent);
		if ((cf_property=IORegistryEntrySearchCFProperty(ioparent,kIOServicePlane,
		           CFSTR("IOClass"), kCFAllocatorDefault,
		           kIORegistryIterateRecursively | kIORegistryIterateParents))) {
			if (CFStringGetCString(cf_property, class, sizeof(class),
			                       kCFStringEncodingASCII) &&
			    strstr(class, "USB")) {
				DEBUG("found USB class device");
				port->transport = SERIAL_TRANSPORT_USB;
			}
			CFRelease(cf_property);
		}
		if ((cf_property=IORegistryEntrySearchCFProperty(ioparent,kIOServicePlane,
		           CFSTR("IOProviderClass"), kCFAllocatorDefault,
		           kIORegistryIterateRecursively | kIORegistryIterateParents))) {
			if (CFStringGetCString(cf_property, class, sizeof(class),
			                       kCFStringEncodingASCII) &&
			    strstr(class, "USB")) {
				DEBUG("found USB class device");
				port->transport = SERIAL_TRANSPORT_USB;
			}
			CFRelease(cf_property);
		}
		IOObjectRelease(ioparent);

		if ((cf_property = IORegistryEntrySearchCFProperty(ioport,kIOServicePlane,
		         CFSTR("USB Interface Name"), kCFAllocatorDefault,
		         kIORegistryIterateRecursively | kIORegistryIterateParents)) ||
		    (cf_property = IORegistryEntrySearchCFProperty(ioport,kIOServicePlane,
		         CFSTR("USB Product Name"), kCFAllocatorDefault,
		         kIORegistryIterateRecursively | kIORegistryIterateParents)) ||
		    (cf_property = IORegistryEntrySearchCFProperty(ioport,kIOServicePlane,
		         CFSTR("Product Name"), kCFAllocatorDefault,
		         kIORegistryIterateRecursively | kIORegistryIterateParents)) ||
		    (cf_property = IORegistryEntryCreateCFProperty(ioport,
		         CFSTR(kIOTTYDeviceKey), kCFAllocatorDefault, 0))) {
			if (CFStringGetCString(cf_property, description, sizeof(description),
			                       kCFStringEncodingASCII)) {
				DEBUGF("found description %s", description);
				port->description = strdup(description);
			}
			CFRelease(cf_property);
		} else {
			DEBUG("no description for this device");
		}

		cf_bus = IORegistryEntrySearchCFProperty(ioport, kIOServicePlane,
		                                         CFSTR("USBBusNumber"),
		                                         kCFAllocatorDefault,
		                                         kIORegistryIterateRecursively
		                                         | kIORegistryIterateParents);
		cf_address = IORegistryEntrySearchCFProperty(ioport, kIOServicePlane,
		                                         CFSTR("USB Address"),
		                                         kCFAllocatorDefault,
		                                         kIORegistryIterateRecursively
		                                         | kIORegistryIterateParents);
		if (cf_bus && cf_address &&
		    CFNumberGetValue(cf_bus    , kCFNumberIntType, &bus) &&
		    CFNumberGetValue(cf_address, kCFNumberIntType, &address)) {
			DEBUGF("found matching USB bus:address %03d:%03d", bus, address);
			port->usb_bus = bus;
			port->usb_address = address;
		}
		if (cf_bus)
			CFRelease(cf_bus);
		if (cf_address)
			CFRelease(cf_address);

		cf_vendor = IORegistryEntrySearchCFProperty(ioport, kIOServicePlane,
		                                         CFSTR("idVendor"),
		                                         kCFAllocatorDefault,
		                                         kIORegistryIterateRecursively
		                                         | kIORegistryIterateParents);
		cf_product = IORegistryEntrySearchCFProperty(ioport, kIOServicePlane,
		                                         CFSTR("idProduct"),
		                                         kCFAllocatorDefault,
		                                         kIORegistryIterateRecursively
		                                         | kIORegistryIterateParents);
		if (cf_vendor && cf_product &&
		    CFNumberGetValue(cf_vendor , kCFNumberIntType, &vid) &&
		    CFNumberGetValue(cf_product, kCFNumberIntType, &pid)) {
			DEBUGF("found matching USB VID:PID %04X:%04X", vid, pid);
			port->usb_vid = vid;
			port->usb_pid = pid;
		}
		if (cf_vendor)
			CFRelease(cf_vendor);
		if (cf_product)
			CFRelease(cf_product);

		if ((cf_property = IORegistryEntrySearchCFProperty(ioport,kIOServicePlane,
		         CFSTR("USB Vendor Name"), kCFAllocatorDefault,
		         kIORegistryIterateRecursively | kIORegistryIterateParents))) {
			if (CFStringGetCString(cf_property, manufacturer, sizeof(manufacturer),
			                       kCFStringEncodingASCII)) {
				DEBUGF("found manufacturer %s", manufacturer);
				port->usb_manufacturer = strdup(manufacturer);
			}
			CFRelease(cf_property);
		}

		if ((cf_property = IORegistryEntrySearchCFProperty(ioport,kIOServicePlane,
		         CFSTR("USB Product Name"), kCFAllocatorDefault,
		         kIORegistryIterateRecursively | kIORegistryIterateParents))) {
			if (CFStringGetCString(cf_property, product, sizeof(product),
			                       kCFStringEncodingASCII)) {
				DEBUGF("found product name %s", product);
				port->usb_product = strdup(product);
			}
			CFRelease(cf_property);
		}

		if ((cf_property = IORegistryEntrySearchCFProperty(ioport,kIOServicePlane,
		         CFSTR("USB Serial Number"), kCFAllocatorDefault,
		         kIORegistryIterateRecursively | kIORegistryIterateParents))) {
			if (CFStringGetCString(cf_property, serial, sizeof(serial),
			                       kCFStringEncodingASCII)) {
				DEBUGF("found serial number %s", serial);
				port->usb_serial = strdup(serial);
			}
			CFRelease(cf_property);
		}

		IOObjectRelease(ioport);
		break;
	}
	IOObjectRelease(iter);

	return 0;
}

PRIVATE int serial_list(serial_t ***list)
{
	CFMutableDictionaryRef classes;
	io_iterator_t iter;
	char path[PATH_MAX];
	io_object_t port;
	CFTypeRef cf_path;
	Boolean result;
	int ret = 0;

	DEBUG("creating matching dictionary");
	if (!(classes = IOServiceMatching(kIOSerialBSDServiceValue))) {
		SET_FAIL(ret, "IOServiceMatching() failed");
		goto out_done;
	}

	DEBUG("getting matching services");
	if (IOServiceGetMatchingServices(kIOMasterPortDefault, classes,
	                                 &iter) != KERN_SUCCESS) {
		SET_FAIL(ret, "IOServiceGetMatchingServices() failed");
		goto out_done;
	}

	DEBUG("iterating over results");
	while ((port = IOIteratorNext(iter))) {
		cf_path = IORegistryEntryCreateCFProperty(port,
				CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0);
		if (cf_path) {
			result = CFStringGetCString(cf_path, path, sizeof(path),
			                            kCFStringEncodingASCII);
			CFRelease(cf_path);
			if (result) {
                DEBUGF("found port %s", path);
    			if (!(*list = serial_list_append(*list, path))) {
    				SET_ERROR(ret, ENOMEM, "list append failed");
    				IOObjectRelease(port);
    				goto out;
    			}
			}
		}
		IOObjectRelease(port);
	}
out:
	IOObjectRelease(iter);
out_done:

	return ret;
}
