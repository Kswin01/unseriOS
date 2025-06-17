#pragma once

#define BIT(nr) (1UL << (nr))
#define U(x) (x##U)
#define BIT_32(nr)   (U(1) << (nr))

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