/* For the ARMv8 generic timer */

#pragma once

#include <stdint.h>
#include <util.h>

/* Register names */
#define TIMER_FREQ      "CNTFRQ_EL0"
#define TIMER_CTL       "CNTP_CTL_EL0"
#define TIMER_TVAL      "CNTP_TVAL_EL0"
#define TIMER_CTAL      "CNTP_CVAL_EL0"

#define CNT_INT_EN      BIT(0)
#define CNT_INT_MASK    BIT(1)
#define CNT_INT_STATUS  BIT(2)

int timer_init();
/* Provide time in milliseconds */
int set_timeout(uint32_t time);
void handle_timer_interrupt();