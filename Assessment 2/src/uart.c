#include "uart.h"

// This src file initialises the UART peripheral for reading and writing serial data. 
// Initialised into 9600 baud, 8N1 mode.
void uart_init(void) {
    PORTB.DIRSET = PIN2_bm; // Enable PB2 as output (USART0 TXD)
    USART0.BAUD = 1389;     // 9600 baud @ 3.3 MHz
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm; // Enable TX/RX
}


uint8_t uart_has_data(void) { // Unblocking function which returns uart data, 
    return USART0.STATUS & USART_RXCIF_bm;
}

uint8_t uart_getc(void) {
    while (!uart_has_data()); // Wait for data
    return USART0.RXDATAL;
}

void uart_putc(uint8_t c) {
    while (!(USART0.STATUS & USART_DREIF_bm));  // Wait for TXDATA empty
    USART0.TXDATAL = c;
}

// Uses uart_putc to write a string to serial console.
void uart_puts(char* string) {
    uint8_t i = 0;
    while (string[i] != '\0') {
        uart_putc(string[i++]);
    }
}
