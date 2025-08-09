ENTRY(_start)
SECTIONS {
    . = 0x40100000; /* RAM starts at 0x40000000 but if we ask to load the kernel there, QEMU will not load a DTB */
    .startup . : { build/boot.o(.text) }
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss COMMON) }
    . = ALIGN(8);
    . += 0x40000; /* 16kB of stack memory */
    stack_top = .;
    kernel_end = .;
    . = ALIGN(8);
    .initial_task :
    {
        __start_initial_task = .;
        KEEP(*(.initial_task))
        __stop_initial_task = .;
    }
    initial_task_end = .;
}