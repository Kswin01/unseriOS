#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <util.h>
#include <ktimer.h>

uint64_t timer_freq;

int timer_init() {
    puts("Initialising timer...\n");

    /* Setup timer interrupt */

    /* Initialising the EL1 physical timer - CNTP */
    uint32_t ctl = 0;
    ctl |= CNT_INT_EN;
    /* Mask the interrupt, we will unmask when we set a
    timeout */
    ctl |= CNT_INT_MASK;
    ctl |= (1 << 3);
    MSR(TIMER_CTL, ctl);

    /* Get the current timer frequency so we can
    convert from time to cycles when we set timeouts */
    MRS(TIMER_FREQ, timer_freq);

    puts("Timer initialised!\n");
}

int set_timeout(uint32_t time_ms) {
    /* Calculate the cycles in time_ms for our given freq */
    uint32_t val = (timer_freq * time_ms + 999) / 1000;

    /* Set the TVAL register for our time */
    MSR(TIMER_TVAL, val);

    /* Unmask timer interrupt */
    uint32_t ctl;
    ctl |= CNT_INT_EN;
    ctl |= (1 << 3);

    MSR(TIMER_CTL, ctl);
}

void handle_timer_interrupt() {
    uint64_t ticks;
    MRS("CNTPCT_EL0", ticks);
    printf("TIME: %lld\n", ticks);
    set_timeout(MS_IN_S);
}