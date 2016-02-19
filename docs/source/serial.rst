.. _serial:

serial: Serial Communications
=============================


Data types
----------

.. c:type:: serial_t

   Container for a configured serial port.

.. c:type:: serial_config_t

   Container for all serial port configuration settings.

.. c:type:: serial_transport_t

   Serial transport type.

.. c:var:: SERIAL_TRANSPORT_INVALID = 0x00
.. c:var:: SERIAL_TRANSPORT_NATIVE = 0x01
.. c:var:: SERIAL_TRANSPORT_USB = 0x02
.. c:var:: SERIAL_TRANSPORT_BLUETOOTH = 0x04

.. c:type:: serial_parity_t

   Data parity behaviour.

.. c:var:: SERIAL_PARITY_INVALID = -1
.. c:var:: SERIAL_PARITY_NONE = 0
.. c:var:: SERIAL_PARITY_ODD = 1
.. c:var:: SERIAL_PARITY_EVEN = 2
.. c:var:: SERIAL_PARITY_MARK = 3
.. c:var:: SERIAL_PARITY_SPACE = 4

.. c:type:: serial_rts_t

   RTS pin behaviour.

.. c:var:: SERIAL_RTS_INVALID = -1
.. c:var:: SERIAL_RTS_OFF = 0
.. c:var:: SERIAL_RTS_ON = 1
.. c:var:: SERIAL_RTS_FLOW_CONTROL = 2

.. c:type:: serial_cts_t

   CTS pin behaviour.

.. c:var:: SERIAL_CTS_INVALID = -1
.. c:var:: SERIAL_CTS_IGNORE = 0
.. c:var:: SERIAL_CTS_FLOW_CONTROL = 1

.. c:type:: serial_dtr_t

   DTR pin behaviour.

.. c:var:: SERIAL_DTR_INVALID = -1
.. c:var:: SERIAL_DTR_OFF = 0
.. c:var:: SERIAL_DTR_ON = 1
.. c:var:: SERIAL_DTR_FLOW_CONTROL = 2

.. c:type:: serial_dsr_t

   DSR pin behaviour.

.. c:var:: SERIAL_DSR_INVALID = -1
.. c:var:: SERIAL_DSR_IGNORE = 0
.. c:var:: SERIAL_DSR_FLOW_CONTROL = 1

.. c:type:: serial_xon_xoff_t

   XON/XOFF flow control behaviour.

.. c:var:: SERIAL_XON_XOFF_INVALID = -1
.. c:var:: SERIAL_XON_XOFF_DISABLED = 0
.. c:var:: SERIAL_XON_XOFF_IN = 1
.. c:var:: SERIAL_XON_XOFF_OUT = 2
.. c:var:: SERIAL_XON_XOFF_INOUT = 3

.. c:type:: serial_flowcontrol_t

   Standard flow control combinations.

.. c:var:: SERIAL_FLOWCONTROL_NONE = 0
.. c:var:: SERIAL_FLOWCONTROL_XONXOFF = 1
.. c:var:: SERIAL_FLOWCONTROL_RTSCTS = 2
.. c:var:: SERIAL_FLOWCONTROL_DTRDSR = 3

.. c:type:: serial_buffers_t

   Serial buffer types.

.. c:var:: SERIAL_BUFFER_NONE = 0x00
.. c:var:: SERIAL_BUFFER_INPUT = 0x01
.. c:var:: SERIAL_BUFFER_OUTPUT = 0x02
.. c:var:: SERIAL_BUFFER_BOTH = 0x03

.. c:type:: serial_data_t


Public members
^^^^^^^^^^^^^^

.. c:member:: int serial_config_t.baudrate
.. c:member:: int serial_config_t.bits
.. c:member:: serial_parity_t serial_config_t.parity
.. c:member:: int serial_config_t.stopbits
.. c:member:: serial_rts_t serial_config_t.rts
.. c:member:: serial_cts_t serial_config_t.cts
.. c:member:: serial_dtr_t serial_config_t.dtr
.. c:member:: serial_dsr_t serial_config_t.dsr
.. c:member:: serial_xon_xoff_t serial_config_t.xon_xoff

.. c:member:: char *serial_t.name
.. c:member:: serial_t_transport_t serial.transport
.. c:member:: int serial_t.usb_bus
.. c:member:: int serial_t.usb_address
.. c:member:: int serial_t.usb_vid
.. c:member:: int serial_t.usb_pid
.. c:member:: char *serial_t.usb_manufacturer
.. c:member:: char *serial_t.usb_product
.. c:member:: char *serial_t.usb_serial
.. c:member:: char *serial_t.bluetooth_address


API
---

