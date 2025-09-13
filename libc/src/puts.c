#include <stdio.h>
#include <uart.h>

int puts(const char* string) {
    while(*string != '\0') {
        write_uart(*string);
        string++;
    }
    return 1;
}
