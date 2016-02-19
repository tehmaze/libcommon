.. _platform:

platform: Operating System Support
==================================


API
---

.. c:function:: void __winsock_init(void)

   Initialize Winsock support, no-op on other platforms than Windows.

.. c:function:: int __winsock_errno(long error)

   Convert a Winsock errno to a libc compatible errno, no-op on other platforms
   than Windows.
