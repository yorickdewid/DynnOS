/*
 * DynnOS Operating System
 * Project DynnOS, Quenza
 * Copyright (c) 2010-2012
 * All rights reserved.
 *
 * All files, published and unpublished, are property of
 * the DynnOS poject. Source is reserved for official
 * developers only. Project developers are named in the
 * next section.
 *
 * Developed by: Dinux, Team17, team1717 and Quenza Development Team
 *
 * Description:	Kernel main program, calls all functions within the upper kernel
 * Date:		17-01-2012
 * Timestamp:	03-05-2012
 * DUID:		{Y17-IFEE4R53-R3FC3-3R}->?
 *
 * = Simple Quenza Common License 2012 =
 *
 * All files, published and unpublished, are property of the named owner
 * and / or the copyright holders. It is forbidden to use, change, copy,
 * modify, merge, publish, distribute, promote, sublicense, and/or sell
 * anything that can be called software or is related to information technology
 * relevant to the Software. Software as defined here includes source code
 * executable code, documentation and / or data.
 * Unless specified otherwise by the licensor, the following conditions
 * must apply at all time:
 * 1)	Each component, file, program, document, documentation and associated
 *		projects must include the official QCL or SQCL license, while the
 *		license must be supplied separately.
 * 2)	Each project / Software must have at least one Quenza Unique Identity code
 *		(QUID) which can be obtained from the licensor. The QUID need to be
 *		mentioned in the concerning project / Software files
 *		and the overall description.
 * 3)	The project / Software description must contain	the creation date
 *		and recent timestamp.
 * 4)	The copyright notice and this permission notice shall be
 *		included in all copies or substantial portions of the Software.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * For additional information see DynnOS.com/license
 */

#include <kconf/config.h>
#include <kcommon/call.h> // for temp usermode
#include <kcommon/file.h> // for temp usermode
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kfs/vfs.h>
#include <kcore/modules.h>
#include <kcore/initrd.h>
#include <kcore/kernel.h>
#include <kdebug/log.h>
#include <kdebug/debug.h>
#include <kcore/shell.h>
#include <kbase/mach.h>
#include <kbase/isr.h>
#include <kmod/pit/timer.h>
#include <kmod/kbd/kbd.h>
#include <kmm/paging.h>
#include <ktask/task.h>

unsigned int *ramdisk_addr;

extern unsigned int code;
extern unsigned int data;
extern unsigned int bss;
extern unsigned int end;

extern int task_counter;

void kernel_set_initrd(unsigned int *address)
{
	ramdisk_addr = address;
}

void kinit()
{
	vfs_init();

	//DEBUG
	kprintf("%+Parse ramdisk at 0x%x\n", ramdisk_addr);

	initrd_init(ramdisk_addr);

	modules_init();

	task_init();

	kshell_init();

	//mach_usermode((unsigned int)blub);
	//DEBUG
	kprintf("%!Switch to ring 3\n");
	
	for(;;);
}

void blub(){
	int sin = file_open("/dev/kbd");
	int sout = file_open("/dev/tty");
	char c = ' ';
	unsigned int i;

	for(i = 2000; i; i--)
	{
		file_write_byte(FILE_STDOUT, c);
	}
	file_write_string(FILE_STDOUT, "DynnOS 2012\n");
	file_write_string(FILE_STDOUT, "Copyright (c) 2012\n");
	file_write_string(FILE_STDOUT, "Ring 3\n");
}
