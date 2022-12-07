#include "tcb.h"
#include "enumerators.h"
#include "spi.h"

// Initialises variables for debouncing and display_hex
// Stores hex mapping 0-9 A-F for LED display in uint8_t array
extern uint8_t pb_state;
uint8_t display_select = 0;
uint8_t valueL;
uint8_t valueR;
uint8_t displayR[] = { 
    0b00001000, 
    0b01101011, 
    0b01000100, 
    0b01000001, 
    0b00100011, 
    0b00010001,
    0b00010000,
    0b01001011, 
    0b00000000, 
    0b00000001, 
    0b00000010, 
    0b00110000, 
    0b00011100, 
    0b01100000,
    0b00010100,
    0b00010110,
    0b01111111,
    0b01110111
};
uint8_t displayL[] = {
    0b10001000, 
    0b11101011, 
    0b11000100, 
    0b11000001, 
    0b10100011, 
    0b10010001,
    0b10010000,
    0b11001011, 
    0b10000000, 
    0b10000001, 
    0b10000010, 
    0b10110000, 
    0b10011100, 
    0b11100000,
    0b10010100,
    0b10010110,
    0b11111111,
    0b11110111
};
extern mode current_mode;

// Isolates valueL and valueR
void display_hex(uint8_t value) {
    valueL = value >> 4;
    valueR = value & 0x0F;
}

// Enable TCB0 for debouncing and display_hex function
void tcb0_init(void) {
    cli();
    TCB0.CTRLB = TCB_CNTMODE_INT_gc; // Enable periodic interrupt mode
    TCB0.CCMP = 3333; // Set period for 1 ms (3333 clocks @ 3.3MHz)
    TCB0.INTCTRL = TCB_CAPT_bm; // Enable CAPT interrupt source               
    TCB0.CTRLA = TCB_CLKSEL_DIV1_gc | TCB_ENABLE_bm; //CLK_PER select, no prescaler   
    sei(); 
}

// Interrupt for TCB0, contains logic for 50% duty cycle
// two - digit, time-multiplixed display using spi_write
ISR(TCB0_INT_vect) {
    if ((current_mode == SEQUENCE_SELECT) | (current_mode == TEST)) { // disable in SEQUENCING mode
        if (display_select == 0) { // Left display
            spi_write(displayL[valueL]); // spi_writes valueL by indexing displayL array
            PORTA.OUTCLR = PIN1_bm; // Reset DISP Latch
            PORTA.OUTSET = PIN1_bm; 
            display_select = 1; // Alternates to right display for next interrupt cycle only if mode is sequence_select 
        }
        else if (display_select == 1) {
            spi_write(displayR[valueR]);
            PORTA.OUTCLR = PIN1_bm;
            PORTA.OUTSET = PIN1_bm; 
            display_select = 0;   
        }
    }

    static uint8_t count0 = 0; 
    static uint8_t count1 = 0; //static = 1 copy of the variable

    uint8_t pb_sample = PORTA.IN; //sample the input register
    uint8_t pb_changed = pb_sample ^ pb_state; //detect changes

    //vertical counter logic, increment if changed, reset otherwise
    count1 = (count1 ^ count0) & pb_changed;
    count0 = ~count0 & pb_changed; 
    
    // Update PB state immediately on falling edge or if PB high for three samples
    pb_state ^= (count0 & count1) | (pb_changed & pb_state);
    TCB0.INTFLAGS = TCB_CAPT_bm;   
}