#include <kconf/config.h>
#include <kdebug/debug.h>
#include <kcore/kernel.h>
#include <kdebug/log.h>
#include <kbase/mach.h>
#include <kbase/cpu.h>
#include <kbase/isr.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kcommon/ordered_table.h>
#include <kmm/kheap.h>
#include <kmm/paging.h>
#include <ktask/process.h>
#include <ktask/task.h>

volatile struct task *current_task;
volatile struct task *ready_queue;

extern struct page_directory *kernel_directory;
extern struct page_directory *current_directory;
extern void alloc_frame(struct page *, int, int);
extern unsigned int kernel_stack_addr;
extern unsigned int read_eip();

unsigned int next_pid = 1;
unsigned int task_counter = 0;

void task_init()
{
	mach_interrupts_disable();

	// Relocate the stack so we know where it is.
	move_stack((void*)0xE0000000, 0x4000);

	// Initialise the first task (kernel task)
	current_task = (struct task *) kmalloc(sizeof(struct task));
	current_task->id = next_pid;
	current_task->next = 0;
	current_task->ebp = 0;
	current_task->esp = 0;
	current_task->eip = 0;
	current_task->page_directory = current_directory;
	current_task->kernel_stack = kmalloc_a(KERNEL_STACK_SIZE);

	ready_queue = current_task;

	next_pid++;

	mach_interrupts_enable();
}

void move_stack(void *new_stack_start, unsigned int size)
{
	unsigned int i;

	//Allocate some space for the new stack.
	for(i = (unsigned int)new_stack_start; i >= ((unsigned int)new_stack_start-size); i -= 0x1000)
	{
		// General-purpose stack is in user-mode.
		alloc_frame(get_page(i, 1, current_directory), 0 /* User mode */, 1 /* Is writable */ );
	}

	//Flush the TLB by reading and writing the page directory address again.
	unsigned int pd_addr;

	__asm__ volatile("mov %%cr3, %0" : "=r" (pd_addr));
	__asm__ volatile("mov %0, %%cr3" : : "r" (pd_addr));

	//Old ESP and EBP, read from registers.
	unsigned int old_stack_pointer; __asm__ volatile("mov %%esp, %0" : "=r" (old_stack_pointer));
	unsigned int old_base_pointer;  __asm__ volatile("mov %%ebp, %0" : "=r" (old_base_pointer));

	//Offset to add to old stack addresses to get a new stack address.
	unsigned int offset = (unsigned int)new_stack_start - kernel_stack_addr;

	//New ESP and EBP
	unsigned int new_stack_pointer = old_stack_pointer + offset;
	unsigned int new_base_pointer = old_base_pointer + offset;

	// Copy the stack
	memcpy((void*)new_stack_pointer, (void*)old_stack_pointer, kernel_stack_addr-old_stack_pointer);

	//Backtrace through the original stack, copying new values into the new stack
	for(i = (unsigned int)new_stack_start; i > (unsigned int)new_stack_start-size; i -= 4)
	{
		unsigned int tmp = * (unsigned int*)i;

		// If the value of tmp is inside the range of the old stack, assume it is a base pointer
		// and remap it. This will unfortunately remap ANY value in this range, whether they are
		// base pointers or not.
		if((old_stack_pointer < tmp) && (tmp < kernel_stack_addr))
		{
			tmp = tmp + offset;
			unsigned int *tmp2 = (unsigned int*)i;
			*tmp2 = tmp;
		}
	}

	//Change stacks
	__asm__ volatile("mov %0, %%esp" : : "r" (new_stack_pointer));
	__asm__ volatile("mov %0, %%ebp" : : "r" (new_base_pointer));
}

