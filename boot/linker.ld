ENTRY(_start)
SECTIONS {
    . = 0xffffff8080000000; /* This is where the kernel will be in virtual memory */
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