.. _uint:

uint: Unsigned Integers
=======================



Data Types
----------

.. c:type:: __uint8_t uint8_t

   Container for 8 bit unsigned integers.

.. c:type:: __uint16_t uint16_t

   Container for 16 bit unsigned integers.

.. c:type:: __uint24_t uint24_t

   Container for 24 bit unsigned integers.

.. c:type:: __uint32_t uint32_t

   Container for 32 bit unsigned integers.

.. c:type:: __uint64_t uint64_t

   Container for 64 bit unsigned integers.


API
---

Parsing Packed uints From a Buffer
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. c:function:: uint16_t uint16(uint8_t *buf)
.. c:function:: uint16_t uint16_le(uint8_t *buf)
.. c:function:: uint24_t uint24(uint8_t *buf)
.. c:function:: uint24_t uint24_le(uint8_t *buf)
.. c:function:: uint32_t uint32(uint8_t *buf)
.. c:function:: uint32_t uint32_le(uint8_t *buf)
.. c:function:: uint64_t uint64(uint8_t *buf)
.. c:function:: uint64_t uint64_le(uint8_t *buf)


Packing uints In A Buffer
^^^^^^^^^^^^^^^^^^^^^^^^^

.. c:function:: void uint8_pack(uint8_t *buf, uint8_t in)
.. c:macro:: uint8_pack_le uint8_pack
.. c:function:: uint8_t *uint8_packs(uint8_t in)
.. c:macro:: uint8_packs_le uint8_packs
.. c:function:: void uint16_pack(uint8_t *buf, uint16_t in)
.. c:function:: void uint16_pack_le(uint8_t *buf, uint16_t in)
.. c:function:: uint8_t *uint16_packs(uint16_t in)
.. c:function:: uint8_t *uint16_packs_le(uint16_t in)
.. c:function:: void uint24_pack(uint8_t *buf, uint24_t in)
.. c:function:: void uint24_pack_le(uint8_t *buf, uint24_t in)
.. c:function:: uint8_t *uint24_packs(uint24_t in)
.. c:function:: uint8_t *uint24_packs_le(uint24_t in)
.. c:function:: void uint32_pack(uint8_t *buf, uint32_t in)
.. c:function:: void uint32_pack_le(uint8_t *buf, uint32_t in)
.. c:function:: uint8_t *uint32_packs(uint32_t in)
.. c:function:: uint8_t *uint32_packs_le(uint32_t in)
.. c:function:: void uint64_pack(uint8_t *buf, uint64_t in)
.. c:function:: void uint64_pack_le(uint8_t *buf, uint64_t in)
.. c:function:: uint8_t *uint64_packs(uint64_t in)
.. c:function:: uint8_t *uint64_packs_le(uint64_t in)


Packing uints To A Buffer In Hexadecimal Notation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. c:function:: void xuint_pack(uint8_t *buf, const uint8_t *src, size_t len)
.. c:function:: void xuint8_pack(uint8_t *buf, uint8_t in)
.. c:macro:: xuint8_pack_le xuint8_pack
.. c:function:: uint8_t *xuint8_packs(uint8_t in)
.. c:macro:: xuint8_packs_le xuint8_packs
.. c:function:: void xuint16_pack(uint8_t *buf, uint16_t in)
.. c:function:: void xuint16_pack_le(uint8_t *buf, uint16_t in)
.. c:function:: uint8_t *xuint16_packs(uint16_t in)
.. c:function:: uint8_t *xuint16_packs_le(uint16_t in)
.. c:function:: void xuint24_pack(uint8_t *buf, uint24_t in)
.. c:function:: void xuint24_pack_le(uint8_t *buf, uint24_t in)
.. c:function:: uint8_t *xuint24_packs(uint24_t in)
.. c:function:: uint8_t *xuint24_packs_le(uint24_t in)
.. c:function:: void xuint32_pack(uint8_t *buf, uint32_t in)
.. c:function:: void xuint32_pack_le(uint8_t *buf, uint32_t in)
.. c:function:: uint8_t *xuint32_packs(uint32_t in)
.. c:function:: uint8_t *xuint32_packs_le(uint32_t in)
.. c:function:: void xuint64_pack(uint8_t *buf, uint64_t in)
.. c:function:: void xuint64_pack_le(uint8_t *buf, uint64_t in)
.. c:function:: uint8_t *xuint64_packs(uint64_t in)
.. c:function:: uint8_t *xuint64_packs_le(uint64_t in)
