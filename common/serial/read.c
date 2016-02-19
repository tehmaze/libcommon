#include "common/debug.h"
#include "common/serial.h"
#include "common/serial/internal.h"

#if !defined(PLATFORM_WINDOWS) && !defined(TIOCINQ)
#define TIOCINQ 0x541b
#endif

#if defined(PLATFORM_WINDOWS)
/** To be called after port receive buffer is emptied. */
PRIVATE static int restart_wait(serial_t *port)
{
	DWORD wait_result;

	if (port->wait_running) {
		/* Check status of running wait operation. */
		if (GetOverlappedResult(port->hdl, &port->wait_ovl,
				&wait_result, FALSE)) {
			DEBUG("previous wait completed");
			port->wait_running = FALSE;
		} else if (GetLastError() == ERROR_IO_INCOMPLETE) {
			DEBUG("previous wait still running");
			RETURN_OK();
		} else {
			RETURN_FAIL("GetOverlappedResult() failed");
		}
	}

	if (!port->wait_running) {
		/* Start new wait operation. */
		if (WaitCommEvent(port->hdl, &port->events,
				&port->wait_ovl)) {
			DEBUG("new wait returned, events already pending");
		} else if (GetLastError() == ERROR_IO_PENDING) {
			DEBUG("new wait running in background");
			port->wait_running = TRUE;
		} else {
			RETURN_FAIL("WaitCommEvent() failed");
		}
	}

	RETURN_OK();
}

/* Restart wait operation if buffer was emptied. */
PRIVATE static int restart_wait_if_needed(serial_t *port, unsigned int bytes_read)
{
	DWORD errors;
	COMSTAT comstat;

	if (bytes_read == 0)
		RETURN_OK();

	if (ClearCommError(port->hdl, &errors, &comstat) == 0)
		RETURN_FAIL("ClearCommError() failed");

	if (comstat.cbInQue == 0)
		TRY(restart_wait(port));

	RETURN_OK();
}
#endif

PRIVATE int serial_read(serial_t *port, void *buf, size_t len, unsigned int timeout_ms)
{
	CHECK_OPEN_PORT();

	if (!buf)
		RETURN_ERROR(EINVAL, "NULL buffer");

	if (timeout_ms)
		DEBUGF("reading %d bytes from port %s, timeout %d ms",
			len, port->name, timeout_ms);
	else
		DEBUGF("reading %d bytes from port %s, no timeout",
			len, port->name);

	if (len == 0)
		RETURN_INT(0);

#if defined(PLATFORM_WINDOWS)
	DWORD bytes_read = 0;

	/* Set timeout. */
	if (port->timeouts.ReadIntervalTimeout != 0 ||
			port->timeouts.ReadTotalTimeoutMultiplier != 0 ||
			port->timeouts.ReadTotalTimeoutConstant != timeout_ms) {
		port->timeouts.ReadIntervalTimeout = 0;
		port->timeouts.ReadTotalTimeoutMultiplier = 0;
		port->timeouts.ReadTotalTimeoutConstant = timeout_ms;
		if (SetCommTimeouts(port->hdl, &port->timeouts) == 0)
			RETURN_FAIL("SetCommTimeouts() failed");
	}

	/* Start read. */
	if (ReadFile(port->hdl, buf, len, NULL, &port->read_ovl)) {
		DEBUG("read completed immediately");
		bytes_read = len;
	} else if (GetLastError() == ERROR_IO_PENDING) {
		DEBUG("waiting for read to complete");
		if (GetOverlappedResult(port->hdl, &port->read_ovl, &bytes_read, TRUE) == 0)
			RETURN_FAIL("GetOverlappedResult() failed");
		DEBUGF("Read completed, %d/%d bytes read", bytes_read, len);
	} else {
		RETURN_FAIL("ReadFile() failed");
	}

	TRY(restart_wait_if_needed(port, bytes_read));

	RETURN_INT(bytes_read);

#else
	size_t bytes_read = 0;
	unsigned char *ptr = (unsigned char *) buf;
	struct timeval start, delta, now, end = {0, 0};
	int started = 0;
	fd_set fds;
	int result;

	if (timeout_ms) {
		/* Get time at start of operation. */
		gettimeofday(&start, NULL);
		/* Define duration of timeout. */
		delta.tv_sec = timeout_ms / 1000;
		delta.tv_usec = (timeout_ms % 1000) * 1000;
		/* Calculate time at which we should give up. */
		timeradd(&start, &delta, &end);
	}

	FD_ZERO(&fds);
	FD_SET(port->fd, &fds);

	/* Loop until we have the requested number of bytes. */
	while (bytes_read < len) {
		/*
		 * Check timeout only if we have run select() at least once,
		 * to avoid any issues if a short timeout is reached before
		 * select() is even run.
		 */
		if (timeout_ms && started) {
			gettimeofday(&now, NULL);
			if (timercmp(&now, &end, >))
				/* Timeout has expired. */
				break;
			timersub(&end, &now, &delta);
		}
		result = select(port->fd + 1, &fds, NULL, NULL, timeout_ms ? &delta : NULL);
		started = 1;
		if (result < 0) {
			if (errno == EINTR) {
				DEBUG("select() call was interrupted, repeating");
				continue;
			} else {
				RETURN_ERRNO("select() failed");
			}
		} else if (result == 0) {
			/* Timeout has expired. */
			break;
		}

		/* Do read. */
		result = read(port->fd, ptr, len - bytes_read);

		if (result < 0) {
			if (errno == EAGAIN)
				/*
				 * This shouldn't happen because we did a
				 * select() first, but handle anyway.
				 */
				continue;
			else
				/* This is an actual failure. */
				RETURN_ERRNO("read() failed");
		}

		bytes_read += result;
		ptr += result;
	}

	if (bytes_read < len)
		DEBUG("read timed out");

	RETURN_INT(bytes_read);
#endif
}

