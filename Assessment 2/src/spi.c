#include "spi.h"

// Initialise SPI for communicating with Display
void spi_init(void) {
    PORTA.DIRSET = PIN1_bm;
    PORTMUX.SPIROUTEA = PORTMUX_SPI0_ALT1_gc; //SPi pins on PC0-3
    PORTC.DIR |= (PIN0_bm | PIN2_bm);  //MOSI (PC2) as outputs, because this register controls the 7 segment display

    PORTB.DIRSET = PIN1_bm; //enable PB1 as output, active high (DISP EN)
    PORTB.OUTSET = PIN1_bm; //toggle high

    SPI0.CTRLA = SPI_MASTER_bm; // Master, /4 prescaler, MSB first 
    SPI0.CTRLB = SPI_SSD_bm; // Mode 0, client select disable, unbuffered
    SPI0.INTCTRL = SPI_IE_bm;
    SPI0.CTRLA |= SPI_ENABLE_bm; //enable SPI0
}

void spi_write(uint8_t data) {
    SPI0.DATA = data;
}

// Interrupt for SPI, resets DISP LATCH
ISR(SPI0_INT_vect) {
    PORTA.OUTCLR = PIN1_bm;
    PORTA.OUTSET = PIN1_bm;
    SPI0.INTFLAGS = SPI_IF_bm;
}