#include "common/config.h"
#include "common/platform.h"
#include "common/serial/termios/termios.h"

#if defined(HAVE_TERMIOS)

#include <linux/termios.h>

PRIVATE unsigned long get_termios_get_ioctl(void)
{
#ifdef HAVE_STRUCT_TERMIOS2
	return TCGETS2;
#else
	return TCGETS;
#endif
}

PRIVATE unsigned long get_termios_set_ioctl(void)
{
#ifdef HAVE_STRUCT_TERMIOS2
	return TCSETS2;
#else
	return TCSETS;
#endif
}

PRIVATE size_t get_termios_size(void)
{
#ifdef HAVE_STRUCT_TERMIOS2
	return sizeof(struct termios2);
#else
	return sizeof(struct termios);
#endif
}

#if (defined(HAVE_TERMIOS_SPEED) || defined(HAVE_TERMIOS2_SPEED)) && defined(HAVE_BOTHER)
PRIVATE int get_termios_speed(void *data)
{
#ifdef HAVE_STRUCT_TERMIOS2
	struct termios2 *term = (struct termios2 *) data;
#else
	struct termios *term = (struct termios *) data;
#endif
	if (term->c_ispeed != term->c_ospeed)
		return -1;
	else
		return term->c_ispeed;
}

PRIVATE void set_termios_speed(void *data, int speed)
{
#ifdef HAVE_STRUCT_TERMIOS2
	struct termios2 *term = (struct termios2 *) data;
#else
	struct termios *term = (struct termios *) data;
#endif
	term->c_cflag &= ~CBAUD;
	term->c_cflag |= BOTHER;
	term->c_ispeed = term->c_ospeed = speed;
}
#endif

#ifdef HAVE_STRUCT_TERMIOX
PRIVATE size_t get_termiox_size(void)
{
	return sizeof(struct termiox);
}

PRIVATE int get_termiox_flow(void *data, int *rts, int *cts, int *dtr, int *dsr)
{
	struct termiox *termx = (struct termiox *) data;
	int flags = 0;

	*rts = (termx->x_cflag & RTSXOFF);
	*cts = (termx->x_cflag & CTSXON);
	*dtr = (termx->x_cflag & DTRXOFF);
	*dsr = (termx->x_cflag & DSRXON);

	return flags;
}

PRIVATE void set_termiox_flow(void *data, int rts, int cts, int dtr, int dsr)
{
	struct termiox *termx = (struct termiox *) data;

	termx->x_cflag &= ~(RTSXOFF | CTSXON | DTRXOFF | DSRXON);

	if (rts)
		termx->x_cflag |= RTSXOFF;
	if (cts)
		termx->x_cflag |= CTSXON;
	if (dtr)
		termx->x_cflag |= DTRXOFF;
	if (dsr)
		termx->x_cflag |= DSRXON;
}
#endif

#endif // HAVE_TERMIOS
