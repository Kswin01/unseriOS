/* This is a single stage bootloader, responsible for copying out the kernel and a rootserver,
and jumping to the kernel. */
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include <mem.h>
#include <uart.h>
#include <qemu_virt_aarch64_registers.h>

// Fake out the linker.
__attribute__((section(".kernel"))) DUMMYLINKER = 1;
extern const unsigned char __start_kernel[];
extern const unsigned char __stop_kernel[];

// extern const unsigned char __start_initial_task[];
// extern const unsigned char __ened_initial_task[];

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

void copy_data() {
    size_t size = (size_t)&__stop_kernel - (size_t)&__start_kernel;
    memmove(QEMU_PHYS_MEM_START + (QEMU_MEM_SIZE / 2), &__start_kernel, size);
    asm("dsb sy");
    puts("Finished moving initial task from: ");
    puthex64(&__start_kernel);
    puts(" to ");
    puthex64(QEMU_PHYS_MEM_START + (QEMU_MEM_SIZE / 2));
    puts("--- kernel was of size: ");
    puthex64(size);
    puts("\n");
    // memcpy(loader_config.kernel_vaddr, &__start_kernel, &__stop_kernel - &__start_kernel);
    // printf("Copying rootserver from: %p to %p\n", loader_config.rootserver_start, loader_config.rootserver_vaddr);
    // memcpy(loader_config.rootserver_vaddr, loader_config.rootserver_start, loader_config.rootserver_size);
}

int lmain() {
    // First, initalise the UART.
    init_uart(PL011_UART_REGS);
    puts("We are in the loader main!\n");
    // Copy data
    copy_data();

    setup_ident_map();
    puts("Finished setting up identity map for loader!\n");

    // Enable MMU
    kernel_mem_init();
    puts("Setup kernel translation tables, and enabled MMU!\n");

    // Start kernel
    // Where is the kernel entrypoint? This should be info that we pass in.
}