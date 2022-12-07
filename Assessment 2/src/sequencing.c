#include "sequencing.h"
#include "sequence.h"
#include "enumerators.h"
#include "descrambling.h"
#include "pb_sequencing.h"
#include "descrambling.h"
#include "decoding.h"
#include "tcb.h"
#include "serial_protocol.h"
#include <avr/pgmspace.h>

extern uint8_t sequence_index;
uint16_t offset = 0;
extern mode current_mode;
extern char const SEQUENCE[] PROGMEM;
const char *ptr = SEQUENCE;
uint32_t decoded;
uint8_t descrambled[3];
uint8_t duration;
uint8_t decoded_byte[3];
volatile uint8_t sequence_complete;
extern uint32_t STATE_LFSR;
extern uint8_t disable_tcb1;

// This function calculates the offset and calls the next() function to advance the LFSR state.
// The decode and descramble functions are called to descramble the sequence offset.
void sequence_init(void) {
    PORTB.OUTSET = PIN0_bm;
    STATE_LFSR = 0x11240164;
    offset = sequence_index << 5;
    for (uint16_t i = 0; i < ((offset * 3) >> 2); i++) next();
    fetch_bytes();
    sequence_complete = 0;
    disable_tcb1 = 0;

    // While sequence is not complete, call pushbutton handling logic for Part B3
    while (sequence_complete == 0) {
        parser_service();
        pb_handling();
    }
    terminate_sequence();
}

// Fetches the bytes by decoding and descrambling the sequence
void fetch_bytes(void) {
    // If duration == 0, while loop ends
    if ((duration == 0) && (sequence_complete == 0)) {
        disable_tcb1 = 1;
        sequence_complete = 1;
    }

    // Reads encoded string with offset variable
    uint32_t encoded_string = pgm_read_dword_near(ptr + offset);
    decoded = decode(encoded_string); 
    decoded_byte[0] = decoded >> 16 & 0xFF;
    decoded_byte[1] = decoded >> 8 & 0xFF;
    decoded_byte[2] = decoded & 0xFF;

    // Descrambles decoded bytes and increments offset by 4 chars
    descramble(decoded_byte);
    duration = descrambled[0];
    offset = offset + 4;
}

// Terminates sequence by setting duration to 0, buzzer off, and led 100%.
void terminate_sequence(void) {
    duration = 0;
    disable_tcb1 = 1;
    TCA0.SINGLE.PERBUF = 3333;
    TCA0.SINGLE.CMP1BUF = 3334;
    TCA0.SINGLE.CMP0BUF = 0;
    PORTB.OUTCLR = PIN0_bm;
    display_hex(sequence_index);
    sequence_complete = 1;
    if (current_mode == SEQUENCING) {
        current_mode = SEQUENCE_SELECT;
    }
    else if (current_mode == TEST) {
        current_mode = TEST;
    }
}
