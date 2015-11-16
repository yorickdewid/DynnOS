#include <kcommon/memory.h>
#include <kconf/config.h>
#include <kbase/gdt.h>

struct gdt_entry gdt[GDT_TABLE_SIZE];
struct gdt_ptr gdtPointer;

void gdt_set_gate(unsigned char index, unsigned int base, unsigned int limit, unsigned char access, unsigned char granularity)
{
	gdt[index].baseLow = (base & 0xFFFF);
	gdt[index].baseMiddle = (base >> 16) & 0xFF;
	gdt[index].baseHigh = (base >> 24) & 0xFF;
	gdt[index].limitLow = (limit & 0xFFFF);
	gdt[index].granularity = (limit >> 16) & 0x0F;
	gdt[index].granularity |= (granularity & 0xF0);
	gdt[index].access = access;
}

void gdt_init()
{
	memset(&gdt, 0, sizeof (struct gdt_entry) * GDT_TABLE_SIZE);

	gdt_set_gate(0x00, 0x00000000, 0x00000000, 0x00, 0x00); // Null segment
	gdt_set_gate(0x01, 0x00000000, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel code segment
	gdt_set_gate(0x02, 0x00000000, 0xFFFFFFFF, 0x92, 0xCF); // Kernel data segment
	gdt_set_gate(0x03, 0x00000000, 0xFFFFFFFF, 0xFA, 0xCF); // User code segment
	gdt_set_gate(0x04, 0x00000000, 0xFFFFFFFF, 0xF2, 0xCF); // User data segment

	gdtPointer.base = (unsigned int)&gdt;
	gdtPointer.limit = (sizeof (struct gdt_entry) * GDT_TABLE_SIZE) - 1;
	gdt_flush(&gdtPointer);
}

