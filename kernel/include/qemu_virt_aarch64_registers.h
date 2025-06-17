/* General register definition for the generic qemu aarch64. */

#pragma once

// Serial
#define PL011_UART_REGS 0x9000000
#define PL011_BAUD 115200

// Interrupts
#define QEMU_GICV3_REGS 0x8000000