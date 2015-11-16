#ifndef __KBD_H
#define __KBD_H

#define KBD_PORT_READ	0x60
#define KBD_BUFFER_SIZE	256

#define KBD_RESET	0
#define KBD_SCROLL_LED	1
#define KBD_NUM_LED	2
#define KBD_CAPS_LED	4
 
struct kbd_device
{
	struct modules_device base;
	char buffer[KBD_BUFFER_SIZE];
	unsigned int bufferSize;
	unsigned int bufferHead;
	unsigned int bufferTail;
	unsigned char toggleAlt;
	unsigned char toggleCtrl;
	unsigned char toggleShift;
	unsigned char toggleCaps;
};

extern unsigned char kbd_buffer();
extern void kbd_update_leds(char status);
extern void kbd_init();

#endif

