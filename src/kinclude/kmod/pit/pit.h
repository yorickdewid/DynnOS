#ifndef __PIT_H
#define __PIT_H

#define PIT_HERTZ	1193180
#define PIT_FREQUENCY	100

extern unsigned int get_pit();
extern void pit_init();

#endif

