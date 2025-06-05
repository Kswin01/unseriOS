#include <stdio.h>

int puts(const char* string) {
    while(*string != '\0') {
        putchar(*string);
        string++;
    }
    return 1;
}
