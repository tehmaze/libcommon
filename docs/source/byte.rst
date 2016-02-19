.. _byte:

byte: Common Byte Operations
============================


Data types
----------

No data types are defined in this module.


API
---

.. c:function:: int8_t byte_cmp(const void *a, const void *b, size_t len)

   Compare two byte strings and return the difference.

.. c:function:: void byte_copy(void *dst, const void *src, size_t len)

   Copy a byte string.

.. c:function:: bool byte_equal(const void *a, const void *b, size_t len)

   Check if two byte strings are equal.

.. warning::

   Do not use to compare passwords, see :c:func:`byte_equal_safe`.

.. c:function:: bool byte_equal_safe(const void *a, const void *b, size_t len)

   Check if two byte strings are equal by doing a full XOR comparison of both
   byte strings and checking the result.

.. c:function:: void byte_zero(void *buf, size_t len)

   Set the contents of a byte string to all zero.
