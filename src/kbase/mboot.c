#include <kconf/config.h>
#include <kdebug/debug.h>
#include <kbase/mboot.h>

void multiboot_dump(struct mboot_info *mboot_ptr)
{
	kprintf("MULTIBOOT header at 0x%x:\n", (unsigned int) mboot_ptr);
	kprintf("Flags 0x%x\n", mboot_ptr->flags);
	kprintf("Mem lower 0x%x\n", mboot_ptr->mem_lower);
	kprintf("Mem upper 0x%x\n", mboot_ptr->mem_upper);
	kprintf("Boot device 0x%x\n", mboot_ptr->boot_device);
	kprintf("cmdline 0x%x\n", mboot_ptr->cmdline);
	kprintf("Mod count 0x%x\n", mboot_ptr->mods_count);
	kprintf("Mod addr 0x%x\n", mboot_ptr->mods_addr);
	kprintf("Syms num 0x%x\n", mboot_ptr->num);
	kprintf("Syms size 0x%x\n", mboot_ptr->size);
	kprintf("Syms addr 0x%x\n", mboot_ptr->addr);
	kprintf("Syms shndx 0x%x\n", mboot_ptr->shndx);
	kprintf("MMap length 0x%x\n", mboot_ptr->mmap_length);
	kprintf("MMap addr 0x%x\n", mboot_ptr->mmap_addr);
	kprintf("Drives length 0x%x\n", mboot_ptr->drives_length);
	kprintf("Drives addr 0x%x\n", mboot_ptr->drives_addr);
	kprintf("Config table 0x%x\n", mboot_ptr->config_table);
	kprintf("Loader name 0x%x\n", mboot_ptr->boot_loader_name);
	kprintf("APM table 0x%x\n", mboot_ptr->apm_table);
	kprintf("VBE Control 0x%x\n", mboot_ptr->vbe_control_info);
	kprintf("VBE Mode 0x%x\n", mboot_ptr->vbe_mode_info);
	kprintf("VBE interface 0x%x\n", mboot_ptr->vbe_mode);
	kprintf("VBE segment 0x%x\n", mboot_ptr->vbe_interface_seg);
	kprintf("VBE offset 0x%x\n", mboot_ptr->vbe_interface_off);
	kprintf("VBE length 0x%x\n", mboot_ptr->vbe_interface_len);
}

void boot_info(struct mboot_info *mboot_ptr)
{
	if(mboot_ptr->flags & (1 << 2))
	{
		kprintf("Boot paramaters: %s\n", (char *) mboot_ptr->cmdline);
	}
	if(mboot_ptr->flags & (1 << 9))
	{
		kprintf("Loader name: %s\n", (char *) mboot_ptr->boot_loader_name);
	}
	if(mboot_ptr->flags & (1 << 0))
	{
		int mem_mb = ((mboot_ptr->mem_upper+mboot_ptr->mem_lower) / 1024);
		kprintf("Passed physical RAM %dMb\n", mem_mb);
	}
	if(mboot_ptr->flags & (1 << 3))
	{
		kprintf("Passed %d modules:\n", mboot_ptr->mods_count);
		if(mboot_ptr->mods_count > 0)
		{
			unsigned int i;
			for(i = 0; i < mboot_ptr->mods_count; ++i)
			{
				int module_start = *((int*)mboot_ptr->mods_addr + 8 * i);
				int module_end = *(int*)(mboot_ptr->mods_addr + 8 * i + 4);
				kprintf("  -Module [%d] at 0x%x::0x%x\n", i+1, module_start, module_end);
			}
		}
	}
}
