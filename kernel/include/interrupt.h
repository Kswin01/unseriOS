#pragma once

// NOTE: Only dealing with GICv3 for now.
void init_gic_v3();

void kernel_interrupt_handler();