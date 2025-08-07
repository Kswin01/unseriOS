/* This is a single stage bootloader, responsible for copying out the kernel and a rootserver,
and jumping to the kernel. */
#include <stdint.h>
#include <stddef.h>

#include <uart.h>
#include <qemu_virt_aarch64_registers.h>

typedef struct loader_data {
    // Kernel start is the address of the ELF segment.
    uint64_t kernel_start;
    uint64_t kernel_size;
    uint64_t kernel_vaddr;

    uint64_t rootserver_start;
    uint64_t rootserver_size;
    uint64_t rootserver_vaddr;
} loader_data_t;

__attribute__((__section__(".loader_data"))) loader_data_t loader_config;

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
    printf("Copying kernel from %p to %p\n", loader_config.kernel_start, loader_config.kernel_vaddr);
    memcpy(loader_config.kernel_vaddr, loader_config.kernel_start, loader_config.kernel_size);
    printf("Copying rootserver from: %p to %p\n", loader_config.rootserver_start, loader_config.rootserver_vaddr);
    memcpy(loader_config.rootserver_vaddr, loader_config.rootserver_start, loader_config.rootserver_size);
}

int loader_main() {
    // First, initalise the UART.
    init_uart(PL011_UART_REGS);

    // Copy data
    copy_data();

    // Create identity mapping for the kernel

    // Enable MMU

    // Start kernel
    // Where is the kernel entrypoint? This should be info that we pass in.
}