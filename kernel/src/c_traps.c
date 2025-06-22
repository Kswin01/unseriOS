#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

// Our local kernel implementations
#include <interrupt.h>
#include <uart.h>
#include <cpu.h>
#include <traps.h>
#include <util.h>

// Useful information for the generic qemu virt aarch64 platform
#include <qemu_virt_aarch64_registers.h>


// Make this function visibile for the assembly to call into, and ensure that this is NOT inlined.
void __attribute__((externally_visible)) __attribute__((noinline)) kernel_halt(void) {
    puts("halting kernel!\n");
    while(1) {
        asm("nop");
    }
}

void __attribute__((externally_visible)) __attribute__((noinline)) sync_abort (void) {
    puts("We received a synchronous abort!\n");
    uint32_t esr;
    MRS("esr_el1", esr);
    puts("This is the esr: ");
    puthex32(esr);
    puts("\n");
    kernel_halt();
}

void __attribute__((externally_visible)) __attribute__((noinline)) async_abort (void) {
    // ARM SError's refer to aynchronous aborts
    puts("We received an asynchronous abort!\n");
    kernel_halt();
}


void __attribute__((externally_visible)) __attribute__((noinline)) handle_interrupt (void) {
    puts("We received an interrupt!\n");
    kernel_halt();
}

void init_vector_table(void) {
    puts("Initialising vector tables!\n");
    asm("dsb sy");

    MSR("vbar_el1", (uintptr_t)arm_vector_table);

    asm("isb");
    puts("Finished initialisng vector tables!\n");
}