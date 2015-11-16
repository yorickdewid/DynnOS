#include <kconf/config.h>
#include <kcommon/elf_str.h>
#include <kcommon/file.h>
#include <kcommon/string.h>
#include <kfs/vfs.h>
#include <kcore/modules.h>
#include <kmod/elf/elf.h>
#include <kbase/mach.h>
#include <kbase/isr.h>
#include <kmm/paging.h>
#include <kcore/syscall.h>

static void *syscallRoutines[SYSCALL_ROUTINES_SIZE];

void syscall_register_handler(unsigned char index, void (*handler)(struct syscall_registers *registers))
{
	syscallRoutines[index] = handler;
}

void syscall_unregister_handler(unsigned char index)
{
	syscallRoutines[index] = 0;
}

static void syscall_open(struct syscall_registers *registers)
{
	registers->eax = vfs_open((char *)registers->esi);
}

static void syscall_close(struct syscall_registers *registers)
{
	vfs_close(registers->ebx);

	registers->eax = 0;
}

static void syscall_read(struct syscall_registers *registers)
{
	unsigned int index = registers->ebx;
	char *buffer = (char *)registers->esi;
	unsigned int count = registers->ecx;

	struct vfs_node *node = vfs_get(index);

	if(!node || !node->read)
	{
		registers->eax = 0;
		return;
	}

	registers->eax = node->read(node, 0, count, buffer);
}

static void syscall_write(struct syscall_registers *registers)
{
	unsigned int index = registers->ebx;
	char *buffer = (char *)registers->esi;
	unsigned int count = registers->ecx;

	struct vfs_node *node = vfs_get(index);

	if(!node || !node->write)
	{
		registers->eax = 0;
		return;
	}

	registers->eax = node->write(node, 0, count, buffer);
}

static void syscall_info(struct syscall_registers *registers)
{
	char *name = (char *)registers->esi;
	struct file_info *info = (struct file_info *)registers->edi;
    
	struct vfs_node *file = vfs_find_root(name);

	if(file)
	{
		info->id = file->id;
		strcpy(info->name, file->name);
		info->length = file->length;

		registers->eax = 1;
		return;
	}

	registers->eax = 0;
}

static void syscall_map(struct syscall_registers *registers)
{
	void *address = (void *)registers->ebx;

	if(!elf_check(address))
	{
		registers->eax = 0;
		return;
	}

	struct elf_header *header = (struct elf_header *)address;
	struct elf_program_header *programHeader = (struct elf_program_header *)(address + header->programHeaderOffset);

	unsigned int index = (programHeader->virtualAddress / 0x1000) / 1024;

	//struct mmu_directory *kernelDirectory = mmu_get_kernel_directory();
	//struct mmu_table *programTable = mmu_get_program_table();

	//mmu_clear_table(programTable);

	//mmu_add_table(kernelDirectory, index, programTable, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE);
	//mmu_map(kernelDirectory, programHeader->virtualAddress, registers->ebx, programHeader->memorySize, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);
	// //mmu_set_directory(kernelDirectory);

	registers->eax = header->entry;
}

void syscall_handler(struct syscall_registers *registers)
{
	void (*handler)(struct syscall_registers *registers) = syscallRoutines[registers->eax];

	if(handler)
	{
		handler(registers);
	}
}

void syscall_init()
{
	syscall_register_handler(SYSCALL_ROUTINE_OPEN, syscall_open);
	syscall_register_handler(SYSCALL_ROUTINE_CLOSE, syscall_close);
	syscall_register_handler(SYSCALL_ROUTINE_READ, syscall_read);
	syscall_register_handler(SYSCALL_ROUTINE_WRITE, syscall_write);
	syscall_register_handler(SYSCALL_ROUTINE_INFO, syscall_info);
	syscall_register_handler(SYSCALL_ROUTINE_MAP, syscall_map);
}

