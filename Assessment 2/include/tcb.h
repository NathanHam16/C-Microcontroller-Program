#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

void tcb0_init(void);
void tcb1_init(void);
void display_hex(uint8_t value);