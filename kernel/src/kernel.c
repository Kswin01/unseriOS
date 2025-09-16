#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// Our local kernel implementations
#include <interrupt.h>
#include <uart.h>
#include <cpu.h>
#include <mem.h>
#include <traps.h>
#include <printf.h>
// Useful information for the generic qemu virt aarch64 platform
#include <qemu_virt_aarch64_registers.h>
#include <gic/gicv3.h>;
// The initial task will be embedded in the kernel ELF
// by the build system.
extern const unsigned char __start_initial_task[];
extern const unsigned char __stop_initial_task[];

void copy_initial_task() {
    size_t size = (size_t)&__stop_initial_task - (size_t)&__start_initial_task;
    memmove(QEMU_PHYS_MEM_START + (QEMU_MEM_SIZE / 2), &__start_initial_task, size);
    asm("dsb sy");
    puts("Finished moving initial task from: ");
    puthex64(&__start_initial_task);
    puts(" to ");
    puthex64(QEMU_PHYS_MEM_START + (QEMU_MEM_SIZE / 2));
    puts("\n");
}

void kmain(void) {
    // We need to setup kernel memory

    // Enable UART device -> all setup for the PL011.
    init_uart(PADDR_TO_KERNEL_VADDR(PL011_UART_REGS));

    puts("kernel_starting...\n\n-------------------- WELCOME TO unseriOS! --------------------\n\n");

    init_cpu();

    init_vector_table();

    init_gic_v3(PADDR_TO_KERNEL_VADDR(QEMU_GICV3_DIST_REGS), PADDR_TO_KERNEL_VADDR(QEMU_GICV3_RDIST_REGS));
    
    timer_init();

    printf("Setting a timer interrupt for rolling 1 second intervals:\n");
    set_timeout(MS_IN_S);
}
