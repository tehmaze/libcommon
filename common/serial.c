#include <ctype.h>
#include "common/serial.h"
#include "common/debug.h"
#include "common/platform.h"
#include "common/array.h"
#include "common/serial/internal.h"

#if defined(HAVE_TERMIOS)
#include "common/serial/termios/termios.h"
#endif

PRIVATE static const baudrate_t baudrates[] = {
#if defined(PLATFORM_WINDOWS)
	/*
	 * The baudrates 50/75/134/150/200/1800/230400/460800 do not seem to
	 * have documented CBR_* macros.
	 */
	BAUD(110), BAUD(300), BAUD(600), BAUD(1200), BAUD(2400), BAUD(4800),
	BAUD(9600), BAUD(14400), BAUD(19200), BAUD(38400), BAUD(57600),
	BAUD(115200), BAUD(128000), BAUD(256000),
#else
	BAUD(50), BAUD(75), BAUD(110), BAUD(134), BAUD(150), BAUD(200),
	BAUD(300), BAUD(600), BAUD(1200), BAUD(1800), BAUD(2400), BAUD(4800),
	BAUD(9600), BAUD(19200), BAUD(38400), BAUD(57600), BAUD(115200),
	BAUD(230400),
#if !defined(PLATFORM_DARWIN)
	BAUD(460800),
#endif
#endif
};

#define BAUDRATES ARRAY_SIZE(baudrates)

PRIVATE static int get_config(serial_t *port, serial_data_t *data, serial_config_t *config);

PRIVATE static int set_config(serial_t *port, serial_data_t *data, const serial_config_t *config);

#if defined(PLATFORM_WINDOWS)
/** To be called after port receive buffer is emptied. */
PRIVATE static int restart_wait(serial_t *port)
{
	DWORD wait_result;

	if (port->wait_running) {
		/* Check status of running wait operation. */
		if (GetOverlappedResult(port->hdl, &port->wait_ovl,
				&wait_result, FALSE)) {
			DEBUG("previous wait completed");
			port->wait_running = FALSE;
		} else if (GetLastError() == ERROR_IO_INCOMPLETE) {
			DEBUG("previous wait still running");
			RETURN_OK();
		} else {
			RETURN_FAIL("GetOverlappedResult() failed");
		}
	}

	if (!port->wait_running) {
		/* Start new wait operation. */
		if (WaitCommEvent(port->hdl, &port->events,
				&port->wait_ovl)) {
			DEBUG("new wait returned, events already pending");
		} else if (GetLastError() == ERROR_IO_PENDING) {
			DEBUG("new wait running in background");
			port->wait_running = TRUE;
		} else {
			RETURN_FAIL("WaitCommEvent() failed");
		}
	}

	RETURN_OK();
}
#endif


#if defined(USE_TERMIOX)
PRIVATE static int get_flow(int fd, serial_data_t *data)
{
	void *termx;

	DEBUG("getting advanced flow control");

	if (!(termx = talloc_size(NULL, get_termiox_size())))
		RETURN_ERROR(ENOMEM, "termiox malloc failed");

	DEBUGF("ioctl TCGETX on %d", fd);
	if (ioctl(fd, TCGETX, termx) < 0) {
		TALLOC_FREE(termx);
		RETURN_ERRNO("getting termiox failed");
	}

	get_termiox_flow(termx, &data->rts_flow, &data->cts_flow,
			&data->dtr_flow, &data->dsr_flow);

	TALLOC_FREE(termx);

	RETURN_OK();
}

PRIVATE static int set_flow(int fd, serial_data_t *data)
{
	void *termx;

	DEBUG("getting advanced flow control");

	if (!(termx = talloc_size(NULL, get_termiox_size())))
		RETURN_ERROR(ENOMEM, "termiox malloc failed");

	DEBUGF("ioctl TCGETX on %d", fd);
	if (ioctl(fd, TCGETX, termx) < 0) {
		TALLOC_FREE(termx);
		RETURN_ERRNO("getting termiox failed");
	}

	DEBUG("setting advanced flow control");

	set_termiox_flow(termx, data->rts_flow, data->cts_flow,
			data->dtr_flow, data->dsr_flow);

	DEBUGF("ioctl TCSETX on %d", fd);
	if (ioctl(fd, TCSETX, termx) < 0) {
		TALLOC_FREE(termx);
		RETURN_FAIL("setting termiox failed");
	}

	TALLOC_FREE(termx);

	RETURN_OK();
}
#endif /* USE_TERMIOX */

PRIVATE void serial_free(serial_t *port)
{
    TALLOC_FREE(port);
}

