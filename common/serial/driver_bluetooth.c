#include "common/serial.h"
#include "common/debug.h"

PRIVATE char *serial_bluetooth_address(const serial_t *port)
{
	if (!port || port->transport != SERIAL_TRANSPORT_BLUETOOTH
	    || !port->bluetooth_address)
		return NULL;

	RETURN_STRING(port->bluetooth_address);
}
