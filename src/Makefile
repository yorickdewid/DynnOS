# Makefile for JamesM's kernel tutorials.
# The C and C++ rules are already setup by default.
# The only one that needs changing is the assembler 
# rule, as we use nasm instead of GNU as.

SOURCES=kbase/init.o \
		kcommon/calls.o \
		kcommon/file.o \
		kcommon/memory.o \
		kcommon/string.o \
		kcommon/math.o \
		kcommon/ordered_table.o \
		kdebug/debug.o \
		kdebug/log.o \
		kmod/elf/elf.o \
		kmod/tty/tty.o \
		kmod/ata/ata.o \
		kmod/io/io.o \
		kmod/kbd/kbd.o \
		kmod/pci/pci.o \
		kmod/pit/pit.o \
		kmod/pit/timer.o \
		kmod/rtc/rtc.o \
		kmod/serial/serial.o \
		kmod/vga/vga.o \
		kcore/initrd.o \
		kcore/kernel.o \
		kcore/modules.o \
		kcore/kshell.o \
		kcore/shell.o \
		kcore/syscall.o \
		kcore/reboot.o \
		kfs/vfs.o \
		kbase/mach.o \
		kbase/mboot.o \
		kbase/cpu.o \
		kbase/ios.o \
		kbase/fpu.o \
		kbase/fpus.o \
		kbase/tss.o \
		kbase/tsss.o \
		kbase/gdt.o \
		kbase/gdts.o \
		kbase/idt.o \
		kbase/idts.o \
		kbase/irq.o \
		kbase/irqs.o \
		kbase/isr.o \
		kbase/isrs.o \
		kmm/kheap.o \
		kmm/paging.o \
		ktask/process.o \
		ktask/task.o \

CFLAGS=-nostdlib -nostdinc -fno-builtin
#CFLAGS = -Wall -Wextra -m32 -pedantic -O0 -std=c99 -finline-functions -fno-stack-protector -nostdinc -ffreestanding -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-unused-value -Wno-pointer-sign -g
LDFLAGS=-Tlink.ld -Map kernel.map
ASFLAGS=-felf

all: $(SOURCES) link run

%.o: %.c
	cc $(CFLAGS) -I kinclude/ -c -o $@ $<
	@echo "CC" "$<"

link:
	ld $(LDFLAGS) -o xex.dynnos $(SOURCES)
	@echo "LD" "$<"

.s.o:
	nasm $(ASFLAGS) $<
	@echo "NASM" "$<"

run:
#	../update_image.sh
#	../run_bochs.sh

clean:
	@rm -f xex.dynnos kernel.map *.txt
	@find . -type f -name "*.o" -exec rm -f {} \;
	@find . -type f -name "*.a" -exec rm -f {} \;
	@find . -type f -name "*~" -exec rm -f {} \;
	@echo "Remove old files"