PRIVATE int serial_open(serial_t *port, char mode)
{
	serial_data_t data;
	serial_config_t config;
	int ret;

	CHECK_PORT();

	if (mode != 'r' && mode != 'w' && mode != 'x')
		RETURN_ERROR(EINVAL, "invalid mode");

	DEBUGF("serial: opening port %s", port->name);

#if defined(PLATFORM_WINDOWS)
	DWORD desired_access = 0, flags_and_attributes = 0, errors;
	char *escaped_port_name;
	COMSTAT status;

	/* Prefix port name with '\\.\' to work with ports above COM9. */
	if (!(escaped_port_name = malloc(strlen(port->name) + 5)))
		RETURN_ERROR(ENOMEM, "serial: escaped port name malloc failed");
	sprintf(escaped_port_name, "\\\\.\\%s", port->name);

	/* Map 'flags' to the OS-specific settings. */
	flags_and_attributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;
	if (mode == 'r' || mode == 'x')
		desired_access |= GENERIC_READ;
	if (mode == 'w' || mode == 'x')
		desired_access |= GENERIC_WRITE;

	port->hdl = CreateFile(escaped_port_name, desired_access, 0, 0,
			 OPEN_EXISTING, flags_and_attributes, 0);

	free(escaped_port_name);

	if (port->hdl == INVALID_HANDLE_VALUE)
		RETURN_FAIL("serial: port CreateFile() failed");

	/* All timeouts initially disabled. */
	port->timeouts.ReadIntervalTimeout = 0;
	port->timeouts.ReadTotalTimeoutMultiplier = 0;
	port->timeouts.ReadTotalTimeoutConstant = 0;
	port->timeouts.WriteTotalTimeoutMultiplier = 0;
	port->timeouts.WriteTotalTimeoutConstant = 0;

	if (SetCommTimeouts(port->hdl, &port->timeouts) == 0) {
		serial_close(port);
		RETURN_FAIL("serial: SetCommTimeouts() failed");
	}

	/* Prepare OVERLAPPED structures. */
#define INIT_OVERLAPPED(ovl) do { \
	memset(&port->ovl, 0, sizeof(port->ovl)); \
	port->ovl.hEvent = INVALID_HANDLE_VALUE; \
	if ((port->ovl.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL)) \
			== INVALID_HANDLE_VALUE) { \
		serial_close(port); \
		RETURN_FAIL(#ovl "serial: CreateEvent() failed"); \
	} \
} while (0)

	INIT_OVERLAPPED(read_ovl);
	INIT_OVERLAPPED(write_ovl);
	INIT_OVERLAPPED(wait_ovl);

	/* Set event mask for RX and error events. */
	if (SetCommMask(port->hdl, EV_RXCHAR | EV_ERR) == 0) {
		serial_close(port);
		RETURN_FAIL("serial: SetCommMask() failed");
	}

	port->writing = FALSE;
	port->wait_running = FALSE;

	ret = restart_wait(port);

	if (ret < 0) {
		serial_close(port);
		RETURN_CODEVAL(ret);
	}
#else // PLATFORM_WINDOWS
	int flags_local = O_NDELAY | O_NOCTTY | O_NONBLOCK;

	/* Map 'flags' to the OS-specific settings. */
	if (mode == 'x')
		flags_local |= O_RDWR;
	else if (mode == 'r')
		flags_local |= O_RDONLY;
	else if (mode == 'w')
		flags_local |= O_WRONLY;

	DEBUGF("serial: open %s with %#02x",
		port->name, flags_local);

	if ((port->fd = open(port->name, flags_local)) < 0)
		RETURN_ERROR(errno, "serial: open() failed");

    if (fcntl(port->fd, F_SETFL, FNDELAY) == -1)
        RETURN_ERROR(errno, "serial: fcntl(FNDELAY) failed");
#endif

	ret = get_config(port, &data, &config);
	if (ret < 0) {
		serial_close(port);
		RETURN_CODEVAL(ret);
	}

	/* Set sane port settings. */
#if defined(PLATFORM_WINDOWS)
	data.dcb.fBinary = TRUE;
	data.dcb.fDsrSensitivity = FALSE;
	data.dcb.fErrorChar = FALSE;
	data.dcb.fNull = FALSE;
	data.dcb.fAbortOnError = FALSE;
#else
	/* Turn off all fancy termios tricks, give us a raw channel. */
	data.term.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IMAXBEL);
#ifdef IUCLC
	data.term.c_iflag &= ~IUCLC;
#endif
	data.term.c_oflag &= ~(OPOST | ONLCR | OCRNL | ONOCR | ONLRET);
#ifdef OLCUC
	data.term.c_oflag &= ~OLCUC;
#endif
#ifdef NLDLY
	data.term.c_oflag &= ~NLDLY;
#endif
#ifdef CRDLY
	data.term.c_oflag &= ~CRDLY;
#endif
#ifdef TABDLY
	data.term.c_oflag &= ~TABDLY;
#endif
#ifdef BSDLY
	data.term.c_oflag &= ~BSDLY;
#endif
#ifdef VTDLY
	data.term.c_oflag &= ~VTDLY;
#endif
#ifdef FFDLY
	data.term.c_oflag &= ~FFDLY;
#endif
#ifdef OFILL
	data.term.c_oflag &= ~OFILL;
#endif
	data.term.c_lflag &= ~(ISIG | ICANON | ECHO | IEXTEN);
	data.term.c_cc[VMIN] = 0;
	data.term.c_cc[VTIME] = 1;

	/* Ignore modem status lines; enable receiver; leave control lines alone on close. */
	data.term.c_cflag |= (CLOCAL | CREAD | HUPCL);
#endif

#if defined(PLATFORM_WINDOWS)
	if (ClearCommError(port->hdl, &errors, &status) == 0)
		RETURN_FAIL("ClearCommError() failed");
#endif

	ret = set_config(port, &data, &config);

	if (ret < 0) {
		serial_close(port);
		RETURN_CODEVAL(ret);
	}

	return 0;
}

static int find_next_arg(char **save, char **arg, char **val)
{
    char *saveptr;
    char *token = strtok_r(NULL, " ", save);

    if (!token)
        return 0;

    *arg = strtok_r(token, "=", &saveptr);
    if (!*arg) {
        *val = NULL;
        return -1;
    }

    *val = strtok_r(NULL, "", &saveptr);
    if (!*val) {
        return -1;
    }

    return 1;
}

struct find_port {
    uint8_t transports;
    const char *serial;
    const char *product;
    const char *manufacturer;
};

