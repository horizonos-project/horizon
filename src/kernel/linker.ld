ENTRY(_start)

SECTIONS
{
    . = 1M;

    kernel_start = .;

    .text ALIGN(4K) : {
        *(.multiboot)
        *(.text*)
    }

    .rodata ALIGN(4K) : { *(.rodata*) }

    .data ALIGN(4K)  : { *(.data*) }

    .bss  ALIGN(4K)  : {
        *(COMMON)
        *(.bss*)
        *(COMMON)
    }

    kernel_end = .;

    . = ALIGN(4K);
    . = 2M;

    .initramfs ALIGN(4096) : {
        initramfs_start = .;
        *(.initramfs)
        initramfs_end = .;
    }
}