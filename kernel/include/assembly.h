#pragma once

#define ASM_BIT(nr) (1 << (nr))

#define BEGIN_FUNC(_name) \
    .global _name ; \
    .type _name, %function ; \
_name:

#define END_FUNC(_name) \
    .size _name, .-_name

// Definitions used in boot.S
#define SCTLR_MMU_EN                ASM_BIT(0)  /* Enable MMU for EL0 and EL1 translation. */
#define SCTLR_ALIGN_CHECK_EN        ASM_BIT(1)  /* Alignment check enable for EL0 and El1. */
#define SCTLR_STAGE1_CACHE          ASM_BIT(2)  /* Stage 1 cacheability control for data acesss.
                                                If cleared all data acess to stage 1 from normal mem
                                                is treated as stage 1 non-cacheable. */
#define SCTLR_SP1_ALIGN_EN           ASM_BIT(3)  /* Stack Pointer alignment check enable. If a load/store
                                                uses the SP as the base address, and the SP is not aligned
                                                to a 16 byte boundary, then a alignment fault exception is
                                                generated. This register is for memory accesses in EL1. */
#define SCTLR_SP0_ALIGN_EN          ASM_BIT(4)  /* Same as above but for EL0. */
#define SCTLR_ICACHE_EN             ASM_BIT(12) /* Stage 1 instruction access cacheability control. */

#define SCTLR_SET SCTLR_MMU_EN | SCTLR_ALIGN_CHECK_EN | SCTLR_STAGE1_CACHE | SCTLR_ICACHE_EN

#define TCR_T0SZ(x)       ((64 - (x)))
#define TCR_T1SZ(x)       ((64 - (x)) << 16)
#define TCR_TxSZ(x)       (TCR_T0SZ(x) | TCR_T1SZ(x))
