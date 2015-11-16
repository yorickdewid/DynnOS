#include <kconf/config.h>
#include <kcommon/call.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kbase/mach.h>
#include <kdebug/debug.h>

static unsigned short *textmemptr;
static int attrib = 0x0F;
static int csr_x, csr_y;

static void outportb(unsigned short _port, unsigned char _data)
{
	__asm__ volatile("outb %1, %0" : : "dN" (_port), "a" (_data));
}

static void scroll()
{
	unsigned blank, temp;

	blank = 0x20 | (attrib << 8);

	if(csr_y >= 25)
	{
		temp = csr_y - 25 + 1;
		memcpy(textmemptr, textmemptr + temp * 80, (25 - temp) * 80 * 2);
		memsetw(textmemptr + (25 - temp) * 80, blank, 80);

		csr_y = 25 - 1;
	}
}

static void move_csr()
{
	unsigned temp;

	temp = csr_y * 80 + csr_x;

	outportb(0x3D4, 14);
	outportb(0x3D5, temp >> 8);
	outportb(0x3D4, 15);
	outportb(0x3D5, temp);
}

static void cls()
{
	unsigned blank;
	unsigned int i;

	blank = 0x20 | (attrib << 8);

	for(i = 0; i < 25; i++)
	{
		memsetw(textmemptr + i * 80, blank, 80);
	}

	csr_x = 0;
	csr_y = 0;
	move_csr();
}

static void putch(unsigned char c)
{
	unsigned short *where;
	unsigned att = attrib << 8;

	if(c == '\b')
	{
		if(csr_x != 0)
		{
			csr_x--;
			putch(' ');
			csr_x--;
		}
	}
	else if(c == '\t')
	{
		csr_x = (csr_x + 8) & ~(8 - 1);
	}
	else if(c == '\r')
	{
		csr_x = 0;
	}
	else if(c == '\n')
	{
		csr_x = 0;
		csr_y++;
	}
	else if(c >= ' ')
	{
		where = textmemptr + (csr_y * 80 + csr_x);
		*where = c | att;
		csr_x++;
	}

	if(csr_x >= 80)
	{
		csr_x = 0;
		csr_y++;
	}

	scroll();
	move_csr();
}

static void printbullet(unsigned int type)
{
	switch(type)
	{
		case DEBUG_TYPE_SHOW:
			settextcolor(0xF, 0x0);
			kprintf("[");
			settextcolor(0x2, 0x0);
			kprintf("*");
			settextcolor(0xF, 0x0);
			kprintf("] ");
			resettextcolor();
			break;

		case DEBUG_TYPE_INFO:
			settextcolor(0xF, 0x0);
			kprintf("[");
			settextcolor(0x1, 0x0);
			kprintf("*");
			settextcolor(0xF, 0x0);
			kprintf("] ");
			resettextcolor();
			break;

		case DEBUG_TYPE_DEBUG:
			settextcolor(0xF, 0x0);
			kprintf("[");
			settextcolor(0x4, 0x0);
			kprintf("*");
			settextcolor(0xF, 0x0);
			kprintf("] ");
			resettextcolor();
			break;
	}
}
void kprintf(unsigned char *format, ...)
{
	char **arg = (char **) &format;
	char *p;
	int c;
	char buf[20];
     
	arg++;
      
	while((c = *format++) != 0)
	{
		if(c != '%')
		{
			putch(c);
		}else{
			c = *format++;

			switch(c)
			{
				case '+':
					printbullet(DEBUG_TYPE_SHOW);
					break;
				case '!':
					printbullet(DEBUG_TYPE_INFO);
					break;
				case '-':
					printbullet(DEBUG_TYPE_DEBUG);
					break;
				case 'd':
				case 'u':
				case 'x':
					itoa(buf, c, *((int *) arg++));
					p = buf;
					goto string;
					break;  
				case 's':
					p = *arg++;
					if(!p)
					{
						p = "[null]";
					}
     
					string:
					while(*p)
					{
						putch(*p++);
					}
					break;
				default:
					putch(*((int *) arg++));
					break;
			}
		}
	}
}

void settextcolor(unsigned char forecolor, unsigned char backcolor)
{
	attrib = (backcolor << 4) | (forecolor & 0x0F);
}

void resettextcolor()
{
	settextcolor(0x07, 0x0);
}

void kassert(unsigned int condition, char *message)
{
	if(condition)
	{
		return;
	}

	mach_interrupts_disable();

	kprintf("%-Kernel oops: assertion error: %s\n", message);

	for(;;);
}

void kpanic(char *message)
{
	mach_interrupts_disable();

	kprintf("%-Kernel panic: %s\n", message);

	for(;;);
}

void kdbg_init()
{
	textmemptr = (unsigned short *)0xB8000;
	cls();
	resettextcolor();
	kprintf("%+Kernel debugger loaded\n");
}

