#ifndef __SERIAL_H
#define __SERIAL_H

#define SERIAL_COM1	0x03F8
#define SERIAL_COM2	0x02F8
#define SERIAL_COM3	0x03E8
#define SERIAL_COM4	0x02E8

struct serial_device
{
	struct modules_device base;
	unsigned int port;
};

extern void serial_init();

#endif

