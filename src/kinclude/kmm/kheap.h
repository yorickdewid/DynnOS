#ifndef __KHEAP_H
#define __KHEAP_H

#define KHEAP_START         0xC0000000
#define KHEAP_INITIAL_SIZE  0x100000

#define HEAP_INDEX_SIZE   0x20000
#define HEAP_MAGIC        0x123890AB
#define HEAP_MIN_SIZE     0x70000

struct header
{
	unsigned int magic;
	unsigned char is_hole;
	unsigned int size;
};

struct footer
{
	unsigned int magic;
	struct header *header;
};

struct heap
{
	struct ordered_array index;
	unsigned int start_address;
	unsigned int end_address;
	unsigned int max_address;
	unsigned char supervisor;
	unsigned char readonly;
	char flag[8];
};

extern unsigned int kmalloc_int(unsigned int sz, int align, unsigned int *phys);
extern unsigned int kmalloc_a(unsigned int sz);
extern unsigned int kmalloc_p(unsigned int sz, unsigned int *phys);
extern unsigned int kmalloc_ap(unsigned int sz, unsigned int *phys);
extern unsigned int kmalloc(unsigned int sz);
extern struct heap *create_heap(unsigned int start, unsigned int end, unsigned int max, unsigned char supervisor, unsigned char readonly);
extern void *alloc(unsigned int size, unsigned char page_align, struct heap *heap);
extern void free(void *p, struct heap *heap);
extern void kfree(void *p);

#endif
