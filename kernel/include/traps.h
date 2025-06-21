#pragma once

#include <stdint.h>

// This vector table is defined in `traps.S`. We are getting the pointer to this table here.
extern char arm_vector_table[1];

void init_vector_table(void);
