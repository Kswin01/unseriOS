#include <stdio.h>
#include <stdint.h>

volatile uint8_t *uart = (uint8_t *) 0x09000000;

// @kwinter - TODO: implement an actual serial driver for the kernel
int putchar(int ic) {
    char c = (char) ic;
    *uart = c;
    return ic;
}

void _putchar(char c) {
    *uart = c;
}