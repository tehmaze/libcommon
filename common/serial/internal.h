#ifndef _COMMON_SERIAL_INTERNAL_H
#define _COMMON_SERIAL_INTERNAL_H

/* Standard baud rates. */
#if defined(PLATFORM_WINDOWS)
#define BAUD_TYPE DWORD
#define BAUD(n) {CBR_##n, n}
#else
#define BAUD_TYPE speed_t
#define BAUD(n) {B##n, n}
#endif

typedef struct {
	BAUD_TYPE index;
	int       value;
} baudrate_t;

#define CHECK_PORT() do { \
	if (!port) \
		RETURN_ERROR(EINVAL, "NULL port"); \
	if (!port->name) \
		RETURN_ERROR(EINVAL, "NULL port name"); \
} while (0)
#if defined(PLATFORM_WINDOWS)
#define CHECK_PORT_HANDLE() do { \
	if (port->hdl == INVALID_HANDLE_VALUE) \
		RETURN_ERROR(EINVAL, "port not open"); \
} while (0)
#else
#define CHECK_PORT_HANDLE() do { \
	if (port->fd < 0) \
		RETURN_ERROR(EINVAL, "port not open"); \
} while (0)
#endif
#define CHECK_OPEN_PORT() do { \
	CHECK_PORT(); \
	CHECK_PORT_HANDLE(); \
} while (0)

#endif // _COMMON_SERIAL_INTERNAL_H
