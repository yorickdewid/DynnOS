#include <kconf/config.h>
#include <kdebug/debug.h>
#include <kcore/kernel.h>
#include <kdebug/log.h>
#include <kbase/cpu.h>
#include <kbase/isr.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kcommon/ordered_table.h>
#include <ktask/process.h>
#include <kmm/kheap.h>
#include <kmm/paging.h>

/* macro definitions */
#define FRAME_FROM_ADDR(a) (a/4096)
#define INDEX_FROM_BIT(a) (a/(32))
#define OFFSET_FROM_BIT(a) (a%(32))

extern unsigned int placement_address;
extern struct heap *kheap;

/* page directories */
struct page_directory *kernel_directory = 0;
struct page_directory *current_directory = 0;

/* frameset frame and counter */
unsigned int *frames;
unsigned int nframes;

/* set bit in bitset */
static void set_frame(unsigned int frame_addr)
{
	unsigned int frame = FRAME_FROM_ADDR(frame_addr);
	unsigned int idx = INDEX_FROM_BIT(frame);
	unsigned int off = OFFSET_FROM_BIT(frame);
	frames[idx] |= (0x1 << off);
}

/* clear bit in bitset */
static void clear_frame(unsigned int frame_addr)
{
	unsigned int frame = FRAME_FROM_ADDR(frame_addr);
	unsigned int idx = INDEX_FROM_BIT(frame);
	unsigned int off = OFFSET_FROM_BIT(frame);
	frames[idx] &= ~(0x1 << off);
}

/* test bit in bitset */
static unsigned int test_frame(unsigned int frame_addr)
{
	unsigned int frame = FRAME_FROM_ADDR(frame_addr);
	unsigned int idx = INDEX_FROM_BIT(frame);
	unsigned int off = OFFSET_FROM_BIT(frame);
	return (frames[idx] & (0x1 << off));
}

/* first free physical frame */
static int first_frame()
{
	unsigned int i, j;
	unsigned int check;

	for(i = 0; i < INDEX_FROM_BIT(nframes); i++)
	{
		if(frames[i] != 0xFFFFFFFF)
		{
			for(j = 0; j < 32; j++)
			{
				check = 0x1 << j;
				if(!(frames[i] & check))
				{
					return i*32+j;
				}
			}
		}
	}
	return -1;
}

void alloc_frame(struct page *page, int is_kernel, int is_writeable)
{
	if(page->frame != 0)
	{
		return;
	}
	else
	{
		unsigned int idx = first_frame();
		if(idx == (unsigned int)-1)
		{
			kpanic("MAX FRAME FAULT");
		}
		set_frame(idx * 0x1000);
		page->present = 1;
		page->rw = (is_writeable==1) ? 1:0;
		page->user = (is_kernel==1) ? 0:1;
		page->frame = idx;
	}
}

void dma_frame(struct page *page, int is_kernel, int is_writeable, unsigned int address)
{
	page->present = 1;
	page->rw = (is_writeable==1) ? 1:0;
	page->user = (is_kernel==1) ? 0:1;
	page->frame = address / 0x1000;
}

void free_frame(struct page *page)
{
	unsigned int frame = page->frame;
	if(!frame)
	{
		return;
	}
	else
	{
		clear_frame(frame);
		page->frame = 0x0;
	}
}

void info_page(struct page *page)
{
	if(!page->frame)
	{
		kprintf("%-frame is not allocated\n");
		return;
	}
	else
	{
		kprintf("<< Page options for 0x%x\n", page->frame);
		kprintf("<< Present 0x%x\n", page->present);
		kprintf("<< RW 0x%x\n", page->rw);
		kprintf("<< User mode 0x%x\n", page->user);
		kprintf("<< Accessed 0x%x\n", page->accessed);
		kprintf("<< Dirty 0x%x\n", page->dirty);
		kprintf("<< Unused 0x%x\n", page->unused);
	}
}

void info_page_directory(struct page_directory *dir)
{
	if(!dir->physicalAddr)
	{
		kprintf("%-No directory mapped\n");
		return;
	}
	else
	{
		kprintf("<< Directory options for 0x%x\n", dir);
		kprintf("<< Physical access at 0x%x\n", dir->physicalAddr);

		int i;
		for(i = 0; i < 1024; i++)
		{
			if(dir->tables[i])
			{	
				if(kernel_directory->tables[i] == dir->tables[i])
				{
					kprintf("<< [%u] Kernel table at phys 0x%x\n", i, dir->tablesPhysical[i]);
				}
				else
				{
					kprintf("<< [%u] Table at phys 0x%x\n", i, dir->tablesPhysical[i]);
				}
			}
		}
	}
}

void physical_memory_use()
{
	unsigned int i, j;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int check;

	for(i = 0; i < INDEX_FROM_BIT(nframes); i++)
	{
		if(frames[i] != 0xFFFFFFFF)
		{
			for(j = 0; j < 32; j++)
			{
				check = 0x1 << j;
				if(frames[i] & check)
				{
					x++;
				}
				else if(!(frames[i] & check))
				{
					y++;
				}
			}
		}
	}

	kprintf("<< Physical memory usage:\n");
	kprintf("<< Used\t%u kb\n", (x * 4));
	kprintf("<< Free\t%u kb\n", (y * 4));
	kprintf("<< Total\t%u kb\n", (nframes * 4));
}

