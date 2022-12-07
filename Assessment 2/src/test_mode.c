#include "test_mode.h"
#include "tcb.h"
#include <stdio.h>
#include "decoding.h"

extern uint32_t STATE_LFSR;
extern uint8_t valueL;
extern uint8_t valueR;
extern uint32_t payload[33];

void test_mode(void) {
    // Hardcode valueL and valueR from tcb0 to 17th index of displayL, which displays '-'
    valueL = 17;
    valueR = 17;
}

// Checksum test
uint8_t check_sum(void) {
    uint8_t checksum = base64(payload[33]); // Converts checksum to base54
    uint16_t sum = 0;

    for (uint8_t i = 0; i < 32; i ++) { // Excludes checksum byte
        sum = sum + payload[i]; // Sum of all bytes
    }
    if (checksum == sum) return 1;
    else return 0;
}

void execute_test(void) {
    STATE_LFSR = 0x11240164;
}