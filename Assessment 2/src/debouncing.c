#include "debouncing.h"
#include "spi.h"
#include "enumerators.h"
#include "tcb.h"

// Declaring debouncing, sequence_index, and potentiometer variables
volatile uint8_t pb_state = 0xFF;
uint8_t pb_prev = 0xFF;
uint8_t pb_current = 0xFF;
uint8_t pb_falling = 0x00;
uint8_t pb_rising;
uint8_t pb_changed;
uint8_t sequence_index;
uint8_t potentiometer;
extern mode current_mode;

// Enable pull-up registers, initialise with sequence_index = 0 and LED displaying 0x00
void buttons_init(void) {
    spi_write(0b00001000);
    spi_write(0b10001000);
    sequence_index = 0;
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}

// Initialise pushbutton debouncing variables
void pb_init(void) {
    pb_prev = pb_current;
    pb_current = pb_state;
    pb_changed = pb_prev ^ pb_current;
    pb_falling = pb_changed & ~pb_state;
    pb_rising = pb_changed & pb_state;
}

// This function implemente logic for sequence select mode.
void get_index(void) {
    PORTB.OUTCLR = PIN0_bm;
    potentiometer = ADC0.RESULT;
    // If falling edge and S3 pressed,increment and display only if sequence_index < 255
    if (pb_falling & PIN6_bm) {
        if (sequence_index < 255) {
            sequence_index = sequence_index + 1;
            display_hex(sequence_index);
        }
    }
    // If falling edge on S2, decrement and display only if sequnece_index > 0
    if (pb_falling & PIN5_bm) {
        if (sequence_index > 0) {
            sequence_index = sequence_index - 1;
            display_hex(sequence_index);
        }
    }
    // When rising edge on S1, sequence_index = potentiometer
    if (pb_rising & PIN4_bm) {
        sequence_index = potentiometer;
        display_hex(potentiometer);
    }     
    // When S1 is depressed, display the current potentiometer value  
    if (~pb_state & PIN4_bm) display_hex(potentiometer);
    // If falling edge on S4, change to SEQUENCING mode.
    if (pb_falling & PIN7_bm) {
        current_mode= SEQUENCING;
    }
}