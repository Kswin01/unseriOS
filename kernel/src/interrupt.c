/* Setup and use of the ARM GICv3 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <interrupt.h>
#include <util.h>
#include <gic/gic_common.h>
#include <gic/gicv3.h>
// TODO: Find a better way of including platform specific information
#include <qemu_virt_aarch64_registers.h>

#define MPIDR_AFF0(x) (x & 0xff)
#define MPIDR_AFF1(x) ((x >> 8) & 0xff)
#define MPIDR_AFF2(x) ((x >> 16) & 0xff)
#define MPIDR_AFF3(x) ((x >> 32) & 0xff)

/* Wait for completion of a distributor change */
static uint32_t gicv3_do_wait_for_rwp(volatile uint32_t *ctlr_addr)
{
    puts("Waiting for distributor to start up!\n");
    uint32_t val;
    bool waiting = true;
    uint32_t ret = 0;

    uint64_t gpt_cnt_tval = 0;
    uint32_t deadline_ms =  GIC_DEADLINE_MS;
    uint64_t gpt_cnt_ciel;

    /* Check the value before reading the generic timer */
    val = *ctlr_addr;
    if (!(val & GICR_CTLR_RWP_BIT)) {
        puts("Returning early?\n");
        return 0;
    }
    MRS("cntpct_el0", gpt_cnt_tval);
    gpt_cnt_ciel = gpt_cnt_tval + (deadline_ms * TICKS_PER_MS);

    while (waiting) {
        MRS("cntpct_el0", gpt_cnt_tval);
        val = *ctlr_addr;

        if (gpt_cnt_tval >= gpt_cnt_ciel) {
            puts("GICV3 RWP Timeout after ");
            puthex64(deadline_ms);
            puts(" ms\n");
            ret = 1;
            waiting = false;

        } else if (!(val & GICR_CTLR_RWP_BIT)) {
            ret = 0;
            waiting = false;
        }
    }
    puts("Distributor has started up!\n");
    return ret;
}

void init_gic_dist(uint64_t gic_regs) {
    // Initialise the global GIC Distributor interface - This sequence is based on the seL4 GIC setup.
    // Disable Group 0, secure group 1 and non-secure group 1 interrupts.
    // NOTE: We could just set this register to 0
    CLR_BIT_REG_MASK(gic_regs + GICD_CTLR, CTLR_ENABLE_G0_BIT | CTLR_ENABLE_G1S_BIT | CTLR_ENABLE_G1NS_BIT);

    // Enable affinity routing for non secure and secure state.
    SET_BIT_REG_MASK(gic_regs + GICD_CTLR, CTLR_ARE_NS_BIT | CTLR_ARE_NS_BIT);

    // Setup the SPI's

    // The type register includes the maximum number of interrupt ID's that is supported by the
    // GIC implementation.
    uint32_t type;
    READ_REG(type, gic_regs + GICD_TYPER);

    // From GIC architecture spec:
    // "If the value of this field is N, the maximum SPI INTID is 32(N+1) minus 1."
    uint32_t nr_lines = 32 * ((type & TYPER_IT_LINES_NO_MASK) + 1);

    // The SGI's are reserved before the SPI's.
    for (int i = MIN_SPI_ID, j = 0; i < nr_lines; i += 16, j++) {
        // For each register in the line configuration,
        // 2 bits are reserved for each interrupt ID.
        // Therefore we will increment this loop by 16.

        // We are assuming that all interrupts are initially level sensitive.
        WRITE_REG_UINT32(gic_regs + GICD_ICFGR + (sizeof(uint32_t) * j), 0);
    }

    // Set the prios of the global interrupts
    for (int i = MIN_SPI_ID, j = 0; i < nr_lines; i += 4, j++) {
        // For each interrupt priority register, we contain information for 4
        // interrupt id's. We will therefore increment this loop by 4.
        WRITE_REG_UINT32(gic_regs + GICD_IPRIORITYR + (sizeof(uint32_t) * j), GICD_IPRIORITYR_DEF_VAL);
    }

    // Disable and clear all interrupts
    for (int i = MIN_SPI_ID, j = 0; i < nr_lines; i += 32, j++) {
        // For each interrupt priority register, we contain information for 32
        // interrupt id's. We will therefore increment this loop by 32.
        // Interrupt clear enable register. Writing 1 disables forwarding of the corresponding interrupt id.
        WRITE_REG_UINT32(gic_regs + GICD_ICENABLER + (sizeof(uint32_t) * j), IRQ_SET_ALL);
        // Interrupt clear pending register. Writing 1 sets the corresponding interrupt id from pending to inactive.
        WRITE_REG_UINT32(gic_regs + GICD_ICPENDR + (sizeof(uint32_t) * j), IRQ_SET_ALL);
    }

    // Enable the distributor
    WRITE_REG_UINT32(gic_regs + GICD_CTLR, CTLR_ENABLE_G0_BIT | CTLR_ENABLE_G1S_BIT | CTLR_ENABLE_G1NS_BIT);

    // TODO: Need to actually wait for the distributor to actually start up.
    gicv3_do_wait_for_rwp(gic_regs + GICD_CTLR);

    uint32_t mpidr = 0;
    MRS("MPIDR_EL1", mpidr);
    uint64_t affinity = (uint64_t)MPIDR_AFF3(mpidr) << 32 | MPIDR_AFF2(mpidr) << 16 |
                        MPIDR_AFF1(mpidr) << 8  | MPIDR_AFF0(mpidr);;
    puts("This is the affinity for this CPU: ");
    puthex64(affinity);
    puts("\n");
    // Set the affinity for all global interrupts to this CPU (assume CPU 0 for now).
    for (int i = MIN_SPI_ID; i < nr_lines; i++) {
        // TODO: We need to get the PE id of what we are currently running on.
        WRITE_REG_UINT64(gic_regs + GICD_IROUTER + ((sizeof(uint64_t)) * i),  (affinity << 32) | (affinity << 16) | (affinity << 8) | (affinity & 0xFF));
        puts("Writing affinity!\n");
    }
    puts("Finished?\n");
}

void init_gic_v3(uint64_t gic_regs) {
    puts("In init gic_v3\n");

    init_gic_dist(gic_regs);

    puts("Finished GIC_v3 INIT!\n");
}

void kernel_interrupt_handler() {

}