PRIVATE int serial_read_next(serial_t *port, void *buf, size_t len, unsigned int timeout_ms)
{
	CHECK_OPEN_PORT();

	if (!buf)
		RETURN_ERROR(EINVAL, "NULL buffer");

	if (len == 0)
		RETURN_ERROR(EINVAL, "zero len");

	if (timeout_ms)
		DEBUGF("reading next max %d bytes from port %s, timeout %d ms",
			len, port->name, timeout_ms);
	else
		DEBUGF("reading next max %d bytes from port %s, no timeout",
			len, port->name);

#if defined(PLATFORM_WINDOWS)
	DWORD bytes_read = 0;

	/* If timeout_ms == 0, set maximum timeout. */
	DWORD timeout_val = (timeout_ms == 0 ? MAXDWORD - 1 : timeout_ms);

	/* Set timeout. */
	if (port->timeouts.ReadIntervalTimeout != MAXDWORD ||
			port->timeouts.ReadTotalTimeoutMultiplier != MAXDWORD ||
			port->timeouts.ReadTotalTimeoutConstant != timeout_val) {
		port->timeouts.ReadIntervalTimeout = MAXDWORD;
		port->timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
		port->timeouts.ReadTotalTimeoutConstant = timeout_val;
		if (SetCommTimeouts(port->hdl, &port->timeouts) == 0)
			RETURN_FAIL("SetCommTimeouts() failed");
	}

	/* Loop until we have at least one byte, or timeout is reached. */
	while (bytes_read == 0) {
		/* Start read. */
		if (ReadFile(port->hdl, buf, len, &bytes_read, &port->read_ovl)) {
			DEBUG("read completed immediately");
		} else if (GetLastError() == ERROR_IO_PENDING) {
			DEBUG("waiting for read to complete");
			if (GetOverlappedResult(port->hdl, &port->read_ovl, &bytes_read, TRUE) == 0)
				RETURN_FAIL("GetOverlappedResult() failed");
			if (bytes_read > 0) {
				DEBUG("read completed");
			} else if (timeout_ms > 0) {
				DEBUG("read timed out");
				break;
			} else {
				DEBUG("restarting read");
			}
		} else {
			RETURN_FAIL("ReadFile() failed");
		}
	}

	TRY(restart_wait_if_needed(port, bytes_read));

	RETURN_INT(bytes_read);

#else
	size_t bytes_read = 0;
	struct timeval start, delta, now, end = {0, 0};
	int started = 0;
	fd_set fds;
	int result;

	if (timeout_ms) {
		/* Get time at start of operation. */
		gettimeofday(&start, NULL);
		/* Define duration of timeout. */
		delta.tv_sec = timeout_ms / 1000;
		delta.tv_usec = (timeout_ms % 1000) * 1000;
		/* Calculate time at which we should give up. */
		timeradd(&start, &delta, &end);
	}

	FD_ZERO(&fds);
	FD_SET(port->fd, &fds);

	/* Loop until we have at least one byte, or timeout is reached. */
	while (bytes_read == 0) {
		/*
		 * Check timeout only if we have run select() at least once,
		 * to avoid any issues if a short timeout is reached before
		 * select() is even run.
		 */
		if (timeout_ms && started) {
			gettimeofday(&now, NULL);
			if (timercmp(&now, &end, >))
				/* Timeout has expired. */
				break;
			timersub(&end, &now, &delta);
		}
		result = select(port->fd + 1, &fds, NULL, NULL, timeout_ms ? &delta : NULL);
		started = 1;
		if (result < 0) {
			if (errno == EINTR) {
				DEBUG("select() call was interrupted, repeating");
				continue;
			} else {
				RETURN_FAIL("select() failed");
			}
		} else if (result == 0) {
			/* Timeout has expired. */
			break;
		}

		/* Do read. */
		result = read(port->fd, buf, len);

		if (result < 0) {
			if (errno == EAGAIN)
				/* This shouldn't happen because we did a select() first, but handle anyway. */
				continue;
			else
				/* This is an actual failure. */
				RETURN_FAIL("read() failed");
		}

		bytes_read = result;
	}

	if (bytes_read == 0)
		DEBUG("read timed out");

	RETURN_INT(bytes_read);
#endif
}

