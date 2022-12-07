#include "tca.h"
#include "spi.h"

// Stores the note count of PER in a size 12 array, where octave halves the count
uint32_t count[] = {
    121212,
    114408,
    107988,
    101927,
    96206,
    90806,
    85710,
    80899,
    76359,
    72073,
    68028,
    64210
};
extern uint8_t descrambled[3];
uint8_t octave;
uint8_t note;
uint8_t brightness; 
uint32_t counter;
uint32_t PER = 0;
extern uint8_t duration; 

// Initialises Timer counter A which interacts with the LED and buzzer.
void tca_init(void) {
    // Enable buzzer and LED as outputs
    PORTB.DIRSET = PIN0_bm; // Clearing active high
    PORTB.DIRSET = PIN1_bm; // Setting active low
    PORTB.OUTSET = PIN0_bm | PIN1_bm;

    cli();
    // Change default port to wo0 on pb0 and w01 pb1(buzzer and led)
    PORTMUX.TCAROUTEA = PORTMUX_TCA00_DEFAULT_gc | PORTMUX_TCA01_DEFAULT_gc;
    // Configure single slope mode and enables cmp0 and cmp1 for buzzer and led respectively.
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_CMP1EN_bm;
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
    // Set period to 1 ms, buzzer duty cycle 0, brightness duty cycle 100%
    TCA0.SINGLE.PER = 3333;
    TCA0.SINGLE.CMP0 = 0;
    TCA0.SINGLE.CMP1 = 3334;
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; 
    sei();
}

void tca_step(void) {
    // Bitmasks on descrambled to fetch the brightmess, duration, and frequency from each byte.
    // PER is the number of counts for the desired frequency, where octave divides count by 2.
    PORTB.OUTSET = PIN0_bm; 
    brightness = descrambled[1];
    note = descrambled[2] & 0x0F;
    octave = descrambled[2] >> 4 & 0x0F;
    PER = count[note] >> octave; // duty cycle / 3333
}

// Interrupt for TCA0. Alters buzzer and LED according to programme specifications.
ISR(TCA0_OVF_vect) {
    if (octave == 0) {
        TCA0.SINGLE.CMP0BUF = 0;
    }
    else {
        TCA0.SINGLE.PERBUF = PER;
        TCA0.SINGLE.CMP0BUF = PER >> 1;
    }
    // When duration is greater than 0, LED is altered.
    if (duration > 0) {
        spi_write(0b10000000);
        if (brightness == 0) TCA0.SINGLE.CMP1BUF = 0;
        else TCA0.SINGLE.CMP1BUF = ((brightness + 1) *PER) >> 8;
    }
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