static int find_by_serial(struct find_port *find, char *val)
{
    size_t i, len;
    if (val == NULL) {
        errno = EINVAL;
        return -1;
    }

    len = strlen(val);
    for (i = 0; i < len; i++) {
        char c = val[i];
        if (c >= 'A' && c <= 'F') {
            c = val[i] = tolower(c);
        }
        if ((c < 'a' || c > 'f') && (c < '0' || c > '9')) {
            DEBUGF("bad serial: %s (%c)", val, c);
            errno = EINVAL;
            return -1;
        }
    }

    find->transports = SERIAL_TRANSPORT_USB;
    find->serial = val;
    return 0;
}

static int find_by_product(struct find_port *find, char *val)
{
    find->transports = SERIAL_TRANSPORT_USB;
    find->product = val;
    return 0;
}

static int find_by_manufacturer(struct find_port *find, char *val)
{
    find->transports = SERIAL_TRANSPORT_USB;
    find->manufacturer = val;
    return 0;
}

int serial_find(const char *identifier, char **found)
{
    char *id    = NULL;
    char *save  = NULL;
    char *token = NULL;
    char *arg   = NULL;
    char *val   = NULL;
    size_t i;
    int ret;
    struct find_port find;
    byte_zero(&find, sizeof find);
    find.transports = SERIAL_TRANSPORT_NATIVE |
                      SERIAL_TRANSPORT_USB |
                      SERIAL_TRANSPORT_BLUETOOTH;
 
    serial_t *port = NULL;
    serial_t **ports = talloc_zero(NULL, serial_t *);
    if (ports == NULL)
        RETURN_ERROR(ENOMEM, "out of memory");

    if (identifier == NULL || strlen(identifier) == 0)
        RETURN_ERROR(EINVAL, "empty identifier");

    id = talloc_strdup(NULL, identifier);
    if (id == NULL) {
        TALLOC_FREE(ports);
        RETURN_ERROR(ENOMEM, "out of memory");
    }

    token = strtok_r(id, ":", &save);
    if (token) {
        if (serial_list(&ports) != 0) {
            fprintf(stderr, "serial port iteration failed: %s\n", strerror(errno));
            TALLOC_FREE(ports);
            RETURN_ERROR(errno, "iteration failed");
        }
        
        ret = 0;
        while (ret == 0 && find_next_arg(&save, &arg, &val) == 1) {
            if (!strcmp("serial", arg)) {
                ret = find_by_serial(&find, val);
            } else if (!strcmp("product", arg)) {
                ret = find_by_product(&find, val);
            } else if (!strcmp("manufacturer", arg)) {
                ret = find_by_manufacturer(&find, val);
            } else {
                ret = -1;
                errno = EINVAL;
            }               
        }

        ret = -1;
        for (i = 0; ports[i] != NULL; i++) {
            port = ports[i];

            if ((find.transports & serial_transport(port)) == 0)
                continue;
            if (find.serial) {
                val = serial_usb_serial(port);
                if (val == NULL || strcmp(find.serial, val))
                    continue;
            }
            if (find.product) {
                val = serial_usb_product(port);
                if (val == NULL || strcmp(find.product, val))
                    continue;
            }
            if (find.manufacturer) {
                val = serial_usb_manufacturer(port);
                if (val == NULL || strcmp(find.manufacturer, val))
                    continue;
            }

            /* all criteria match */
            ret = 0;
            errno = 0;
            *found = talloc_strdup(NULL, serial_name(port));
            break;
        }
    } else {
        ret = -1;
        errno = EINVAL;
    }

    TALLOC_FREE(ports);
    RETURN_CODEVAL(ret);
}

PRIVATE int serial_by_name(const char *portname, serial_t **port_ptr)
{
	serial_t *port;
#ifndef NO_PORT_METADATA
	int ret;
#endif
	if (!port_ptr) {
        DEBUGF("%s: received NULL port pointer", portname);
        errno = EINVAL;
        return -1;
    }
	*port_ptr = NULL;

	if (!portname) {
        DEBUG("NULL port name");
        errno = EINVAL;
        return -1;
	}

	DEBUGF("building structure for port %s", portname);

	if (!(port = talloc_zero(NULL, serial_t))) {
        DEBUG("out of memory allocating port structure");
		return -1;
    }
    if (!(port->name = talloc_strdup(port, portname))) {
        DEBUG("out of memory duplicating port name");
        TALLOC_FREE(port);
        errno = ENOMEM;
        return -1;
    }

#if defined(PLATFORM_WINDOWS)
	port->usb_path = NULL;
	port->hdl = INVALID_HANDLE_VALUE;
#else
	port->fd = -1;
#endif

	port->description = NULL;
	port->transport = SERIAL_TRANSPORT_NATIVE;
	port->usb_bus = -1;
	port->usb_address = -1;
	port->usb_vid = -1;
	port->usb_pid = -1;
	port->usb_manufacturer = NULL;
	port->usb_product = NULL;
	port->usb_serial = NULL;
	port->bluetooth_address = NULL;

#ifndef NO_PORT_METADATA
	if ((ret = serial_details(port)) != 0) {
		TALLOC_FREE(port);
		return ret;
	}
#endif

	*port_ptr = port;

	return 0;
}

