format: String Formatting
=========================


Data types
----------

No data types are defined in this module.


API
---


Network Address Formatting
^^^^^^^^^^^^^^^^^^^^^^^^^^

 .. c:function:: unsigned int format_addr4(char *dst, const addr4_t addr)
 .. c:function:: char *       format_addr4s(const addr4_t addr)
 .. c:function:: unsigned int format_addr6(char *dst, const addr6_t addr)
 .. c:function:: char *       format_addr6s(const addr6_t addr)
 .. c:function:: unsigned int format_ip4(char *dst, const ip4_t ip)
 .. c:function:: char *       format_ip4s(const ip4_t ip)
 .. c:function:: unsigned int format_ip6(char *dst, const ip6_t ip)
 .. c:function:: char *       format_ip6s(const ip6_t ip)


File System Path Formatting
^^^^^^^^^^^^^^^^^^^^^^^^^^^

 .. c:function:: int          format_path_canonical(char *dst, size_t len, const char *path)
 .. c:function:: const char * format_path_ext(const char *path)
 .. c:function:: void         format_path_join(char *dst, size_t len, const char *dir, const char *file)
 .. c:function:: char         format_path_sep(void)


Number Formatting
^^^^^^^^^^^^^^^^^

 .. c:function:: size_t       format_ulong(char *dst, unsigned long i)
 .. c:function:: size_t       format_xlong(char *dst, unsigned long i)
