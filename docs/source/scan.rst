.. _scan:

scan: String Parsing
====================


API
---

.. c:function:: int scan_fromhex(unsigned char c)
.. c:function:: size_t scan_ulong(const char *src, unsigned long int *dst)
.. c:function:: size_t scan_xlong(const char *src, unsigned long *dst)
.. c:function:: unsigned int scan_ip4(const char *src, ip4_t ip)
.. c:function:: unsigned int scan_ip6(const char *src, ip6_t ip)
.. c:function:: unsigned int scan_ip6_flat(const char *src, ip6_t ip)
