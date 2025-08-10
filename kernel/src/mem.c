#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <util.h>
#include <mem.h>

#include "qemu_virt_aarch64_registers.h"

// gyatt allocator = physical memory allocator

/* We will maintain a one-to-one mapping between physical memory and
 * kernel virtual memory. The first gigabyte of physical memory
 * will be mapped in as device memory, and non executable. This
 * is where our peripherals exist. We will then map up until the
 * end of the kernel, to the nearest 2MiB page.

 * The kernel page tables are statically sized, we are not
 * doing any lazy loading as we are not going to implementent
 * memory allocation for now. We will also need to include mappings
 * for the first 1GB - This, we are going to assume is device memory
 * (UART and such).

 * We will map one PGD, and one PUD. This is enough for 512GiB worth
 * of virtual memory. We will only actually fill the first 1GiB of
 * pyshical memory (and mark this as device memory), and the memory
 * required by the kernel, which will be determined by kernel_end.
 *
 * We will map all this memory as large 2MiB pages. This will reduce the
 * memory overhead of the translation tables by removing the need for
 * the final level page tables.
 */

// All paging structures need to be 4KiB page aligned
uint64_t kernel_pgd[512] ALIGN_BSS(AARCH64_SMALL_PAGE);
uint64_t kernel_pud[512] ALIGN_BSS(AARCH64_SMALL_PAGE);
// This is 512 PDs, each with 512 large pages.
uint64_t kernel_pds[512][512] ALIGN_BSS(AARCH64_SMALL_PAGE);


uint64_t ident_pgd[512] ALIGN_BSS(AARCH64_SMALL_PAGE);
uint64_t ident_pud[512] ALIGN_BSS(AARCH64_SMALL_PAGE);
uint64_t ident_pds[512][512] ALIGN_BSS(AARCH64_SMALL_PAGE);

void el1_mmu_enable();


void clean_and_invalidateL1() {
    /* Data synchronisation barrier. Ensures
     * the completion of memory accesses.
     *
     * sy in this case means that read and writes
     * in the whole system must complete before
     * continuing, not just within a specfic
     * shareablity domain.
     */
    asm volatile("dsb sy");

    // Clean and invalidate the data cache by set way.
    // We will need to go through the cache and call
    // asm("DC CISW") on all cache sets. This will involve
    // reading the cache size (alll the css___el1 registers.)

    asm volatile("dsb sy");

    // Clean and invalidate the instruction cache to Point
    // of Unification. PoU can be in the L1 data cache or
    // outside of the processor, and then dependent on the
    // external memory system.
    // iallu - clean and invalidate ALL to PoU
    // ialluis - clean and invalidate ALL to PoU
    asm volatile("ic iallu");

    asm volatile("dsb sy");
}

void tlb_invalidate() {
    asm("nop");
    return;
}

// Create a large page entry for the kernel page directories.
uint64_t large_page_entry(uint64_t paddr, bool device_mem) {
    uint64_t page_entry = 0;
    page_entry = (paddr & ULL(0xfffffffff000));
    // Leave BIT(1) = 0, designates a block entry.
    // Mark as valid
    page_entry |= BIT(0);

    if (device_mem) {
        // Setting to exec never for device memory.
        page_entry |= (BIT_64(54) | BIT_64(53));
        // Marking as strongly ordered
        page_entry |= (0 << 2);
    } else {
        // Mark as normal memory.
        page_entry |= (4 << 2);
    }

    page_entry |= (1 << 10);
    return page_entry;
}

// Create a kernel translation table entry for a supplied
// physical address.
uint64_t kernel_translation_table_entry(uint64_t paddr) {
    uint64_t table_entry = 0;
    table_entry = (paddr & ULL(0xfffffffff000));
    // Mark this as a table descriptor
    table_entry |= BIT(1);
    // Mark this as a valid entry
    table_entry |= BIT(0);
    return table_entry;
}

