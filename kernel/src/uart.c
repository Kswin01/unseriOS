/*  Setup and use of the PL011 UART device for the kernel. */

#include <stdio.h>
#include <stdint.h>

#include <uart.h>
#include <qemu_virt_aarch64_registers.h>

volatile pl011_uart_regs_t *regs;
/*
 * BaudDivInt + BaudDivFrac/64 = (RefFreq/ (16 x BaudRate))
 */
static void set_baud(long bps)
{
    float baud_div = PL011_UART_REF_CLOCK / (16 * bps);
    uint32_t baud_div_int = (uint32_t)baud_div;
    uint32_t baud_div_frac = (uint32_t)((baud_div * 64) + 0.5);

    /* Minimum divide ratio possible is 1 */
    // assert(baud_div_int >= 1);

    /* Maximum divide ratio is 0xFFFF */
    // assert(baud_div_int < 0xFFFF || (baud_div_int == 0xFFFF && baud_div_frac == 0));

    regs->ibrd = baud_div_int;
    regs->fbrd = baud_div_frac;
}

void write_uart(char ch) {
    // Busy wait until we can put a character out.
    uint32_t timeout = 0;
    while(regs->fr & PL011_FR_TXFF) {
        // Very arbitrary timeout for now.
        if (timeout >= 0x1000) {
            break;
        }
        asm("nop");
    }

    regs->dr = ch;
}

void init_uart(uint64_t addr) {
    // puts("Initialising the UART!\n");
    regs = (pl011_uart_regs_t *) addr;

    /* Wait for UART to finish transmitting. */
    while (regs->fr & PL011_FR_UART_BUSY);

    /* Disable the UART - UART must be disabled before control registers are reprogrammed. */
    regs->tcr &= ~(PL011_CR_RX_EN | PL011_CR_TX_EN | PL011_CR_UART_EN);

    /* Configure stop bit length to 1 */
    regs->lcr_h &= ~(PL011_LCR_2_STP_BITS);

    /* Set data length to 8 */
    regs->lcr_h |= (0b11 < PL011_LCR_WLEN_SHFT);

    /* Configure the reference clock and baud rate. Difficult to use automatic detection here as it requires the next incoming character to be 'a' or 'A'. */
    // set_baud(PL011_BAUD);

    /* Enable FIFOs */
    regs->lcr_h |= PL011_LCR_FIFO_EN;

    /* Disable parity checking */
    regs->lcr_h |= PL011_LCR_PARTY_EN;

    /* Enable receive interrupts when FIFO level exceeds 1/8 or after 32 ticks */
    // regs->ifls &= ~(PL011_IFLS_RX_MASK << PL011_IFLS_RX_SHFT);
    // regs->imsc |= (PL011_IMSC_RX_TIMEOUT | PL011_IMSC_RX_INT);

    /* Enable transmit interrupts if the FIFO drops below 1/8 - used when the write fifo becomes full */
    // regs->ifls &= ~(PL011_IFLS_TX_MASK << PL011_IFLS_TX_SHFT);
    // regs->imsc |= PL011_IMSC_TX_INT;

    /* Enable the UART */
    regs->tcr |= PL011_CR_UART_EN;

    /* Enable transmit */
    regs->tcr |= PL011_CR_TX_EN;
    puts("Initialised the PL011 UART!\n");
}
