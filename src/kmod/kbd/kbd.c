#include <kconf/config.h>
#include <kcommon/call.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kdebug/debug.h>
#include <kfs/vfs.h>
#include <kcore/modules.h>
#include <kbase/irq.h>
#include <kbase/isr.h>
#include <kmm/paging.h>
#include <kbase/ios.h>
#include <kmod/kbd/kbd.h>

static char kbdMapUS[256] =
{
       0,   27,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '+', '\b', '\t',
     'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n',    0,  'a',  's',
     'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',  '`',    0, '\\',  'z',  'x',  'c',  'v',
     'b',  'n',  'm',  ',',  '.',  '/',    0,    0,    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,   27,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '=', '\b', '\t',
     'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}', '\n',    0,  'A',  'S',
     'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':', '\"',  '~',    0,  '|',  'Z',  'X',  'C',  'V',
     'B',  'N',  'M',  '<',  '>',  '?',    0,    0,    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
};

static struct kbd_device kbdDevice;

static unsigned int kbd_device_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	char c = 0;

	if(kbdDevice.bufferHead != kbdDevice.bufferTail)
	{
		c = kbdDevice.buffer[kbdDevice.bufferTail];
		kbdDevice.bufferTail = (kbdDevice.bufferTail + 1) % KBD_BUFFER_SIZE;
	}

	if(c)
	{
		((char *)buffer)[0] = c;

		return 1;
	}

	return 0;
}

static unsigned int kbd_device_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	if((kbdDevice.bufferHead + 1) % KBD_BUFFER_SIZE != kbdDevice.bufferTail)
	{
		kbdDevice.buffer[kbdDevice.bufferHead] = ((char *)buffer)[0];
		kbdDevice.bufferHead = (kbdDevice.bufferHead + 1) % KBD_BUFFER_SIZE;

		return 1;
	}
	return 0;
}

static void kbd_handler(struct isr_registers *registers)
{
	unsigned char scancode = io_inb(KBD_PORT_READ);

	if(scancode == 0x2A || scancode == 0x36)
	{
		kbdDevice.toggleShift = 1;
	}

	if(scancode == 0xAA || scancode == 0xB6)
	{
		kbdDevice.toggleShift = 0;
	}

	if(scancode == 0x3A)
	{
		if(kbdDevice.toggleCaps)
		{
			kbdDevice.toggleCaps = 0;
			kbdDevice.toggleShift = 0;
		}
		else
		{
			kbdDevice.toggleCaps = 1;
			kbdDevice.toggleShift = 1;
		}
	}

	if(scancode & 0x80)
	{
		// Break codes
	}else{
		if(kbdDevice.toggleShift)
		{
			scancode += 128;
		}
            
		kbdDevice.base.node.write(&kbdDevice.base.node, 0, 1, &kbdMapUS[scancode]);
	}
}

unsigned char kbd_buffer()
{
	char c = 0;

	if(kbdDevice.bufferHead != kbdDevice.bufferTail)
	{
		c = kbdDevice.buffer[kbdDevice.bufferTail];
		kbdDevice.bufferTail = (kbdDevice.bufferTail + 1) % KBD_BUFFER_SIZE;
	}

	if(c)
	{
		return c;
	}

	return 0;
}

void kbd_update_leds(char status){

 	while((io_inb(0x64) & 2) != 0){} //loop until zero
 	io_outb(0x60, 0xED);
 
 	while((io_inb(0x64) & 2) != 0){} //loop until zero
 	io_outb(0x60, status);
}

void kbd_init()
{
	kbdDevice.bufferHead = 0;
	kbdDevice.bufferTail = 0;
	kbdDevice.toggleAlt = 0;
	kbdDevice.toggleCtrl = 0;
	kbdDevice.toggleShift = 0;

	strcpy(kbdDevice.base.node.name, "kbd");
	kbdDevice.base.node.length = 0;
	kbdDevice.base.node.read = kbd_device_read;
	kbdDevice.base.node.write = kbd_device_write;

	struct vfs_node *devNode = vfs_find_root("/dev");
	devNode->write(devNode, devNode->length, 1, &kbdDevice.base.node);

	irq_register_handler(IRQ_ROUTINE_KBD, kbd_handler);

	modules_register_device(MODULES_DEVICE_TYPE_KEYBOARD, &kbdDevice.base);
}

