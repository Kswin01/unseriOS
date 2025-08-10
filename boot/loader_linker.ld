ENTRY(_start)
SECTIONS {
    . = 0x40100000; /* RAM starts at 0x40000000 but if we ask to load the kernel there, QEMU will not load a DTB */
    .startup . : { build/loader_boot.o(.text) }
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss COMMON) }
    . = ALIGN(8);
    . += 0x40000; /* 16kB of stack memory */
    stack_top = .;
    loader_end = .;
    . = ALIGN(8);
    .kernel :
    {
        __start_kernel = .;
        /* Reserve 100 pages for the kernel */
       . += 0x100000;
        __stop_kernel = .;
    }
    kernel_end = .;
}