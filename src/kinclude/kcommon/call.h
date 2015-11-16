#ifndef __CALL_H
#define __CALL_H

struct file_info;

extern int call_open(char *name);
extern void call_close(unsigned int index);
extern unsigned int call_read(unsigned int index, void *buffer, unsigned int count);
extern unsigned int call_write(unsigned int index, void *buffer, unsigned int count);
extern unsigned int call_info(char *name, struct file_info *info);
extern int call_map(unsigned int address);
extern int call_halt();
extern int call_reboot();

#endif