static void page_fault(struct isr_registers regs)
{
	unsigned int fault_addr = cpu_get_cr2();

	// Find out what happend
	int page_present = !(regs.error & 0x1);
	int page_rw = regs.error & 0x2;
	int page_us = regs.error & 0x4;
	int page_reserved = regs.error & 0x8;
	int page_id = regs.error & 0x10;

	kprintf("[VMM Fault] ");
	if(page_present)
	{
		kprintf("<present>");
	}
	if(page_rw)
	{
		kprintf("<read-only>");
	}
	if(page_us)
	{
		kprintf("<user-mode>");
	}
    if(page_reserved)
    {
    	kprintf("<reserved>");
    }
	kprintf(" at 0x%x\n", fault_addr);
	kpanic("PAGING FAULT\n");
}

void vmm_init()
{
	// 16MB physical memory
	unsigned int mem_end_page = 0x1000000;

	// seup frameset
	nframes = FRAME_FROM_ADDR(mem_end_page);
	frames = (unsigned int *) kmalloc(INDEX_FROM_BIT(nframes));
	memset(frames, NULL, INDEX_FROM_BIT(nframes));

	// setup kernel page directory
	unsigned int phys;
	kernel_directory = (struct page_directory *) kmalloc_a(sizeof(struct page_directory));
	memset(kernel_directory, NULL, sizeof(struct page_directory));
	kernel_directory->physicalAddr = (unsigned int) kernel_directory->tablesPhysical;

	// preallocate heap pages
	unsigned int i = 0;
	for(i = KHEAP_START; i < KHEAP_START + KHEAP_INITIAL_SIZE; i += 0x1000)
	{
		get_page(i, TRUE, kernel_directory);
	}

	i = 0;
	while(i < placement_address + 0x1000)
	{
		alloc_frame(get_page(i, TRUE, kernel_directory), FALSE, FALSE);
		i += 0x1000;
	}

	// allocate heap
	for(i = KHEAP_START; i < KHEAP_START + KHEAP_INITIAL_SIZE; i += 0x1000)
	{
		alloc_frame(get_page(i, TRUE, kernel_directory), FALSE, FALSE);
	}

	isr_register_handler(ISR_ROUTINE_PF, (struct isr_registers *) page_fault);

	switch_page_directory(kernel_directory);

	// create heap
	kheap = create_heap(KHEAP_START, KHEAP_START+KHEAP_INITIAL_SIZE, 0xCFFFF000, FALSE, FALSE);

	current_directory = clone_directory(kernel_directory);
	switch_page_directory(current_directory);
}

void switch_page_directory(struct page_directory *dir)
{
	current_directory = dir;

	cpu_set_cr3(dir->physicalAddr);
	cpu_set_cr0(cpu_get_cr0() | 0x80000000);
}

struct page *get_page(unsigned int address, int make, struct page_directory *dir)
{
	address /= 0x1000;
	unsigned int table_idx = address / 1024;
	unsigned int tmp;

	if(dir->tables[table_idx])
	{
		return &dir->tables[table_idx]->pages[address % 1024];
	}
	else if(make)
	{	
		dir->tables[table_idx] = (struct page_table *) kmalloc_ap(sizeof(struct page_table), &tmp);
		memset(dir->tables[table_idx], NULL, 0x1000);
		dir->tablesPhysical[table_idx] = tmp | 0x7;
		return &dir->tables[table_idx]->pages[address % 1024];
	}
	else
	{
		return FALSE;
	}
}

static struct page_table *clone_table(struct page_table *src, unsigned int *physAddr)
{
	struct page_table *table = (struct page_table *) kmalloc_ap(sizeof(struct page_table), physAddr);
	memset(table, NULL, sizeof(struct page_directory));

	int i;
	for(i = 0; i < 1024; i++)
	{
		if(src->pages[i].frame)
		{
			alloc_frame(&table->pages[i], FALSE, FALSE);
			if(src->pages[i].present)
			{
				table->pages[i].present = 1;
			}
			if(src->pages[i].rw)
			{
				table->pages[i].rw = 1;
			}
			if(src->pages[i].user)
			{
				table->pages[i].user = 1;
			}
			if(src->pages[i].accessed)
			{
				table->pages[i].accessed = 1;
			}
			if(src->pages[i].dirty)
			{
				table->pages[i].dirty = 1;
			}
			copy_page_physical(src->pages[i].frame * 0x1000, table->pages[i].frame * 0x1000);
		}
	}
	return table;
}

struct page_directory *clone_directory(struct page_directory *src)
{
	unsigned int phys;
	struct page_directory *dir = (struct page_directory *) kmalloc_ap(sizeof(struct page_directory), &phys);
	memset(dir, NULL, sizeof(struct page_directory));

	// Get the offset of tablesPhysical
	unsigned int offset = (unsigned int) dir->tablesPhysical - (unsigned int) dir;
	dir->physicalAddr = phys + offset;

	int i;
	for(i = 0; i < 1024; i++)
	{
		if(!src->tables[i])
		{	
			continue;
		}
		if(kernel_directory->tables[i] == src->tables[i])
		{
			// page is in kernel, link it
			dir->tables[i] = src->tables[i];
			dir->tablesPhysical[i] = src->tablesPhysical[i];
		}
		else
		{
			// copy table
			unsigned int phys2;
			dir->tables[i] = clone_table(src->tables[i], &phys2);
			dir->tablesPhysical[i] = phys2 | 0x07;
		}
	}
	return dir;
}
