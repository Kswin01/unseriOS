/* General register definition for the generic qemu aarch64. */

#pragma once

#include "util.h"

// Timer
#define TIMER_INTERRUPT 

// Serial
#define PL011_UART_REGS 0x9000000
#define PL011_BAUD 115200

// Interrupts
#define QEMU_GICV3_DIST_REGS 0x8000000
#define QEMU_GICV3_RDIST_REGS 0x80a0000

// Physical Memory
#define QEMU_PHYS_MEM_START 0x40000000
#define QEMU_MEM_SIZE 0x80000000

#define TIMER_CLOCK_HZ ULL(62500000)
#define TICKS_PER_MS (TIMER_CLOCK_HZ / HZ_IN_KHZ)