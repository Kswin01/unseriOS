/* This is a single stage bootloader, responsible for copying out the kernel and a rootserver,
and jumping to the kernel. */
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include <mem.h>
#include <vmem.h>
#include <uart.h>
#include <qemu_virt_aarch64_registers.h>
#include <elf.h>

// This assumes that the kernel will fit into the first 100 pages.
__attribute__((section(".kernel"))) char kernel[0x100000];

typedef void (*kernel_entry)(void);

static void *memcpy(void *dst, const void *src, size_t sz)
{
    char *dst_ = dst;
    const char *src_ = src;
    while (sz-- > 0) {
        *dst_++ = *src_++;
    }

    return dst;
}

void *memmove(void *restrict dest, const void *restrict src, size_t n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    /* no copying to do */
    if (d == s) {
        return dest;
    }
    /* for non-overlapping regions, just use memcpy */
    else if (s + n <= d || d + n <= s) {
        return memcpy(dest, src, n);
    }
    /* if copying from the start of s to the start of d, just use memcpy */
    else if (s > d) {
        return memcpy(dest, src, n);
    }

    /* copy from end of 's' to end of 'd' */
    size_t i;
    for (i = 1; i <= n; i++) {
        d[n - i] = s[n - i];
    }

    return dest;
}

uintptr_t copy_kernel() {
    // @kwinter: Maybe add kernel binary signing here?
    // Check the ELF magic 
    // size_t size = (size_t)&__stop_kernel - (size_t)&kernel;
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *) kernel;
    for (int i = 0; i < ELF_MAGIC_LEN; i++) {
        if (ehdr->e_ident[i] != elf_magic[i]) {
            puts("ELF Magic is bad! Going to spin now!\n");
            while(1) {
                asm volatile("nop");
            }
        }
    }

    puts("LOADER: ELF Magic check complete!\n");

    for (int i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr *phdr = (Elf64_Phdr *)(kernel + ehdr->e_phoff + i * ehdr->e_phentsize);
        if (!(phdr->p_type & 1)) {
            puts("LOADER: Nothing to load! Going to spin now!\n");
            while(1) {
                asm volatile("nop");
            }
        } else {
            // TODO: Fix this up in the case that we have multiple loadable segments
            puts("LOADER: Copying ");
            puthex32(phdr->p_filesz);
            puts(" to vaddr: ");
            puthex64(phdr->p_vaddr);
            puts("\n");
            memcpy(phdr->p_vaddr, kernel + phdr->p_offset, phdr->p_filesz);
            uint32_t num_zeroes = phdr->p_memsz - phdr->p_filesz;
            puts("LOADER: Finished copying data. Now copying ");
            puthex32(num_zeroes);
            puts(" zeroes into memory!\n");

            //@kwinter: Figure out why the memset implementation is not working!
            for (int j = 0; j < num_zeroes; j++) {
                char *addr_to_zero = (char *)(phdr->p_vaddr + phdr->p_filesz + j);
                *addr_to_zero = 0;
            }
            puts("LOADER: Finished loading kernel ELF!\n");
        }
    }

    return ehdr->e_entry;
}

int lmain() {
    // First, initalise the UART.
    init_uart(PL011_UART_REGS);
    puts("We are in the loader main!\n");

    puts("Finished copying the kernel!\n");
    // // Before we copy the ELF out, get the entry point
    // puts("this is the kernel entry address: ");
    // puthex64(hdr->e_entry);
    // puts("\n");
    // uint64_t hdr_entry = hdr->e_entry;
    setup_ident_map();
    puts("LOADER: Finished setting up identity map for loader!\n");

    // Enable MMU
    kernel_mem_init();
    puts("LOADER: Setup kernel translation tables, and enabled MMU!\n");

    // Copy kernel
    uintptr_t k_entry = copy_kernel();
    asm volatile("dsb ish");
    asm volatile("isb");

    // Start kernel
    puts("LOADER: Loader finished, jumping to kernel at address  ");
    puthex64(k_entry);
    puts(", good luck and have fun...\n");
    kernel_entry kmain = (kernel_entry) k_entry;
    kmain();
    puts("We have returned?\n");
}