void switch_task()
{
	// If we haven't initialised tasking yet, just return.
	if(!current_task)
	{
		return;     
	}

	task_counter++;

	// Read esp, ebp now for saving later on.
	unsigned int esp, ebp, eip;

	__asm__ volatile("mov %%esp, %0" : "=r"(esp));
	__asm__ volatile("mov %%ebp, %0" : "=r"(ebp));

	eip = read_eip();

	// Have we just switched tasks?
	if(eip == 0x1000)
	{
		return;
	}

	// No, we didn't switch tasks. Let's save some register values and switch.
	current_task->eip = eip;
	current_task->esp = esp;
	current_task->ebp = ebp;

	if(task_counter < 2)
	{
		kprintf("%-current_task at 0x%x\n", current_task);
		kprintf("%-current_task->id at 0x%x\n", current_task->id);
		kprintf("%-current_task->next at 0x%x\n", current_task->next);
		kprintf("%-current_task->ebp at 0x%x\n", current_task->ebp);
		kprintf("%-current_task->esp at 0x%x\n", current_task->esp);
		kprintf("%-current_task->eip at 0x%x\n", current_task->eip);
		kprintf("%-current_task->page_directory at 0x%x\n", current_task->page_directory);
		kprintf("%-current_task->kernel_stack at 0x%x\n", current_task->kernel_stack);
	}

	// Get the next task to run.
	current_task = current_task->next;
	// If we fell off the end of the linked list start again at the beginning.
	if(!current_task)
	{
		current_task = ready_queue;
	}

	eip = current_task->eip;
	esp = current_task->esp;
	ebp = current_task->ebp;

	// Make sure the memory manager knows we've changed page directory.
	current_directory = current_task->page_directory;

	// Change our kernel stack over
	mach_set_stack(current_task->kernel_stack + KERNEL_STACK_SIZE);

	if(task_counter < 2)
	{
		kprintf("[%u] CPU: EIP 0x%x ESP 0x%x EBP 0x%x PD 0x%x\n", task_counter, eip, esp, ebp, current_directory->physicalAddr);
	}

	__asm__ volatile("cli");
	__asm__ volatile("mov %0, %%ebx" : : "r" (eip));
	__asm__ volatile("mov %0, %%esp" : : "r" (esp));
	__asm__ volatile("mov %0, %%ebp" : : "r" (ebp));
	__asm__ volatile("mov %0, %%cr3" : : "r" (current_directory->physicalAddr));
	__asm__ volatile("mov $0x1000, %eax");
	__asm__ volatile("sti");
	__asm__ volatile("jmp *%ebx");
}

int fork() // throw new task in ready_queue
{
    // We are modifying kernel structures
    cpu_interrupts_off();

    // Take a pointer to this process' task struct for later reference.
    struct task *parent_task = (struct task*)current_task;

    // Clone the address space.
    struct page_directory *directory = clone_directory(current_directory);

    // Create a new process.
    struct task *new_task = (struct task *) kmalloc(sizeof(struct task));

    new_task->id = next_pid++;
    new_task->ebp = 0;
    new_task->esp = 0;
    new_task->eip = 0;
    new_task->page_directory = directory;
    current_task->kernel_stack = kmalloc_a(KERNEL_STACK_SIZE);
    new_task->next = 0;

    // Add it to the end of the ready queue.
    struct task *tmp_task = (struct task *) ready_queue;
    while (tmp_task->next)
    {
        tmp_task = tmp_task->next;
      }
    tmp_task->next = new_task;

    // This will be the entry point for the new process.
    unsigned int eip = read_eip();


    // We could be the parent or the child here - check.
    if (current_task == parent_task)
    {
        // We are the parent, so set up the esp/ebp/eip for our child.
        unsigned int esp;
        unsigned int ebp;
        __asm__ volatile("mov %%esp, %0" : "=r"(esp));
        __asm__ volatile("mov %%ebp, %0" : "=r"(ebp));
        new_task->esp = esp;
        new_task->ebp = ebp;
        new_task->eip = eip;
        cpu_interrupts_on();

        return new_task->id;
    }
    else
    {
        // We are the child.
        return 0;
    }

}

int get_pid()
{
    return current_task->id;
}

void switch_to_user_mode()
{
    // Set up our kernel stack.
    mach_set_stack(current_task->kernel_stack+KERNEL_STACK_SIZE);
    
    // Set up a stack structure for switching to user mode.
    __asm__ volatile("  \
      cli; \
      mov $0x23, %ax; \
      mov %ax, %ds; \
      mov %ax, %es; \
      mov %ax, %fs; \
      mov %ax, %gs; \
                    \
       \
      mov %esp, %eax; \
      pushl $0x23; \
      pushl %esp; \
      pushf; \
      pushl $0x1B; \
      push $1f; \
      iret; \
    1: \
      "); 
      
}
