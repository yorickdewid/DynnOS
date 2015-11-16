#ifndef __MACH_H
#define __MACH_H

struct mboot_info;

extern void mach_init(struct mboot_info *header, unsigned int magic, unsigned int stack_addr);
extern void mach_interrupts_disable();
extern void mach_interrupts_enable();
extern void mach_reboot();
extern void mach_halt();
extern void mach_set_stack(unsigned int address);
extern void mach_usermode(unsigned int address);

#endif
