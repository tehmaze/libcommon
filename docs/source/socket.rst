.. _socket:

socket: IP addressing, UDP and TCP connections
==============================================


Data types
----------

.. c:type:: ip4_t

   Packed IPv4 address.

.. c:type:: addr4_t

   Packed IPv4 address with port.

.. c:type:: ip6_t

   Packed IPv6 address.

.. c:type:: addr6_t

   Packed IPv6 address with port.

.. c:type:: socket_t

   Container for any IPv4 or IPv6 socket.


Variables
^^^^^^^^^

.. c:var:: const ip4_t ip4loopback

   Packed IPv4 loopback address.

.. c:var:: const ip4_t ip4any

   Packet IPv4 ``INADDR_ANY`` address.

.. c:var:: const uint8_t ip6mappedv4prefix[12]

   Packed IPv6 mapped IPv4 prefix.

.. c:var:: const ip6_t ip6loopback

   Packed IPv6 loopback address.

.. c:var:: const ip6_t ip6any

   Packet IPv6 ``INADDR_ANY`` address.

.. c:var:: bool ip6disabled

   Flag that indicates a disabled IPv6 stack. Set by the ip6* functions if
   detected.


API
---

.. c:function:: int ip6resolve(ip6_t ip, const char *host)

.. c:function:: socket_t *socket_new(uint8_t v, uint32_t scope_id)

   Initialize a new :c:type:`socket_t` struct.

.. c:macro:: socket_new4()

   Initialize a new :c:type:`socket_t` struct for an IPv4 address.

.. c:macro:: socket_new6(scope_id)

   Initialize a new :c:type:`socket_t` struct for an IPv6 address.

.. c:function:: void socket_free(socket_t *s)

   Destroy a :c:type:`socket_t` struct.

.. c:macro:: socket_free4

   Alias for :c:func:`socket_free`.

.. c:macro:: socket_free6

   Alias for :c:func:`socket_free`.

.. c:function:: socket_t *socket_tcp4(void)

.. c:function:: socket_t *socket_tcp6(uint32_t scope_id)

.. c:function:: socket_t *socket_udp4(void)

.. c:function:: socket_t *socket_udp6(uint32_t scope_id)

.. c:function:: int socket_bind(socket_t *s, const ip6_t ip, uint16_t port)

.. c:function:: int socket_close(socket_t *s)

.. c:function:: int socket_connect(socket_t *s, const ip6_t ip, uint16_t port)

.. c:function:: bool socket_connected(socket_t *s)

.. c:function:: int socket_listen(socket_t *s, unsigned int backlog)

.. c:function:: ssize_t socket_read(socket_t *s, void *buf, size_t len)

.. c:function:: ssize_t socket_recv(socket_t *s, void *buf, size_t len, ip6_t ip, uint16_t *port)

.. c:function:: ssize_t socket_send(socket_t *s, const void *buf, size_t len, ip6_t ip, uint16_t port)

.. c:function:: ssize_t socket_sendfile(socket_t *s, int in, off_t offset, size_t len)

.. c:function:: int socket_sendfile_full(socket_t *s, int in, size_t len, size_t *out)

.. c:function:: ssize_t socket_write(socket_t *s, const void *buf, size_t len)

.. c:function:: int socket_set_blocking(socket_t *s, int v)

.. c:function:: int socket_set_ipv6only(socket_t *s, int v)

.. c:function:: int socket_set_nopipe(socket_t *s, int v)

.. c:function:: int socket_set_reuseaddr(socket_t *s, int v)

.. c:function:: int socket_set_reuseport(socket_t *s, int v)

.. c:macro:: isip4mapped(ip)

   Macro to check if a given packed IP address is an IPv6 mapped IPv4 address.

.. c:macro:: CHECK_SOCKET(s)

   Macro to check if a :c:type:`socket_t` struct is initialized properly.