PRIVATE int serial_read_nonblock(serial_t *port, void *buf, size_t len)
{
	CHECK_OPEN_PORT();

	if (!buf)
		RETURN_ERROR(EINVAL, "NULL buffer");

	DEBUGF("reading up to %d bytes from port %s", len, port->name);

#if defined(PLATFORM_WINDOWS)
	DWORD bytes_read;

	/* Set timeout. */
	if (port->timeouts.ReadIntervalTimeout != MAXDWORD ||
			port->timeouts.ReadTotalTimeoutMultiplier != 0 ||
			port->timeouts.ReadTotalTimeoutConstant != 0) {
		port->timeouts.ReadIntervalTimeout = MAXDWORD;
		port->timeouts.ReadTotalTimeoutMultiplier = 0;
		port->timeouts.ReadTotalTimeoutConstant = 0;
		if (SetCommTimeouts(port->hdl, &port->timeouts) == 0)
			RETURN_FAIL("SetCommTimeouts() failed");
	}

	/* Do read. */
	if (ReadFile(port->hdl, buf, len, NULL, &port->read_ovl) == 0)
		if (GetLastError() != ERROR_IO_PENDING)
			RETURN_FAIL("ReadFile() failed");

	/* Get number of bytes read. */
	if (GetOverlappedResult(port->hdl, &port->read_ovl, &bytes_read, FALSE) == 0)
		RETURN_FAIL("GetOverlappedResult() failed");

	TRY(restart_wait_if_needed(port, bytes_read));

	RETURN_INT(bytes_read);
#else
	ssize_t bytes_read;

	/* Returns the number of bytes read, or -1 upon failure. */
	if ((bytes_read = read(port->fd, buf, len)) < 0) {
		if (errno == EAGAIN)
			/* No bytes available. */
			bytes_read = 0;
		else
			/* This is an actual failure. */
			RETURN_FAIL("read() failed");
	}
	RETURN_INT(bytes_read);
#endif
}

PRIVATE int serial_read_waiting(serial_t *port)
{
	CHECK_OPEN_PORT();

	DEBUGF("checking input bytes waiting on port %s", port->name);

#if defined(PLATFORM_WINDOWS)
	DWORD errors;
	COMSTAT comstat;

	if (ClearCommError(port->hdl, &errors, &comstat) == 0)
		RETURN_FAIL("ClearCommError() failed");
	RETURN_INT(comstat.cbInQue);
#else
	int bytes_waiting;
	if (ioctl(port->fd, TIOCINQ, &bytes_waiting) < 0)
		RETURN_FAIL("TIOCINQ ioctl failed");
	RETURN_INT(bytes_waiting);
#endif
}