PRIVATE int serial_close(serial_t *port)
{
	CHECK_OPEN_PORT();

	DEBUGF("closing port %s", port->name);

#if defined(PLATFORM_WINDOWS)
	/* Returns non-zero upon success, 0 upon failure. */
	if (CloseHandle(port->hdl) == 0)
		RETURN_FAIL("port CloseHandle() failed");
	port->hdl = INVALID_HANDLE_VALUE;

	/* Close event handles for overlapped structures. */
#define CLOSE_OVERLAPPED(ovl) do { \
	if (port->ovl.hEvent != INVALID_HANDLE_VALUE && \
		CloseHandle(port->ovl.hEvent) == 0) \
		RETURN_FAIL(# ovl "event CloseHandle() failed"); \
} while (0)
	CLOSE_OVERLAPPED(read_ovl);
	CLOSE_OVERLAPPED(write_ovl);
	CLOSE_OVERLAPPED(wait_ovl);

#else
	/* Returns 0 upon success, -1 upon failure. */
	if (close(port->fd) == -1)
		RETURN_FAIL("close() failed");
	port->fd = -1;
#endif

	RETURN_OK();
}

PRIVATE int serial_flush(serial_t *port, serial_buffers_t buffers)
{
	CHECK_OPEN_PORT();

	if (buffers > SERIAL_BUFFER_BOTH)
		RETURN_ERROR(EINVAL, "invalid buffer selection");

	static const char *buffer_names[] = {"no", "input", "output", "both"};

	DEBUGF("flushing %s buffers on port %s",
		buffer_names[buffers], port->name);

#if defined(PLATFORM_WINDOWS)
	DWORD flags = 0;
	if (buffers & SERIAL_BUFFER_INPUT)
		flags |= PURGE_RXCLEAR;
	if (buffers & SERIAL_BUFFER_OUTPUT)
		flags |= PURGE_TXCLEAR;

	/* Returns non-zero upon success, 0 upon failure. */
	if (PurgeComm(port->hdl, flags) == 0)
		RETURN_FAIL("PurgeComm() failed");

	if (buffers & SERIAL_BUFFER_INPUT)
		TRY(restart_wait(port));
#else
	int flags = 0;
	if (buffers == SERIAL_BUFFER_BOTH)
		flags = TCIOFLUSH;
	else if (buffers == SERIAL_BUFFER_INPUT)
		flags = TCIFLUSH;
	else if (buffers == SERIAL_BUFFER_OUTPUT)
		flags = TCOFLUSH;

	/* Returns 0 upon success, -1 upon failure. */
	if (tcflush(port->fd, flags) < 0)
		RETURN_FAIL("tcflush() failed");
#endif
	RETURN_OK();
}

PRIVATE int serial_drain(serial_t *port)
{
	CHECK_OPEN_PORT();

	DEBUGF("draining port %s", port->name);

#if defined(PLATFORM_WINDOWS)
	/* Returns non-zero upon success, 0 upon failure. */
	if (FlushFileBuffers(port->hdl) == 0)
		RETURN_FAIL("FlushFileBuffers() failed");
	RETURN_OK();
#else
	int result;
	while (1) {
#ifdef __ANDROID__
		int arg = 1;
		result = ioctl(port->fd, TCSBRK, &arg);
#else
		result = tcdrain(port->fd);
#endif
		if (result < 0) {
			if (errno == EINTR) {
				DEBUG("tcdrain() was interrupted");
				continue;
			} else {
				RETURN_FAIL("tcdrain() failed");
			}
		} else {
			RETURN_OK();
		}
	}
#endif
}

PRIVATE char *serial_name(const serial_t *port)
{
	if (!port) {
		return NULL;
    }
	RETURN_STRING(port->name);
}


PRIVATE serial_transport_t serial_transport(const serial_t *port)
{
	if (!port) {
		DEBUG("NULL port");
        return SERIAL_TRANSPORT_INVALID;
    }

	RETURN_INT(port->transport);
}

PRIVATE char *serial_description(const serial_t *port)
{
	if (!port || !port->description) {
		return NULL;
    }
	RETURN_STRING(port->description);
}

PRIVATE serial_transport_t sp_get_port_transport(const serial_t *port)
{
	if (!port) {
		RETURN_ERROR(SERIAL_TRANSPORT_INVALID, "NULL port");
    }
	RETURN_INT(port->transport);
}


