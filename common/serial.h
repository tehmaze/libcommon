#ifndef _COMMON_SERIAL_H
#define _COMMON_SERIAL_H

#include "common.h"
#include "common/platform.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <talloc.h>

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#include <tchar.h>
#include <setupapi.h>
// These come from http://www.microsoft.com/whdc/DevTools/WDK/WDKpkg.mspx
#include <cfgmgr32.h>
#undef DEFINE_GUID
#define DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
	static const GUID name = { l,w1,w2,{ b1,b2,b3,b4,b5,b6,b7,b8 } }
#include <usbioctl.h>
#include <usbiodef.h>
#else // PLATFORM_WINDOWS
#include <limits.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <limits.h>
#include <poll.h>
#endif // PLATFORM_WINDOWS

#if defined(PLATFORM_DARWIN)
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/serial/ioss.h>
#include <sys/syslimits.h>
#endif // PLATFORM_DARWIN

#if defined(PLATFORM_LINUX)
#include <dirent.h>
#endif // PLATFORM_LINUX

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(TCGETX) && defined(TCSETX) && defined(HAVE_STRUCT_TERMIOX)
#define USE_TERMIOX 1
#endif
#if !defined(TIOCOUTQ) && defined(FIONWRITE)
#define TIOCOUTQ FIONWRITE
#endif

typedef enum {
    SERIAL_TRANSPORT_INVALID   = 0x00,
	SERIAL_TRANSPORT_NATIVE    = 0x01,
	SERIAL_TRANSPORT_USB       = 0x02,
	SERIAL_TRANSPORT_BLUETOOTH = 0x04,
} serial_transport_t;

/** Parity settings. */
typedef enum {
	SERIAL_PARITY_INVALID = -1,
	SERIAL_PARITY_NONE = 0,
	SERIAL_PARITY_ODD = 1,
	SERIAL_PARITY_EVEN = 2,
	SERIAL_PARITY_MARK = 3,
	SERIAL_PARITY_SPACE = 4
} serial_parity_t;

/** RTS pin behaviour. */
typedef enum {
	SERIAL_RTS_INVALID = -1,
	SERIAL_RTS_OFF = 0,
	SERIAL_RTS_ON = 1,
	SERIAL_RTS_FLOW_CONTROL = 2
} serial_rts_t;

/** CTS pin behaviour. */
typedef enum {
	SERIAL_CTS_INVALID = -1,
	SERIAL_CTS_IGNORE = 0,
	SERIAL_CTS_FLOW_CONTROL = 1
} serial_cts_t;

/** DTR pin behaviour. */
typedef enum {
	SERIAL_DTR_INVALID = -1,
	SERIAL_DTR_OFF = 0,
	SERIAL_DTR_ON = 1,
	SERIAL_DTR_FLOW_CONTROL = 2
} serial_dtr_t;

/** DSR pin behaviour. */
typedef enum {
	SERIAL_DSR_INVALID = -1,
	SERIAL_DSR_IGNORE = 0,
	SERIAL_DSR_FLOW_CONTROL = 1
} serial_dsr_t;

/** XON/XOFF flow control behaviour. */
typedef enum {
	SERIAL_XON_XOFF_INVALID = -1,
	SERIAL_XON_XOFF_DISABLED = 0,
	SERIAL_XON_XOFF_IN = 1,
	SERIAL_XON_XOFF_OUT = 2,
	SERIAL_XON_XOFF_INOUT = 3
} serial_xon_xoff_t;

/** Standard flow control combinations. */
typedef enum {
	SERIAL_FLOWCONTROL_NONE = 0,
	SERIAL_FLOWCONTROL_XONXOFF = 1,
	SERIAL_FLOWCONTROL_RTSCTS = 2,
	SERIAL_FLOWCONTROL_DTRDSR = 3
} serial_flowcontrol_t;

typedef struct {
	int                  baudrate;
	int                  bits;
	serial_parity_t      parity;
	int                  stopbits;
    serial_rts_t         rts;
    serial_cts_t         cts;
    serial_dtr_t         dtr;
    serial_dsr_t         dsr;
    serial_xon_xoff_t    xon_xoff;
} serial_config_t;

typedef enum {
	SERIAL_BUFFER_NONE   = 0x00,
	SERIAL_BUFFER_INPUT  = 0x01,
	SERIAL_BUFFER_OUTPUT = 0x02,
	SERIAL_BUFFER_BOTH   = 0x03
} serial_buffers_t;

