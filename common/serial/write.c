#include "common/debug.h"
#include "common/serial.h"
#include "common/serial/internal.h"

PRIVATE int serial_write(serial_t *port, const void *buf, size_t len, unsigned int timeout_ms)
{
	CHECK_OPEN_PORT();

	if (!buf)
		RETURN_ERROR(EINVAL, "NULL buffer");

	if (timeout_ms)
		DEBUGF("writing %d bytes to port %s, timeout %d ms",
			len, port->name, timeout_ms);
	else
		DEBUGF("writing %d bytes to port %s, no timeout",
			len, port->name);

	if (len == 0)
		RETURN_INT(0);

#ifdef _WIN32
	DWORD bytes_written = 0;
	BOOL result;

	/* Wait for previous non-blocking write to complete, if any. */
	if (port->writing) {
		DEBUG("waiting for previous write to complete");
		result = GetOverlappedResult(port->hdl, &port->write_ovl, &bytes_written, TRUE);
		port->writing = 0;
		if (!result)
			RETURN_FAIL("previous write failed to complete");
		DEBUG("previous write completed");
	}

	/* Set timeout. */
	if (port->timeouts.WriteTotalTimeoutConstant != timeout_ms) {
		port->timeouts.WriteTotalTimeoutConstant = timeout_ms;
		if (SetCommTimeouts(port->hdl, &port->timeouts) == 0)
			RETURN_FAIL("SetCommTimeouts() failed");
	}

	/* Start write. */
	if (WriteFile(port->hdl, buf, len, NULL, &port->write_ovl)) {
		DEBUG("write completed immediately");
		RETURN_INT(len);
	} else if (GetLastError() == ERROR_IO_PENDING) {
		DEBUG("waiting for write to complete");
		if (GetOverlappedResult(port->hdl, &port->write_ovl, &bytes_written, TRUE) == 0) {
			if (GetLastError() == ERROR_SEM_TIMEOUT) {
				DEBUG("write timed out");
				RETURN_INT(0);
			} else {
				RETURN_FAIL("GetOverlappedResult() failed");
			}
		}
		DEBUGF("write completed, %d/%d bytes written", bytes_written, len);
		RETURN_INT(bytes_written);
	} else {
		RETURN_FAIL("WriteFile() failed");
	}
#else
	size_t bytes_written = 0;
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

	/* Loop until we have written the requested number of bytes. */
	while (bytes_written < len) {
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
		result = select(port->fd + 1, NULL, &fds, NULL, timeout_ms ? &delta : NULL);
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

		/* Do write. */
		result = write(port->fd, ptr, len - bytes_written);

		if (result < 0) {
			if (errno == EAGAIN)
				/* This shouldn't happen because we did a select() first, but handle anyway. */
				continue;
			else
				/* This is an actual failure. */
				RETURN_FAIL("write() failed");
		}

		bytes_written += result;
		ptr += result;
	}

	if (bytes_written < len)
		DEBUG("write timed out");

	RETURN_INT(bytes_written);
#endif
}

PRIVATE int serial_write_nonblock(serial_t *port, const void *buf, size_t len)
{
	CHECK_OPEN_PORT();

	if (!buf)
		RETURN_ERROR(EINVAL, "NULL buffer");

	DEBUGF("writing up to %d bytes to port %s", len, port->name);

	if (len == 0)
		RETURN_INT(0);

#ifdef _WIN32
	DWORD written = 0;
	BYTE *ptr = (BYTE *) buf;

	/* Check whether previous write is complete. */
	if (port->writing) {
		if (HasOverlappedIoCompleted(&port->write_ovl)) {
			DEBUG("previous write completed");
			port->writing = 0;
		} else {
			DEBUG("previous write not complete");
			/* Can't take a new write until the previous one finishes. */
			RETURN_INT(0);
		}
	}

	/* Set timeout. */
	if (port->timeouts.WriteTotalTimeoutConstant != 0) {
		port->timeouts.WriteTotalTimeoutConstant = 0;
		if (SetCommTimeouts(port->hdl, &port->timeouts) == 0)
			RETURN_FAIL("SetCommTimeouts() failed");
	}

	/*
	 * Keep writing data until the OS has to actually start an async IO
	 * for it. At that point we know the buffer is full.
	 */
	while (written < len) {
		/* Copy first byte of user buffer. */
		port->pending_byte = *ptr++;

		/* Start asynchronous write. */
		if (WriteFile(port->hdl, &port->pending_byte, 1, NULL, &port->write_ovl) == 0) {
			if (GetLastError() == ERROR_IO_PENDING) {
				if (HasOverlappedIoCompleted(&port->write_ovl)) {
					DEBUG("asynchronous write completed immediately");
					port->writing = 0;
					written++;
					continue;
				} else {
					DEBUG("asynchronous write running");
					port->writing = 1;
					RETURN_INT(++written);
				}
			} else {
				/* Actual failure of some kind. */
				RETURN_FAIL("WriteFile() failed");
			}
		} else {
			DEBUG("single byte written immediately");
			written++;
		}
	}

	DEBUG("all bytes written immediately");

	RETURN_INT(written);
#else
	/* Returns the number of bytes written, or -1 upon failure. */
	ssize_t written = write(port->fd, buf, len);

	if (written < 0) {
		RETURN_FAIL("write() failed");
	} else {
		RETURN_INT(written);
    }
#endif
}
