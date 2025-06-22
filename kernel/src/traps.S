/* This file sets up the vector table and also defines the entry points for the all the exception vectors. */

/* Fun bit of info that I didnt know: .s files indicate files that *don't* need pre-processing,
whereas .S files indicate that they do. */


#include <assembly.h>

/* Kindly borrowing these macros from seL4. */
#define ELR     elr_el1
#define ESR     esr_el1
#define SPSR    spsr_el1
#define TPIDR   tpidr_el1

.macro lsp_i _tmp
    mrs     \_tmp, TPIDR
    mov     sp, \_tmp
.endm

.macro ventry label
.align 7
    b       \label
.endm

.section .vectors, "ax"

/* We are only going to implement the current exception levels vector entries. */
BEGIN_FUNC(arm_vector_table)
    ventry  invalid_vector_entry           // Synchronous EL1t/EL2t
    ventry  invalid_vector_entry           // IRQ EL1t/EL2t
    ventry  invalid_vector_entry           // FIQ EL1t/EL2t
    ventry  invalid_vector_entry           // SError EL1t/EL2t

    ventry  priv_el_sync                    // Current EL Synchronous (EL1/2)
    ventry  priv_el_irq                     // IRQ
    ventry  invalid_vector_entry           // FIQ
    ventry  priv_el_async                    // SError

    ventry  invalid_vector_entry                  // Synchronous 64-bit EL0/EL1
    ventry  invalid_vector_entry                   // IRQ 64-bit EL0/EL1
    ventry  invalid_vector_entry           // FIQ 64-bit EL0/EL1
    ventry  invalid_vector_entry                  // SError 64-bit EL0/EL1

    ventry  invalid_vector_entry           // Synchronous 32-bit EL0/EL1
    ventry  invalid_vector_entry           // IRQ 32-bit EL0/EL1
    ventry  invalid_vector_entry           // FIQ 32-bit EL0/EL1
    ventry  invalid_vector_entry           // SError 32-bit EL0/EL1
END_FUNC(arm_vector_table)

.section .vectors.text, "ax"
/* Here is where seL4 also defines a macro for kernel entry. Essentially storing a thread's stack frame. */

BEGIN_FUNC(invalid_vector_entry)
    b kernel_halt
END_FUNC(invalid_vector_entry)

BEGIN_FUNC(priv_el_sync)
    // We should read the exception syndrome register (esr) here.
    b sync_abort
END_FUNC(priv_el_sync)

BEGIN_FUNC(priv_el_irq)
    b handle_interrupt
END_FUNC(priv_el_irq)

BEGIN_FUNC(priv_el_async)
    b async_abort
END_FUNC(priv_el_async)
