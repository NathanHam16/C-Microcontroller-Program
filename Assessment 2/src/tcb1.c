#include "tcb.h"
#include "tca.h"
#include "tca.h"
#include "sequencing.h"

// Finds definition of variables outside current scope
uint16_t duration_counter = 0;
uint16_t sync_counter = 0;
volatile uint8_t sync_enable;
extern uint8_t duration;
uint8_t disable_tcb1 = 1;
extern uint8_t display_select;


// Enables TCB1 same as TCB0 for the duration of each step. 
void tcb1_init(void) {
    cli();
    TCB1.CTRLB = TCB_CNTMODE_INT_gc; // Enable periodic interrupt mode
    TCB1.CCMP = 3333; // Set period for 1 ms (3333 clocks @ 3.3MHz)
    TCB1.INTCTRL = TCB_CAPT_bm; // Enable CAPT interrupt source               
    TCB1.CTRLA = TCB_CLKSEL_DIV1_gc | TCB_ENABLE_bm; //CLK_PER select, no prescaler   
    sei(); 
}

// TCB1 interrupt for handling duration of a step from descrambling
ISR(TCB1_INT_vect) {  

    // If enabled, increment counter and update buzzer and LED values
    if (disable_tcb1 == 0) {
        tca_step(); 
        duration_counter++;   
    }

    // If tcb1 is enabled and duration_counter(ms) * 10 > 131* duration 
    if ((duration_counter*10 >= 131*duration) && (disable_tcb1 == 0)) {
        fetch_bytes(); //Fetch new bytes
        duration_counter = 0; //Reset counter
    }
    
    // CMD_SYNC counter increments and flashes at 1Hz 50% duty cycle upon receipt of message
    if (sync_enable == 1) {
        sync_counter++;
        
        if (sync_counter >= 500) { // 0.5 seconds
            PORTB.OUTTGL |= PIN5_bm; // toggle decimal point at 0.5 second intervals and reset counter
            sync_counter = 0;
        }
    }
    TCB1.INTFLAGS = TCB_CAPT_bm; // Acknowledge interrupt
} 