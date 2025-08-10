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

// Useful information for the generic qemu virt aarch64 platform
#include <qemu_virt_aarch64_registers.h>

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
    init_uart((PL011_UART_REGS));

    puts("kernel_starting...\n\n-------------------- WELCOME TO unseriOS! --------------------\n\n");

    // We want to copy the initial task out into the top 1GiB
    copy_initial_task();
    // Setup a kernel identity map first, so we don't completely bork the PC
    // setup_ident_map();
    // Initialise our proper page tables, and enable the mmu
    // kernel_mem_init();
    // We will reset the program counter and stack in kernel_mem_init,
    // and trampoline to kmain continued
}

void kmain_cont(void) {
    init_uart(PADDR_TO_KERNEL_VADDR(PL011_UART_REGS));

    puts("kernel continuing with mmu now enabled...\n\n");

    init_vector_table();
    // Enable the Generic Interrupt Controller
    init_gic_v3(PADDR_TO_KERNEL_VADDR(QEMU_GICV3_DIST_REGS), PADDR_TO_KERNEL_VADDR(QEMU_GICV3_RDIST_REGS));

    // Deal with CPU state
    init_cpu();
}