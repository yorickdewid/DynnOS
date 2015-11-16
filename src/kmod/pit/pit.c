#include <kconf/config.h>
#include <kcommon/call.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kdebug/debug.h>
#include <kfs/vfs.h>
#include <kcore/modules.h>
#include <ktask/task.h>
#include <kbase/irq.h>
#include <kbase/isr.h>
#include <kbase/ios.h>
#include <kmod/pit/pit.h>

unsigned int pit_counter = 0;

static void pit_handler(struct isr_registers *registers)
{
	pit_counter++;
	switch_task();
}

unsigned int get_pit()
{
	return pit_counter;
}

static void pit_enable()
{
	unsigned short divisor = PIT_HERTZ / PIT_FREQUENCY;

	io_outb(0x43, 0x36);
	io_outb(0x40, (unsigned char)(divisor & 0xFF));
	io_outb(0x40, (unsigned char)((divisor >> 8) & 0xFF));
}

void pit_init()
{
	irq_register_handler(IRQ_ROUTINE_PIT, pit_handler);
	pit_enable();
}

