#pragma once

#include <stdint.h>
#include <stdio.h>

static char
hexchar(unsigned int v)
{
    return v < 10 ? '0' + v : ('a' - 10) + v;
}

static void
puthex64(uint64_t val)
{
    char buffer[16 + 3];
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[16 + 3 - 1] = 0;
    for (unsigned i = 16 + 1; i > 1; i--) {
        buffer[i] = hexchar(val & 0xf);
        val >>= 4;
    }
    puts(buffer);
}

#define BIT_32(nr)   (U(1) << (nr))
#define BIT(nr) (1UL << (nr))
#define U(x) (x##U)
#define ULL(X) X##ULL

#define READ_REG(var,addr) \
    (var = (*(volatile uint32_t *)addr))

#define WRITE_REG_UINT32(addr, val) \
    (*(volatile uint32_t *)(addr) = (val))

#define WRITE_REG_UINT64(addr, val) \
    (*(volatile uint64_t *)(addr) = (val))

#define SET_BIT_REG(addr,n) (*(volatile uint32_t *)(addr) |= (1 << (n)))
#define CLR_BIT_REG(addr,n) (*(volatile uint32_t *)(addr) &= (~(1) << (n)))
#define SET_BIT_REG_MASK(addr,mask) (*(volatile uint32_t *)(addr) |= mask)
#define CLR_BIT_REG_MASK(addr,mask) (*(volatile uint32_t *)(addr) &= ~(mask))

#define MRC(cpreg, v)  asm volatile("mrc  " cpreg :  "=r"(v))
#define MRS(reg, v)  asm volatile("mrs %x0," reg : "=r"(v))
#define MSR(reg, v)                                     \
    do {                                                \
        uint64_t _v = v;                                \
        asm volatile("msr " reg ",%x0" ::  "r" (_v));   \
    } while(0)

// Timer conversions

#define MS_IN_S     ULL(1000)
#define US_IN_MS    ULL(1000)
#define HZ_IN_KHZ   ULL(1000)
#define KHZ_IN_MHZ  ULL(1000)
#define HZ_IN_MHZ   ULL(1000000)
