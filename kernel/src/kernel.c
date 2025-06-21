#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

// Our local kernel implementations
#include <interrupt.h>
#include <uart.h>
#include <cpu.h>

// Useful information for the generic qemu virt aarch64 platform
#include <qemu_virt_aarch64_registers.h>

void kmain(void) {
    // Enable UART device -> all setup for the PL011.
    init_uart(PL011_UART_REGS);

    puts("kernel_starting...\nWELCOME TO unseriOS!\n\n");

    init_vector_table();

    // Enable the Generic Interrupt Controller
    init_gic_v3(QEMU_GICV3_REGS);

    // Deal with CPU state: Disable FPU, init exception vector tables, initialise IRQ controller
    init_cpu();

    // We need to setup kernel memory: kernel virtual memory, stack pointer
    // Setup kernel paging structures. Flush caches and invalidate TLB
    kernel_mem_init();
}
