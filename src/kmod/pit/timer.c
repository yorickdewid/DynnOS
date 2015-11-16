#include <kconf/config.h>
#include <kcommon/call.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kdebug/debug.h>
#include <kfs/vfs.h>
#include <kcore/modules.h>
#include <kmod/pit/pit.h>
#include <kmod/pit/timer.h>

void timer_wait(int sec)
{
	unsigned int eticks = get_pit() + (sec * PIT_FREQUENCY);
	while(get_pit() < eticks);
}