void kernel_setup_traslation_tables() {
    /* LEARNING TIME:
     * Translation tabel format for aarch64 48 bit addresses.
     *
     * Bits 63:59: Table attributes
     * UXN = unpriveleged execute never (User space CANNOT use this translation)
     * PXN = privelege execute never (Kernel CANNOT execute from this memory)
     * XN = execute never
     * AP = Access Privelege Bits, if these regions are read/write and what
     * execpetion level can r/w.
     *
     * All of these attributes apply to all the pages within these table
     * structures, so if I mark the PGD as execute never and read, all pages
     * resident within the PGD will share the same permissions. Permissions
     * can only get MORE restrictive the furhter down the paging tree you
     * get, they can NEVER get more permissive.
     *
     * NOTE: This is only if heirarchical control is enabled
     *
     * Bits 47:12: Translation address - the physical address of the next
     * set of paging structures.
     *
     * Bit 1: Table descriptor.
     * Set this bit to 1 as we are a table entry.
     *
     * Bit 0: Valid
     * Set this bit to 1 to mark it a valid entry.
     */

    // We are just going to go through the statically created
    // translation tables and point them at each other.
    // Point first PGD entry to PUD
    kernel_pgd[511] = kernel_translation_table_entry(&kernel_pud);

    //Point first entry of PUD to PDs.
    for (int i = 0; i < 512; i++) {
        kernel_pud[i] = kernel_translation_table_entry(&kernel_pds[i][0]);
    }

    // @kwinter: Do we actually want to map into 0?
    uint64_t paddr = 0;
    // We will map the first GiB of physical memory to the kernel PD's.
    // We will use large pages for these mappings
    for (int i = 0; i < 512; i++) {
        kernel_pds[GET_KERNEL_PD_INDEX(paddr)][i] = large_page_entry(paddr, true);
        paddr += AARCH64_LARGE_PAGE;
    }

    // Now we can map in the 1GiB that covers our kernel.
    // We don't need to really setup translations for
    // any other regions of memory at this point.

    // We placed our kernel in the top gigabyte of our memory
    paddr = QEMU_PHYS_MEM_START + (QEMU_MEM_SIZE / 2);
    for (int i = 0; i < 512; i++) {
        kernel_pds[GET_KERNEL_PD_INDEX(paddr)][i] = large_page_entry(paddr, false);
        paddr += AARCH64_LARGE_PAGE;
    }
}

void kernel_mmu_start() {
    // Invalidate the caches
    clean_and_invalidateL1();
    // Write to the TTBR register

    asm volatile("dsb sy");
    MSR("ttbr1_el1", (void *)&kernel_pgd);
    asm volatile("isb");
    // Write to the TCR

    // Invalidate the TLB
    tlb_invalidate();

    // Set stack pointer -> (Do we need to clear this as well?)

    // We also need to make sure that our PC is updated appropriately. (Could just add the correct offset to it)

    return;

}

void kernel_mem_init() {
    kernel_setup_traslation_tables();
    puts("Setup kernel translation tables!\n");
    el1_mmu_enable();
    puts("Enabled ident MMU!\n");
}

void setup_ident_map() {
    // We are just going to go through the statically created
    // translation tables and point them at each other.
    // Point first PGD entry to PUD
    ident_pgd[0] = kernel_translation_table_entry((uint64_t)&ident_pud);
    //Point first entry of PUD to PDs.

    for (int i = 0; i < 512; i++) {
        ident_pud[i] = kernel_translation_table_entry((uint64_t)&ident_pds[i][0]);
    }

    uint64_t paddr = 0;
    // We will map the first GiB of physical memory to the kernel PD's.
    // We will use large pages for these mappings
    for (int i = 0; i < 512; i++) {
        ident_pds[GET_IDENT_PD_INDEX(paddr)][i] = large_page_entry(paddr, true);
        paddr += AARCH64_LARGE_PAGE;
    }

    // Now we can map in the 1GiB that covers our kernel.
    // We don't need to really setup translations for
    // any other regions of memory at this point.
    paddr = QEMU_PHYS_MEM_START;
    for (int i = 0; i < 512; i++) {
        ident_pds[GET_IDENT_PD_INDEX(paddr)][i] = large_page_entry(paddr, false);
        paddr += AARCH64_LARGE_PAGE;
    }
}