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
 * Description:	Prekernel main program, provides functions for upper kernel
 * Date:		20-01-2012
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
#include <kdebug/debug.h>
#include <kdebug/log.h>
#include <kcore/kernel.h>
#include <kbase/ios.h>
#include <kbase/tss.h>
#include <kbase/cpu.h>
#include <kbase/fpu.h>
#include <kbase/gdt.h>
#include <kbase/idt.h>
#include <kbase/isr.h>
#include <kbase/irq.h>
#include <kbase/mboot.h>
#include <kmm/paging.h>
#include <kcore/syscall.h>
#include <kbase/mach.h>

unsigned int kernel_stack_addr;

static void mach_init_base()
{
	log_init();
	gdt_init();
	tss_init();
	idt_init();
	fpu_init();
	irq_init();
	isr_init();
	vmm_init();
	syscall_init();

	cpu_interrupts_on();
}

void mach_init(struct mboot_info *header, unsigned int magic, unsigned int stack_addr)
{
	kernel_stack_addr = stack_addr;
	
	kdbg_init();
	kprintf("%!Kernel entry point [HIT]\n");
	kprintf("%-Kernel stack 0x%x to 0x%x\n", kernel_stack_addr, (kernel_stack_addr - 0x4000));

	mach_init_base();

	kassert(magic == MBOOT_MAGIC, "MBOOT_MAGIC is not correct");
	kassert(header->mods_count, "Module does not exist");

	kernel_set_initrd((unsigned int *)header->mods_addr);
	kinit();
}

void mach_interrupts_disable()
{
	cpu_interrupts_off();
}

void mach_interrupts_enable()
{
	cpu_interrupts_on();
}

void mach_reboot()
{
	cpu_interrupts_off();

	unsigned char ready = 0x02;

	while((ready & 0x02) != 0)
	{
		ready = io_inb(0x64);
	}

	io_outb(0x64, 0xFE);
}

void mach_halt()
{
	cpu_halt();
}

void mach_set_stack(unsigned int address)
{
	tss_set_stack(address);
}

void mach_usermode(unsigned int address)
{
	cpu_usermode(address);
}

