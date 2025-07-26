#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

// Our local kernel implementations
#include <interrupt.h>
#include <uart.h>
#include <cpu.h>
#include <mem.h>
#include <traps.h>

// Useful information for the generic qemu virt aarch64 platform
#include <qemu_virt_aarch64_registers.h>

void kmain(void) {
    // We need to setup kernel memory
    // kernel_mem_init();

    // Enable UART device -> all setup for the PL011.
    init_uart((PL011_UART_REGS));
    // kernel_mem_init();
    puts("kernel_starting...\n\n-------------------- WELCOME TO unseriOS! --------------------\n\n");

    init_vector_table();

    // Enable the Generic Interrupt Controller
    init_gic_v3(PADDR_TO_KERNEL_VADDR(QEMU_GICV3_DIST_REGS), PADDR_TO_KERNEL_VADDR(QEMU_GICV3_RDIST_REGS));

    // Deal with CPU state
    init_cpu();
}
