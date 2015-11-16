#ifndef __MBOOT_H
#define __MBOOT_H

#define MBOOT_FLAG_MEMORY	1 << 0
#define MBOOT_FLAG_DEVICE	1 << 1
#define MBOOT_FLAG_CMDLINE	1 << 2
#define MBOOT_FLAG_MODULES	1 << 3
#define MBOOT_FLAG_AOUT		1 << 4
#define MBOOT_FLAG_ELF		1 << 5
#define MBOOT_FLAG_MMAP		1 << 6
#define MBOOT_FLAG_CONFIG	1 << 7
#define MBOOT_FLAG_LOADER	1 << 8
#define MBOOT_FLAG_APM		1 << 9
#define MBOOT_FLAG_VBE		1 << 10

#define MBOOT_MAGIC		0x2BADB002

struct mboot_info
{
	unsigned int flags;
	unsigned int mem_lower;
	unsigned int mem_upper;
	unsigned int boot_device;
	unsigned int cmdline;
	unsigned int mods_count;
	unsigned int mods_addr;
	unsigned int num;
	unsigned int size;
	unsigned int addr;
	unsigned int shndx;
	unsigned int mmap_length;
	unsigned int mmap_addr;
	unsigned int drives_length;
	unsigned int drives_addr;
	unsigned int config_table;
	unsigned int boot_loader_name;
	unsigned int apm_table;
	unsigned int vbe_control_info;
	unsigned int vbe_mode_info;
	unsigned int vbe_mode;
	unsigned int vbe_interface_seg;
	unsigned int vbe_interface_off;
	unsigned int vbe_interface_len;
} __attribute__((packed));

struct mboot_mmap
{
	unsigned int size;
	unsigned int baseLow;
	unsigned int baseHigh;
	unsigned int lengthLow;
	unsigned int lengthHigh;
	unsigned int type;
};

struct mboot_module
{
	unsigned int base;
	unsigned int length;
	unsigned int name;
	unsigned int reserved;
};

extern void multiboot_dump(struct mboot_info *mboot_ptr);
extern void boot_info(struct mboot_info *mboot_ptr);

#endif
