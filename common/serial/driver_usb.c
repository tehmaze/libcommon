#include "common/serial.h"
#include "common/debug.h"

PRIVATE int serial_usb_bus_address(const serial_t *port, int *usb_bus, int *usb_address)
{
	if (!port)
		RETURN_ERROR(EINVAL, "NULL port");
	if (port->transport != SERIAL_TRANSPORT_USB)
		RETURN_ERROR(EINVAL, "port does not use USB transport");
	if (port->usb_bus < 0 || port->usb_address < 0)
		RETURN_ERROR(ENOTSUP, "bus and address values are not available");

	if (usb_bus)
		*usb_bus = port->usb_bus;
	if (usb_address)
		*usb_address = port->usb_address;

	RETURN_OK();
}

PRIVATE int serial_usb_vid_pid(const serial_t *port, int *usb_vid, int *usb_pid)
{
	if (!port)
		RETURN_ERROR(EINVAL, "NULL port");
	if (port->transport != SERIAL_TRANSPORT_USB)
		RETURN_ERROR(EINVAL, "port does not use USB transport");
	if (port->usb_vid < 0 || port->usb_pid < 0)
		RETURN_ERROR(ENOTSUP, "VID:PID values are not available");

	if (usb_vid)
		*usb_vid = port->usb_vid;
	if (usb_pid)
		*usb_pid = port->usb_pid;

	RETURN_OK();
}

PRIVATE char *serial_usb_manufacturer(const serial_t *port)
{
	if (!port || port->transport != SERIAL_TRANSPORT_USB || !port->usb_manufacturer)
		return NULL;

	RETURN_STRING(port->usb_manufacturer);
}

PRIVATE char *serial_usb_product(const serial_t *port)
{
	if (!port || port->transport != SERIAL_TRANSPORT_USB || !port->usb_product)
		return NULL;

	RETURN_STRING(port->usb_product);
}

PRIVATE char *serial_usb_serial(const serial_t *port)
{
	if (!port || port->transport != SERIAL_TRANSPORT_USB || !port->usb_serial)
		return NULL;

	RETURN_STRING(port->usb_serial);
}
