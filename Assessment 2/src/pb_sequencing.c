#include "pb_sequencing.h"
#include "enumerators.h"
#include "tca.h"
#include "debouncing.h"
#include "tca.h"
#include "sequencing.h"

extern volatile uint8_t pb_state;
extern uint8_t pb_prev;
extern uint8_t pb_current;
extern uint8_t pb_falling;
extern uint8_t pb_rising;
extern uint8_t pb_changed;
extern uint8_t disable_tcb1;
mode sequencing_mode = START;
extern uint16_t duration_counter;
extern uint8_t descrambled[3];
extern volatile uint8_t sequence_complete;
extern uint8_t offset;

// Push button handling while in SEQUENCING mode
void pb_handling(void) {
    // Pushbutton sampling
    pb_init();

    switch(sequencing_mode) {
        case START:
            if (pb_falling & PIN7_bm) { // If S4 is pressed, pause sequence
                disable_tcb1 = 1;
                sequencing_mode = PAUSED;
            }
            break;
        case PAUSED:
            if (pb_falling & PIN7_bm) { // If S4 is pressed again, fetch next byte, reset duration_counter and resume sequence
                fetch_bytes();
                duration_counter = 0;
                disable_tcb1 = 0;
                sequencing_mode = START;
            }
            if (pb_falling & PIN6_bm) { // If S3 is pressed, increments step while paused. If duration = 0, terminate
                fetch_bytes();
                tca_step(); 
                duration_counter = 0;
                if (descrambled[0] == 0) {
                    sequencing_mode = START;
                    disable_tcb1 = 1;
                    sequence_complete = 1;
                }
                else sequencing_mode = PAUSED;
            }
            if (pb_falling & PIN5_bm) { // If S2 is pressed, terminate and set sequencing_mode = START
                descrambled[2] = 0; // Set note and octave to 0
                tca_step(); // Execute step
                duration_counter = 0; // Reset duration_counter
                sequencing_mode = START;
                disable_tcb1 = 1; // Disable tcb1
                sequence_complete = 1; // Exits while loop in sequencing.c
            }
            break;
        default:
            sequencing_mode = START;
    }
}