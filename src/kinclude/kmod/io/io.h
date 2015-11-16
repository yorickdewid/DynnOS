#ifndef __IO_H
#define __IO_H

struct io_device
{
	struct modules_device base;
};

extern void io_init();

#endif
