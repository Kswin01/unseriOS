#pragma once

#define GIC_DEADLINE_MS 2

// NOTE: Only dealing with GICv3 for now.
void init_gic_v3(uint64_t gic_dist_regs, uint64_t gic_rdist_regs);

void kernel_interrupt_handler();