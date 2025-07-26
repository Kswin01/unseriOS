#include <assembly.h>

.global enable_mmu

enable_mmu:
    dsb sy
    msr ttbr0_el1, x0
    msr mair_el1, x1
    msr tcr_el1, x2
    isb

    mrs x3, sctlr_el1
    ldr x4, =SCTLR_SET
    orr x3, x3, x4
    msr sctlr_el1, x3
    isb
    ret