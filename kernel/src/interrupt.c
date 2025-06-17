/* Setup and use of the ARM GICv3 */

#include <stdio.h>
#include <stdint.h>

#include <interrupt.h>
#include <util.h>
#include <gic/gic_common.h>
#include <gic/gicv3.h>

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

    // Set the affinity for all global interrupts to this CPU (assume CPU 0 for now).
    for (int i = MIN_SPI_ID; i < nr_lines; i++) {
        // TODO: We need to get the PE id of what we are currently running on.
        // WRITE_REG_UINT64(gic_regs + GICD_IROUTER + (8 * i),  (0ULL << 56) | (0ULL << 48) | (0ULL << 32) | 0ULL);
    }
}

void init_gic_v3(uint64_t gic_regs) {
    puts("In init gic_v3\n");

    init_gic_dist(gic_regs);

    puts("Finished GIC_v3 INIT!\n");
}

void kernel_interrupt_handler() {

}