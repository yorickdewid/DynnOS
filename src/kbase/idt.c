#include <kcommon/memory.h>
#include <kconf/config.h>
#include <kbase/idt.h>

struct idt_entry idt[IDT_TABLE_SIZE];
struct idt_ptr idtPointer;

void idt_set_gate(unsigned char index, void (*callback)(), unsigned short selector, unsigned char flags)
{
	unsigned int base = (unsigned int)callback;

	idt[index].baseLow = (base & 0xFFFF);
	idt[index].baseHigh = (base >> 16) & 0xFFFF;
	idt[index].selector = selector;
	idt[index].zero = 0;
	idt[index].flags = flags | 0x60;
}

void idt_init()
{
	memset(&idt, 0, sizeof (struct idt_entry) * IDT_TABLE_SIZE);

	idtPointer.base = (unsigned int)&idt;
	idtPointer.limit = (sizeof (struct idt_entry) * IDT_TABLE_SIZE) - 1;
	idt_flush(&idtPointer);
}