PRIVATE static int get_config(serial_t *port, serial_data_t *data, serial_config_t *config)
{
    unsigned int i;

	DEBUGF("getting configuration for port %s", port->name);

#if defined(PLATFORM_WINDOWS)
	if (!GetCommState(port->hdl, &data->dcb))
		RETURN_FAIL("GetCommState() failed");

	for (i = 0; i < BAUDRATES; i++) {
		if (data->dcb.BaudRate == baudrates[i].index) {
			config->baudrate = baudrates[i].value;
			break;
		}
	}

	if (i == BAUDRATES)
		/* BaudRate field can be either an index or a custom baud rate. */
		config->baudrate = data->dcb.BaudRate;

	config->bits = data->dcb.ByteSize;

	if (data->dcb.fParity)
		switch (data->dcb.Parity) {
		case NOPARITY:
			config->parity = SERIAL_PARITY_NONE;
			break;
		case ODDPARITY:
			config->parity = SERIAL_PARITY_ODD;
			break;
		case EVENPARITY:
			config->parity = SERIAL_PARITY_EVEN;
			break;
		case MARKPARITY:
			config->parity = SERIAL_PARITY_MARK;
			break;
		case SPACEPARITY:
			config->parity = SERIAL_PARITY_SPACE;
			break;
		default:
			config->parity = -1;
		}
	else
		config->parity = SERIAL_PARITY_NONE;

	switch (data->dcb.StopBits) {
	case ONESTOPBIT:
		config->stopbits = 1;
		break;
	case TWOSTOPBITS:
		config->stopbits = 2;
		break;
	default:
		config->stopbits = -1;
	}

	switch (data->dcb.fRtsControl) {
	case RTS_CONTROL_DISABLE:
		config->rts = SERIAL_RTS_OFF;
		break;
	case RTS_CONTROL_ENABLE:
		config->rts = SERIAL_RTS_ON;
		break;
	case RTS_CONTROL_HANDSHAKE:
		config->rts = SERIAL_RTS_FLOW_CONTROL;
		break;
	default:
		config->rts = -1;
	}

	config->cts = data->dcb.fOutxCtsFlow ? SERIAL_CTS_FLOW_CONTROL : SERIAL_CTS_IGNORE;

	switch (data->dcb.fDtrControl) {
	case DTR_CONTROL_DISABLE:
		config->dtr = SERIAL_DTR_OFF;
		break;
	case DTR_CONTROL_ENABLE:
		config->dtr = SERIAL_DTR_ON;
		break;
	case DTR_CONTROL_HANDSHAKE:
		config->dtr = SERIAL_DTR_FLOW_CONTROL;
		break;
	default:
		config->dtr = -1;
	}

	config->dsr = data->dcb.fOutxDsrFlow ? SERIAL_DSR_FLOW_CONTROL : SERIAL_DSR_IGNORE;

	if (data->dcb.fInX) {
		if (data->dcb.fOutX)
			config->xon_xoff = SERIAL_XON_XOFF_INOUT;
		else
			config->xon_xoff = SERIAL_XON_XOFF_IN;
	} else {
		if (data->dcb.fOutX)
			config->xon_xoff = SERIAL_XON_XOFF_OUT;
		else
			config->xon_xoff = SERIAL_XON_XOFF_DISABLED;
	}

#else // PLATFORM_WINDOWS

	if (tcgetattr(port->fd, &data->term) < 0)
		RETURN_ERRNO("tcgetattr() failed");

	if (ioctl(port->fd, TIOCMGET, &data->controlbits) < 0)
		RETURN_ERRNO("TIOCMGET ioctl failed");

#ifdef USE_TERMIOX
	int ret = get_flow(port->fd, data);

	if (ret == EINVAL) {
		DEBUG("TERMIOX not supported by device");
		data->termiox_supported = 0;
	} else if (ret != 0) {
		RETURN_CODEVAL(ret);
	} else {
		DEBUG("TERMIOX supported by device");
		data->termiox_supported = 1;
	}
#else
	data->termiox_supported = 0;
#endif

	for (i = 0; i < BAUDRATES; i++) {
		if (cfgetispeed(&data->term) == baudrates[i].index) {
			config->baudrate = baudrates[i].value;
			break;
		}
	}

	if (i == BAUDRATES) {
#ifdef __APPLE__
		config->baudrate = (int)data->term.c_ispeed;
#elif defined(USE_TERMIOS_SPEED)
		TRY(get_baudrate(port->fd, &config->baudrate));
#else
		config->baudrate = -1;
#endif
	}

	switch (data->term.c_cflag & CSIZE) {
	case CS8:
		config->bits = 8;
		break;
	case CS7:
		config->bits = 7;
		break;
	case CS6:
		config->bits = 6;
		break;
	case CS5:
		config->bits = 5;
		break;
	default:
		config->bits = -1;
	}

	if (!(data->term.c_cflag & PARENB) && (data->term.c_iflag & IGNPAR))
		config->parity = SERIAL_PARITY_NONE;
	else if (!(data->term.c_cflag & PARENB) || (data->term.c_iflag & IGNPAR))
		config->parity = -1;
#ifdef CMSPAR
	else if (data->term.c_cflag & CMSPAR)
		config->parity = (data->term.c_cflag & PARODD) ? SERIAL_PARITY_MARK : SERIAL_PARITY_SPACE;
#endif
	else
		config->parity = (data->term.c_cflag & PARODD) ? SERIAL_PARITY_ODD : SERIAL_PARITY_EVEN;

	config->stopbits = (data->term.c_cflag & CSTOPB) ? 2 : 1;

	if (data->term.c_cflag & CRTSCTS) {
		config->rts = SERIAL_RTS_FLOW_CONTROL;
		config->cts = SERIAL_CTS_FLOW_CONTROL;
	} else {
		if (data->termiox_supported && data->rts_flow)
			config->rts = SERIAL_RTS_FLOW_CONTROL;
		else
			config->rts = (data->controlbits & TIOCM_RTS) ? SERIAL_RTS_ON : SERIAL_RTS_OFF;

		config->cts = (data->termiox_supported && data->cts_flow) ?
			SERIAL_CTS_FLOW_CONTROL : SERIAL_CTS_IGNORE;
	}

	if (data->termiox_supported && data->dtr_flow)
		config->dtr = SERIAL_DTR_FLOW_CONTROL;
	else
		config->dtr = (data->controlbits & TIOCM_DTR) ? SERIAL_DTR_ON : SERIAL_DTR_OFF;

	config->dsr = (data->termiox_supported && data->dsr_flow) ?
		SERIAL_DSR_FLOW_CONTROL : SERIAL_DSR_IGNORE;

	if (data->term.c_iflag & IXOFF) {
		if (data->term.c_iflag & IXON)
			config->xon_xoff = SERIAL_XON_XOFF_INOUT;
		else
			config->xon_xoff = SERIAL_XON_XOFF_IN;
	} else {
		if (data->term.c_iflag & IXON)
			config->xon_xoff = SERIAL_XON_XOFF_OUT;
		else
			config->xon_xoff = SERIAL_XON_XOFF_DISABLED;
	}
#endif

	return 0;
}

