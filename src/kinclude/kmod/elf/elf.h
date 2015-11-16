#ifndef __ELF_H
#define __ELF_H

struct elf_header;

struct elf_module
{

    struct modules_module base;
    unsigned int (*check)(struct elf_module *module, void *address);

};

extern unsigned int elf_check(void *address);
extern void elf_execute(struct elf_header *header, int argc, char *argv[]);
extern void elf_init();

#endif

