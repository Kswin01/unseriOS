#include <stdio.h>
#include <stdint.h>
#include <qemu_virt_aarch64_registers.h>
#include <uart.h>

volatile uint8_t *uart = (uint8_t *) PL011_UART_REGS;

// @kwinter - TODO: implement an actual serial driver for the kernel
int _putchar(int ic) {
    // char c = (char) ic;
    // *uart = c;
    // return ic;
    write_uart(ic);
}
