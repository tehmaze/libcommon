.. _tai:

tai: International Atomic Time
==============================

TAI or **Temps Atomique International** (French for International Atomic Time),
measures real time. One second of TAI time is a constant duration defined by
cesium radiation. TAI has been measured continuously since 1955 and is the
foundation of all civil time standards.


Data types
----------

.. c:type:: struct tai

   Platform independent container for a TAI, time with second precision.

   It holds a value between 0 inclusive and :math:`2^{64}` exclusive.
   Applications are discouraged to look inside ``struct tai``.

.. c:type:: struct taia

   Platform independent container for a TAIA, time with atto second precision.

   It holds a value between 0 inclusive and :math:`2^{64}` exclusive. The number
   is a multiple of :math:`10^{-18}`. Applicaitons are discouraged to look inside
   ``struct taia``.


Public members
^^^^^^^^^^^^^^

.. c:member:: uint64_t tai.x

   Number of TAI seconds.

.. c:member:: struct tai taia.sec

   Number of TAI seconds.

.. c:member:: unsigned long taia.nano

   Number of nano seconds :math:`0..99999999`

.. c:member:: unsigned long taia.atto

   Number of atto seconds :math:`0..99999999`


API
---

.. c:function:: void tai_now(struct tai *t)

   Returns the current time in TAI.

.. note::

   It is assumed that the :c:func:`time` function returns a time that
   represents the number of TAI seconds since 1970-01-01 00:00:10 TAI.

.. c:function:: void tai_add(struct tai *t, const struct tai *a, const struct tai *b)

   Add two TAI and collect the results in ``t``.

.. c:function:: void tai_sub(struct tai *t, const struct tai *a, const struct tai *b)

   Substract two TAI and collect the results in ``t``.

.. c:function:: int tai_cmp(const struct tai *a, const struct tai *b)

   Compare two TAI, suitable for most sort functions.

.. c:function:: void tai_pack(const struct tai *t, char *s)

   Pack a TAI to string, suitable for disk storage or network usage.

.. c:function:: void tai_unpack(struct tai *t, const char *s)

   Unpack a TAI from string.

.. c:function:: void taia_tai(const struct taia *ta, struct tai *t)

   Retrieve the TAI seconds in :c:type:`struct taia`.

.. c:function:: void taia_now(struct taia *ta)

   Returns the current time in TAI with atto second precision.

.. note::

   The :c:func:`gettimeofday` function returns nano second precision (at best),
   although :c:type:`struc taia` can hold atto second precision.

.. c:function:: void taia_add(struct taia *ta, const struct taia *a, const struct taia *b)

   Add two TAIA and collect the results in ``t``.

.. c:function:: void taia_sub(struct taia *ta, const struct taia *a, const struct taia *b)

   Substract two TAIA and collect the results in ``t``.

.. c:function:: int  taia_cmp(const struct taia *a, const struct taia *b)

   Compare two TAIA, suitable for most sort functions.

.. c:function:: void taia_pack(const struct taia *ta, char *s)

   Pack a TAIA to string, suitable for disk storage or network usage.

.. c:function:: void taia_unpack(struct taia *ta, const char *s)

   Unpack a TAIA from string.