typedef struct {
#if defined(PLATFORM_WINDOWS)
	DCB dcb;
#else
	struct termios term;
	int            controlbits;
	int            termiox_supported;
	int            rts_flow;
	int            cts_flow;
	int            dtr_flow;
	int            dsr_flow;
#endif
} serial_data_t;

typedef struct {
    char               *name;
	char               *description;
	serial_transport_t transport;
	int                usb_bus;
	int                usb_address;
	int                usb_vid;
	int                usb_pid;
	char               *usb_manufacturer;
	char               *usb_product;
	char               *usb_serial;
	char               *bluetooth_address;
#if defined(__MINGW32__)
	char               *usb_path;
	HANDLE             hdl;
	COMMTIMEOUTS       timeouts;
	OVERLAPPED         write_ovl;
	OVERLAPPED         read_ovl;
	OVERLAPPED         wait_ovl;
	DWORD              events;
	BYTE               pending_byte;
	BOOL               writing;
	BOOL               wait_running;
#else
	int                fd;
#endif
} serial_t;

int                serial_open(serial_t *port, char mode);
int                serial_find(const char *identifier, char **found);
int                serial_close(serial_t *port);
int                serial_config_new(serial_config_t **config_ptr);
void               serial_config_free(serial_config_t *config);
int                serial_config_get(serial_t *port, serial_config_t *config);
int                serial_config(serial_t *port, serial_config_t *config);
int                serial_baudrate(serial_t *port, int baudrate);
int                serial_parity(serial_t *port, serial_parity_t parity);
int                serial_bits(serial_t *port, int bits);
int                serial_stopbits(serial_t *port, int stopbits);
int                serial_rts(serial_t *port, serial_rts_t rts);
int                serial_cts(serial_t *port, serial_cts_t cts);
int                serial_dtr(serial_t *port, serial_dtr_t dtr);
int                serial_dsr(serial_t *port, serial_dsr_t dsr);
int                serial_xon_xoff(serial_t *port, serial_xon_xoff_t xon_xoff);
int                serial_flowcontrol(serial_t *port, serial_flowcontrol_t flowcontrol);
int                serial_read(serial_t *port, void *buf, size_t len, unsigned int timeout_ms);
int 			   serial_read_next(serial_t *port, void *buf, size_t len, unsigned int timeout_ms);
int                serial_read_nonblock(serial_t *port, void *buf, size_t len);
int                serial_read_waiting(serial_t *port);
int                serial_write(serial_t *port, const void *buf, size_t len, unsigned int timeout_ms);
int                serial_write_nonblock(serial_t *port, const void *buf, size_t len);
int                serial_write_waiting(serial_t *port);
int                serial_flush(serial_t *port, serial_buffers_t buffers);
int                serial_drain(serial_t *port);
void               serial_free(serial_t *port);
int                serial_details(serial_t *port);
int                serial_by_name(const char *portname, serial_t **port_ptr);
char *             serial_name(const serial_t *port);
serial_transport_t serial_transport(const serial_t *port);
int                serial_list(serial_t ***list);
serial_t **        serial_list_append(serial_t **list, const char *portname);
void               serial_list_free(serial_t **list);
char *             serial_bluetooth_address(const serial_t *port);
int                serial_usb_bus_address(const serial_t *port, int *usb_bus, int *usb_address);
int                serial_usb_vid_pid(const serial_t *port, int *usb_vid, int *usb_pid);
char *             serial_usb_manufacturer(const serial_t *port);
char *             serial_usb_product(const serial_t *port);
char *             serial_usb_serial(const serial_t *port);
int 			   serial_flowcontrol(serial_t *port, serial_flowcontrol_t flowcontrol);
int 			   serial_config_flowcontrol(serial_config_t *config, serial_flowcontrol_t flowcontrol);

#define C(x,type) \
int serial_##x(serial_t *port, type x); \
int serial_config_##x(serial_config_t *config, type x); \
int serial_config_get_##x(serial_config_t *config, type *x);
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

#if defined(PLATFORM_WINDOWS)
char* strtok_r(char *str, const char *delim, char **nextp);
#endif

#if defined(__cplusplus)
}
#endif

#endif // _COMMON_SERIAL_H
