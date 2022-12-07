#include "descrambling.h"

// Initialise mask, student number, and descrambling variables.
volatile uint32_t STATE_LFSR = 0x11240164;
const uint32_t MASK = 0xB4BCD35C;
uint8_t bit;
extern uint8_t descrambled[3];

// Shifts state_lfsr
void next(void) {
    bit = STATE_LFSR & 1;
    STATE_LFSR = STATE_LFSR >> 1;
    if (bit == 1) STATE_LFSR = STATE_LFSR ^ MASK;
}

// Descrambles 3 bytes
void descramble(uint8_t *decoded) {
    for (uint8_t i = 0; i < 3; i++)  {
        descrambled[i] = (STATE_LFSR & 0xFF) ^ (decoded[i]);
        next();
    }
}