PRIVATE static int set_config(serial_t *port, serial_data_t *data, const serial_config_t *config)
{
    unsigned int i;
#if defined(PLATFORM_DARWIN)
	BAUD_TYPE baud_nonstd;

	baud_nonstd = B0;
#endif
#ifdef USE_TERMIOS_SPEED
	int baud_nonstd = 0;
#endif

	DEBUGF("setting configuration for port %s", port->name);

#if defined(PLATFORM_WINDOWS)
	if (config->baudrate >= 0) {
		for (i = 0; i < BAUDRATES; i++) {
			if (config->baudrate == baudrates[i].value) {
				data->dcb.BaudRate = baudrates[i].index;
				break;
			}
		}

		if (i == BAUDRATES)
			data->dcb.BaudRate = config->baudrate;
	}

	if (config->bits >= 0)
		data->dcb.ByteSize = config->bits;

	if (config->parity >= 0) {
		switch (config->parity) {
		case SERIAL_PARITY_NONE:
			data->dcb.Parity = NOPARITY;
			break;
		case SERIAL_PARITY_ODD:
			data->dcb.Parity = ODDPARITY;
			break;
		case SERIAL_PARITY_EVEN:
			data->dcb.Parity = EVENPARITY;
			break;
		case SERIAL_PARITY_MARK:
			data->dcb.Parity = MARKPARITY;
			break;
		case SERIAL_PARITY_SPACE:
			data->dcb.Parity = SPACEPARITY;
			break;
		default:
			RETURN_ERROR(EINVAL, "invalid parity setting");
		}
	}

	if (config->stopbits >= 0) {
		switch (config->stopbits) {
		/* Note: There's also ONE5STOPBITS == 1.5 (unneeded so far). */
		case 1:
			data->dcb.StopBits = ONESTOPBIT;
			break;
		case 2:
			data->dcb.StopBits = TWOSTOPBITS;
			break;
		default:
			RETURN_ERROR(EINVAL, "invalid stop bit setting");
		}
	}

	if (config->rts >= 0) {
		switch (config->rts) {
		case SERIAL_RTS_OFF:
			data->dcb.fRtsControl = RTS_CONTROL_DISABLE;
			break;
		case SERIAL_RTS_ON:
			data->dcb.fRtsControl = RTS_CONTROL_ENABLE;
			break;
		case SERIAL_RTS_FLOW_CONTROL:
			data->dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
			break;
		default:
			RETURN_ERROR(EINVAL, "invalid RTS setting");
		}
	}

	if (config->cts >= 0) {
		switch (config->cts) {
		case SERIAL_CTS_IGNORE:
			data->dcb.fOutxCtsFlow = FALSE;
			break;
		case SERIAL_CTS_FLOW_CONTROL:
			data->dcb.fOutxCtsFlow = TRUE;
			break;
		default:
			RETURN_ERROR(EINVAL, "invalid CTS setting");
		}
	}

	if (config->dtr >= 0) {
		switch (config->dtr) {
		case SERIAL_DTR_OFF:
			data->dcb.fDtrControl = DTR_CONTROL_DISABLE;
			break;
		case SERIAL_DTR_ON:
			data->dcb.fDtrControl = DTR_CONTROL_ENABLE;
			break;
		case SERIAL_DTR_FLOW_CONTROL:
			data->dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
			break;
		default:
			RETURN_ERROR(EINVAL, "invalid DTR setting");
		}
	}

	if (config->dsr >= 0) {
		switch (config->dsr) {
		case SERIAL_DSR_IGNORE:
			data->dcb.fOutxDsrFlow = FALSE;
			break;
		case SERIAL_DSR_FLOW_CONTROL:
			data->dcb.fOutxDsrFlow = TRUE;
			break;
		default:
			RETURN_ERROR(EINVAL, "invalid DSR setting");
		}
	}

	if (config->xon_xoff >= 0) {
		switch (config->xon_xoff) {
		case SERIAL_XON_XOFF_DISABLED:
			data->dcb.fInX = FALSE;
			data->dcb.fOutX = FALSE;
			break;
		case SERIAL_XON_XOFF_IN:
			data->dcb.fInX = TRUE;
			data->dcb.fOutX = FALSE;
			break;
		case SERIAL_XON_XOFF_OUT:
			data->dcb.fInX = FALSE;
			data->dcb.fOutX = TRUE;
			break;
		case SERIAL_XON_XOFF_INOUT:
			data->dcb.fInX = TRUE;
			data->dcb.fOutX = TRUE;
			break;
		default:
			RETURN_ERROR(EINVAL, "invalid XON/XOFF setting");
		}
	}

	if (!SetCommState(port->hdl, &data->dcb))
		RETURN_FAIL("SetCommState() failed");

#else // PLATFORM_WINDOWS

	int controlbits;

	if (config->baudrate >= 0) {
		DEBUGF("setting baudrate to %d", config->baudrate);
		for (i = 0; i < BAUDRATES; i++) {
			if (config->baudrate == baudrates[i].value) {
				if (cfsetospeed(&data->term, baudrates[i].index) < 0)
					RETURN_ERRNO("cfsetospeed() failed");

				if (cfsetispeed(&data->term, baudrates[i].index) < 0)
					RETURN_ERRNO("cfsetispeed() failed");
				break;
			}
		}

		/* Non-standard baud rate */
		if (i == BAUDRATES) {
			DEBUG("setting non-standard baudrate");
#if defined(PLATFORM_DARWIN)
			/* Set "dummy" baud rate. */
			if (cfsetspeed(&data->term, B9600) < 0)
				RETURN_ERRNO("cfsetspeed() failed");
			baud_nonstd = config->baudrate;
#elif defined(USE_TERMIOS_SPEED)
			baud_nonstd = 1;
#else
			RETURN_ERROR(ENOTSUP, "non-standard baudrate not supported by OS");
#endif
		}
	}

	if (config->bits >= 0) {
		DEBUG("setting bits");
		data->term.c_cflag &= ~CSIZE;
		switch (config->bits) {
		case 8:
			data->term.c_cflag |= CS8;
			break;
		case 7:
			data->term.c_cflag |= CS7;
			break;
		case 6:
			data->term.c_cflag |= CS6;
			break;
		case 5:
			data->term.c_cflag |= CS5;
			break;
		default:
			RETURN_ERROR(EINVAL, "invalid data bits setting");
		}
	}

	if (config->parity >= 0) {
		DEBUG("setting parity");
		data->term.c_iflag &= ~IGNPAR;
		data->term.c_cflag &= ~(PARENB | PARODD);
#ifdef CMSPAR
		data->term.c_cflag &= ~CMSPAR;
#endif
		switch (config->parity) {
		case SERIAL_PARITY_NONE:
			data->term.c_iflag |= IGNPAR;
			break;
		case SERIAL_PARITY_EVEN:
			data->term.c_cflag |= PARENB;
			break;
		case SERIAL_PARITY_ODD:
			data->term.c_cflag |= PARENB | PARODD;
			break;
#ifdef CMSPAR
		case SERIAL_PARITY_MARK:
			data->term.c_cflag |= PARENB | PARODD;
			data->term.c_cflag |= CMSPAR;
			break;
		case SERIAL_PARITY_SPACE:
			data->term.c_cflag |= PARENB;
			data->term.c_cflag |= CMSPAR;
			break;
#else
		case SERIAL_PARITY_MARK:
		case SERIAL_PARITY_SPACE:
			RETURN_ERROR(ENOTSUP, "mark/space parity not supported");
#endif
		default:
			RETURN_ERROR(EINVAL, "invalid parity setting");
		}
	}

	if (config->stopbits >= 0) {
		DEBUG("setting stopbits");
		data->term.c_cflag &= ~CSTOPB;
		switch (config->stopbits) {
		case 1:
			data->term.c_cflag &= ~CSTOPB;
			break;
		case 2:
			data->term.c_cflag |= CSTOPB;
			break;
		default:
			RETURN_ERROR(EINVAL, "invalid stop bits setting");
		}
	}

	if (config->rts >= 0 || config->cts >= 0) {
		DEBUG("setting RTS/CTS");
		if (data->termiox_supported) {
			data->rts_flow = data->cts_flow = 0;
			switch (config->rts) {
			case SERIAL_RTS_OFF:
			case SERIAL_RTS_ON:
				controlbits = TIOCM_RTS;
				if (ioctl(port->fd, config->rts == SERIAL_RTS_ON ? TIOCMBIS : TIOCMBIC, &controlbits) < 0)
					RETURN_ERRNO("setting RTS signal level failed");
				break;
			case SERIAL_RTS_FLOW_CONTROL:
				data->rts_flow = 1;
				break;
			default:
				break;
			}
			if (config->cts == SERIAL_CTS_FLOW_CONTROL)
				data->cts_flow = 1;

			if (data->rts_flow && data->cts_flow)
				data->term.c_iflag |= CRTSCTS;
			else
				data->term.c_iflag &= ~CRTSCTS;
		} else {
			/* Asymmetric use of RTS/CTS not supported. */
			if (data->term.c_iflag & CRTSCTS) {
				/* Flow control can only be disabled for both RTS & CTS together. */
				if (config->rts >= 0 && config->rts != SERIAL_RTS_FLOW_CONTROL) {
					if (config->cts != SERIAL_CTS_IGNORE)
						RETURN_ERROR(ENOTSUP, "RTS & CTS flow control must be disabled together");
				}
				if (config->cts >= 0 && config->cts != SERIAL_CTS_FLOW_CONTROL) {
					if (config->rts <= 0 || config->rts == SERIAL_RTS_FLOW_CONTROL)
						RETURN_ERROR(ENOTSUP, "RTS & CTS flow control must be disabled together");
				}
			} else {
				/* Flow control can only be enabled for both RTS & CTS together. */
				if (((config->rts == SERIAL_RTS_FLOW_CONTROL) && (config->cts != SERIAL_CTS_FLOW_CONTROL)) ||
					((config->cts == SERIAL_CTS_FLOW_CONTROL) && (config->rts != SERIAL_RTS_FLOW_CONTROL)))
					RETURN_ERROR(ENOTSUP, "RTS & CTS flow control must be enabled together");
			}

			if (config->rts >= 0) {
				if (config->rts == SERIAL_RTS_FLOW_CONTROL) {
					data->term.c_iflag |= CRTSCTS;
				} else {
					controlbits = TIOCM_RTS;
					if (ioctl(port->fd, config->rts == SERIAL_RTS_ON ? TIOCMBIS : TIOCMBIC,
							&controlbits) < 0)
						RETURN_ERRNO("setting RTS signal level failed");
				}
			}
		}
	}

	if (config->dtr >= 0 || config->dsr >= 0) {
		DEBUG("setting DTR/DSR");
		if (data->termiox_supported) {
			data->dtr_flow = data->dsr_flow = 0;
			switch (config->dtr) {
			case SERIAL_DTR_OFF:
			case SERIAL_DTR_ON:
				controlbits = TIOCM_DTR;
				if (ioctl(port->fd, config->dtr == SERIAL_DTR_ON ? TIOCMBIS : TIOCMBIC, &controlbits) < 0)
					RETURN_ERRNO("setting DTR signal level failed");
				break;
			case SERIAL_DTR_FLOW_CONTROL:
				data->dtr_flow = 1;
				break;
			default:
				break;
			}
			if (config->dsr == SERIAL_DSR_FLOW_CONTROL)
				data->dsr_flow = 1;
		} else {
			/* DTR/DSR flow control not supported. */
			if (config->dtr == SERIAL_DTR_FLOW_CONTROL || config->dsr == SERIAL_DSR_FLOW_CONTROL)
				RETURN_ERROR(ENOTSUP, "DTR/DSR flow control not supported");

			if (config->dtr >= 0) {
				controlbits = TIOCM_DTR;
				if (ioctl(port->fd, config->dtr == SERIAL_DTR_ON ? TIOCMBIS : TIOCMBIC,
						&controlbits) < 0)
					RETURN_ERRNO("setting DTR signal level failed");
			}
		}
	}

	if (config->xon_xoff >= 0) {
		DEBUG("setting XON/XOFF");
		data->term.c_iflag &= ~(IXON | IXOFF | IXANY);
		switch (config->xon_xoff) {
		case SERIAL_XON_XOFF_DISABLED:
			break;
		case SERIAL_XON_XOFF_IN:
			data->term.c_iflag |= IXOFF;
			break;
		case SERIAL_XON_XOFF_OUT:
			data->term.c_iflag |= IXON | IXANY;
			break;
		case SERIAL_XON_XOFF_INOUT:
			data->term.c_iflag |= IXON | IXOFF | IXANY;
			break;
		default:
			RETURN_ERROR(EINVAL, "invalid XON/XOFF setting");
		}
	}

	if (tcsetattr(port->fd, TCSANOW, &data->term) < 0)
		RETURN_ERRNO("tcsetattr() failed");

#ifdef __APPLE__
	if (baud_nonstd != B0) {
		if (ioctl(port->fd, IOSSIOSPEED, &baud_nonstd) == -1)
			RETURN_ERRNO("IOSSIOSPEED ioctl failed");
		/*
		 * Set baud rates in data->term to correct, but incompatible
		 * with tcsetattr() value, same as delivered by tcgetattr().
		 */
		if (cfsetspeed(&data->term, baud_nonstd) < 0)
			RETURN_ERRNO("cfsetspeed() failed");
	}
#elif defined(__linux__)
#ifdef USE_TERMIOS_SPEED
	if (baud_nonstd)
		TRY(set_baudrate(port->fd, config->baudrate));
#endif
#ifdef USE_TERMIOX
	if (data->termiox_supported)
		TRY(set_flow(port->fd, data));
#endif
#endif

#endif /* !_WIN32 */

	DEBUG("set config suceeded");
	return 0;
}

