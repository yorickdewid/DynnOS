#ifndef __PAGING_H
#define __PAGING_H

struct page
{
	unsigned int present : 1;
	unsigned int rw : 1;
	unsigned int user : 1;
	unsigned int accessed : 1;
	unsigned int dirty : 1;
	unsigned int unused : 7;
	unsigned int frame : 20;
};

struct page_table
{
	struct page pages[1024];
};

struct page_directory
{
	struct page_table *tables[1024];
	unsigned int tablesPhysical[1024];
    unsigned int physicalAddr;
};

extern void vmm_init();
extern void alloc_frame(struct page *page, int is_kernel, int is_writeable);
extern void dma_frame(struct page *page, int is_kernel, int is_writeable, unsigned int address);
extern void free_frame(struct page *page);
extern void info_page(struct page *page);
extern void info_page_directory(struct page_directory *dir);
extern void physical_memory_use();
extern void switch_page_directory(struct page_directory *new);
extern struct page *get_page(unsigned int address, int make, struct page_directory *dir);
extern struct page_directory *clone_directory(struct page_directory *src);

#endif
