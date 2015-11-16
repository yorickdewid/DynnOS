#ifndef __TASK_H
#define __TASK_H

#define KERNEL_STACK_SIZE 2048

struct task
{
	int id;
	unsigned int ebp;
	unsigned int esp;
	unsigned int eip;
	struct page_directory *page_directory;
	unsigned int kernel_stack;
	struct task *next;
};

extern void task_init();
extern void switch_task();
extern int fork();
extern void move_stack(void *new_stack_start, unsigned int size);
extern int get_pid();
extern void switch_to_user_mode();

#endif
