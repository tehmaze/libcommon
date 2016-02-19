.. _debug:

debug: Debug Helpers
====================


API
---

.. c:var:: void (*debug_handler)(const char *fmt, ...)

   Debug handler function.

.. c:macro:: DEBUG(msg)

   Print a debug message.

.. c:macro:: DEBUGF(fmt, ...)

   Print a debug message with string formatting.

.. c:macro:: DEBUG_ERROR(err,msg)

   Print a debug error message.

.. c:macro:: RETURN_CODE(x)

   Print a debug message for numeric returns.

.. c:macro:: RETURN_CODEVAL(x)

   Print a debug message for libc compatible errors.

.. c:macro:: RETURN_OK()

   Print a debug message for successful returns.

.. c:macro:: RETURN_ERRNO(msg,...)

   Print a debug message for libc compatible errors with string formatting.
