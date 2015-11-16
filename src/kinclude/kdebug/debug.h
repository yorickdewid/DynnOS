#ifndef __DEBUG_H
#define __DEBUG_H

#define DEBUG_TYPE_SHOW		1
#define DEBUG_TYPE_INFO		2
#define DEBUG_TYPE_DEBUG	4

extern void kprintf(unsigned char *format, ...);
extern void settextcolor(unsigned char forecolor, unsigned char backcolor);
extern void resettextcolor();
extern void kassert(unsigned int condition, char *message);
extern void kpanic(char *message);
extern void kdbg_init();

#endif

