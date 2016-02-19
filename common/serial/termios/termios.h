#ifndef _COMMON_SERIAL_TERMIOS_H
#define _COMMON_SERIAL_TERMIOS_H

#include <stdlib.h>

unsigned long get_termios_get_ioctl(void);
unsigned long get_termios_set_ioctl(void);
size_t get_termios_size(void);
int get_termios_speed(void *data);
void set_termios_speed(void *data, int speed);
size_t get_termiox_size(void);
int get_termiox_flow(void *data, int *rts, int *cts, int *dtr, int *dsr);
void set_termiox_flow(void *data, int rts, int cts, int dtr, int dsr);

#endif // _COMMON_SERIAL_TERMIOS_H
