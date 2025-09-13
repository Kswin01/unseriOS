#include <stdio.h>
#include <stdint.h>
#include <util.h>

#include <cpu.h>

/* CPACR_EL1 register */
#define CPACR_EL1_FPEN          20     // FP registers access

void init_cpu() {
    puts("Initialising the CPU!\n");
    /* Init the FPU */
    uint32_t cpacr;
    MRS("cpacr_el1", cpacr);
    cpacr |= (3 << CPACR_EL1_FPEN);
    MSR("cpacr_el1", cpacr);
    asm volatile("isb");
}