PRIVATE int serial_flowcontrol(serial_t *port, serial_flowcontrol_t flowcontrol)
{
	serial_data_t data;
	serial_config_t config;

	CHECK_OPEN_PORT();
	TRY(get_config(port, &data, &config));
	TRY(serial_config_flowcontrol(&config, flowcontrol));
	TRY(set_config(port, &data, &config));
	RETURN_OK();
}

PRIVATE int serial_config_flowcontrol(serial_config_t *config, serial_flowcontrol_t flowcontrol)
{
	if (!config)
		RETURN_ERROR(EINVAL, "NULL configuration");

	if (flowcontrol > SERIAL_FLOWCONTROL_DTRDSR)
		RETURN_ERROR(EINVAL, "invalid flow control setting");

	if (flowcontrol == SERIAL_FLOWCONTROL_XONXOFF)
		config->xon_xoff = SERIAL_XON_XOFF_INOUT;
	else
		config->xon_xoff = SERIAL_XON_XOFF_DISABLED;

	if (flowcontrol == SERIAL_FLOWCONTROL_RTSCTS) {
		config->rts = SERIAL_RTS_FLOW_CONTROL;
		config->cts = SERIAL_CTS_FLOW_CONTROL;
	} else {
		if (config->rts == SERIAL_RTS_FLOW_CONTROL)
			config->rts = SERIAL_RTS_ON;
		config->cts = SERIAL_CTS_IGNORE;
	}

	if (flowcontrol == SERIAL_FLOWCONTROL_DTRDSR) {
		config->dtr = SERIAL_DTR_FLOW_CONTROL;
		config->dsr = SERIAL_DSR_FLOW_CONTROL;
	} else {
		if (config->dtr == SERIAL_DTR_FLOW_CONTROL)
			config->dtr = SERIAL_DTR_ON;
		config->dsr = SERIAL_DSR_IGNORE;
	}

	RETURN_OK();
}

#define C(x,type) \
PRIVATE int serial_##x(serial_t *port, type x) \
{ \
	serial_data_t data; \
	serial_config_t config; \
	CHECK_OPEN_PORT(); \
	TRY(get_config(port, &data, &config)); \
	config.x = x; \
	TRY(set_config(port, &data, &config)); \
	RETURN_OK(); \
} \
\
PRIVATE int serial_config_##x(serial_config_t *config, type x) \
{ \
	if (!config) RETURN_ERROR(EINVAL, "NULL config pointer"); \
	config->x = x; \
	RETURN_OK(); \
} \
\
PRIVATE int serial_config_get_##x(serial_config_t *config, type *x) \
{ \
	if (!config) RETURN_ERROR(EINVAL, "NULL config pointer"); \
	if (!x) RETURN_ERROR(EINVAL, "NULL result pointer"); \
	*x = config->x; \
	RETURN_OK(); \
}
C(baudrate, int);
C(bits, int);
C(parity, serial_parity_t);
C(stopbits, int);
C(rts, serial_rts_t);
C(cts, serial_cts_t);
C(dtr, serial_dtr_t);
C(dsr, serial_dsr_t);
C(xon_xoff, serial_xon_xoff_t);
#undef C
