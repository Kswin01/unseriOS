#pragma once

#include <stdint.h>
#include "util.h"

// Start of the kernel section
#define KERNEL_ELF_PHYS_BASE 0x40100000
// This is the end physical address that is designated in the linker script
extern uint64_t kernel_end[1];

// This is where our kernel is going to mapped into
#define KERNEL_VADDR_BASE  ULL(0xffffff8000000000)
// This is where the device section is going to go.
#define KERNEL_VADDR_TOP  ULL(0xffffffffc0000000)

// This is the starting vaddr of the kernel elf when accounting for the
// vaddr base offset.
#define KERNEL_ELF_VADDR_BASE (KERNEL_VADDR_BASE + KERNEL_ELF_PHYS_BASE)

// The last address that we supply to the users
#define USER_TOP ULL(0x00007fffffffffff)

#define AARCH64_SMALL_PAGE 0x1000
#define AARCH64_LARGE_PAGE 0x200000

// Apply the kernel vaddr base offset, and then calculate the appropriate index.
#define GET_KERNEL_PD_INDEX(x) (((x + KERNEL_VADDR_BASE) & ((uintptr_t)0x1ff << 30)) >> 30)

#define PADDR_TO_KERNEL_VADDR(x) (x + KERNEL_VADDR_BASE)

// Paging structures
typedef struct pt {
    uint64_t pte[512];
} pt_t;

typedef struct pd {
    pt_t pts[512];
} pd_t;

typedef struct pud {
    pd_t pds[512];
} pud_t;

typedef struct pgd {
    pud_t puds[2];
} kernel_pgd_t;

void kernel_mem_init();