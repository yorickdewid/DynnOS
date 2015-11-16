/* Link.ld -- Linker script for the kernel - ensure everything goes in the */
/*            Correct place.  */
/*            Original file taken from Bran's Kernel Development */
/*            tutorials: http://www.osdever.net/bkerndev/index.php. */

ENTRY(start)
SECTIONS
{

    .text 0x100000 :
    {
        code = .;
        *(.text)
        . = ALIGN(4096);
    }

    .data :
    {
        data = .;
        *(.data)
        *(.rodata)
        . = ALIGN(4096);
    }

    .bss :
    {
        bss = .;
        *(.bss)
        . = ALIGN(4096);
    }

    end = .;
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}
