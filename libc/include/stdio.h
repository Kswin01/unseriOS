#pragma once

#include <sys/cdefs.h>
#include <printf.h>

#define EOF (-1)

// int printf(const char* __restrict, ...);
int printer(const char* string, ...);
int _putchar(int);
int puts(const char*);
