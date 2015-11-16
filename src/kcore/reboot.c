#include <kconf/config.h>
#include <kbase/mach.h>
#include <kcore/reboot.h>
#include <kcore/syscall.h>

void syscall_reboot()
{
	mach_reboot();
}