.. c:function:: int serial_open(serial_t *port, char mode)
.. c:function:: int serial_find(const char *identifier, char **found)
.. c:function:: int serial_close(serial_t *port)
.. c:function:: int serial_config_new(serial_config_t **config_ptr)
.. c:function:: void serial_config_free(serial_config_t *config)
.. c:function:: int serial_config_get(serial_t *port, serial_config_t *config)
.. c:function:: int serial_config(serial_t *port, serial_config_t *config)
.. c:function:: int serial_baudrate(serial_t *port, int baudrate)
.. c:function:: int serial_parity(serial_t *port, serial_parity_t parity)
.. c:function:: int serial_bits(serial_t *port, int bits)
.. c:function:: int serial_stopbits(serial_t *port, int stopbits)
.. c:function:: int serial_rts(serial_t *port, serial_rts_t rts)
.. c:function:: int serial_cts(serial_t *port, serial_cts_t cts)
.. c:function:: int serial_dtr(serial_t *port, serial_dtr_t dtr)
.. c:function:: int serial_dsr(serial_t *port, serial_dsr_t dsr)
.. c:function:: int serial_xon_xoff(serial_t *port, serial_xon_xoff_t xon_xoff)
.. c:function:: int serial_flowcontrol(serial_t *port, serial_flowcontrol_t flowcontrol)
.. c:function:: int serial_read(serial_t *port, void *buf, size_t len, unsigned int timeout_ms)
.. c:function:: int serial_read_next(serial_t *port, void *buf, size_t len, unsigned int timeout_ms)
.. c:function:: int serial_read_nonblock(serial_t *port, void *buf, size_t len)
.. c:function:: int serial_read_waiting(serial_t *port)
.. c:function:: int serial_write(serial_t *port, const void *buf, size_t len, unsigned int timeout_ms)
.. c:function:: int serial_write_nonblock(serial_t *port, const void *buf, size_t len)
.. c:function:: int serial_write_waiting(serial_t *port)
.. c:function:: int serial_flush(serial_t *port, serial_buffers_t buffers)
.. c:function:: int serial_drain(serial_t *port)
.. c:function:: void serial_free(serial_t *port)
.. c:function:: int serial_details(serial_t *port)
.. c:function:: int serial_by_name(const char *portname, serial_t **port_ptr)
.. c:function:: char *serial_name(const serial_t *port)
.. c:function:: serial_transport_t serial_transport(const serial_t *port)
.. c:function:: int serial_list(serial_t ***list)
.. c:function:: serial_t **serial_list_append(serial_t **list, const char *portname)
.. c:function:: void serial_list_free(serial_t **list)
.. c:function:: char *serial_bluetooth_address(const serial_t *port)
.. c:function:: int serial_usb_bus_address(const serial_t *port, int *usb_bus, int *usb_address)
.. c:function:: int serial_usb_vid_pid(const serial_t *port, int *usb_vid, int *usb_pid)
.. c:function:: char *serial_usb_manufacturer(const serial_t *port)
.. c:function:: char *serial_usb_product(const serial_t *port)
.. c:function:: char *serial_usb_serial(const serial_t *port)
.. c:function:: int serial_flowcontrol(serial_t *port, serial_flowcontrol_t flowcontrol)
.. c:function:: int serial_config_flowcontrol(serial_config_t *config, serial_flowcontrol_t flowcontrol)
.. c:function:: int serial_baudrate(serial_t *port, int baudrate)
.. c:function:: int serial_config_baudrate(serial_config_t *config, int baudrate)
.. c:function:: int serial_config_get_baudrate(serial_config_t *config, int *baudrate)
.. c:function:: int serial_bits(serial_t *port, int bits)
.. c:function:: int serial_config_bits(serial_config_t *config, int bits)
.. c:function:: int serial_config_get_bits(serial_config_t *config, int *bits)
.. c:function:: int serial_parity(serial_t *port, serial_parity_t parity)
.. c:function:: int serial_config_parity(serial_config_t *config, serial_parity_t parity)
.. c:function:: int serial_config_get_parity(serial_config_t *config, serial_parity_t *parity)
.. c:function:: int serial_stopbits(serial_t *port, int stopbits)
.. c:function:: int serial_config_stopbits(serial_config_t *config, int stopbits)
.. c:function:: int serial_config_get_stopbits(serial_config_t *config, int *stopbits)
.. c:function:: int serial_rts(serial_t *port, serial_rts_t rts)
.. c:function:: int serial_config_rts(serial_config_t *config, serial_rts_t rts)
.. c:function:: int serial_config_get_rts(serial_config_t *config, serial_rts_t *rts)
.. c:function:: int serial_cts(serial_t *port, serial_cts_t cts)
.. c:function:: int serial_config_cts(serial_config_t *config, serial_cts_t cts)
.. c:function:: int serial_config_get_cts(serial_config_t *config, serial_cts_t *cts)
.. c:function:: int serial_dtr(serial_t *port, serial_dtr_t dtr)
.. c:function:: int serial_config_dtr(serial_config_t *config, serial_dtr_t dtr)
.. c:function:: int serial_config_get_dtr(serial_config_t *config, serial_dtr_t *dtr)
.. c:function:: int serial_dsr(serial_t *port, serial_dsr_t dsr)
.. c:function:: int serial_config_dsr(serial_config_t *config, serial_dsr_t dsr)
.. c:function:: int serial_config_get_dsr(serial_config_t *config, serial_dsr_t *dsr)
.. c:function:: int serial_xon_xoff(serial_t *port, serial_xon_xoff_t xon_xoff)
.. c:function:: int serial_config_xon_xoff(serial_config_t *config, serial_xon_xoff_t xon_xoff)
.. c:function:: int serial_config_get_xon_xoff(serial_config_t *config, serial_xon_xoff_t *xon_xoff)
