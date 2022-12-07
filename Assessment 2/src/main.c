// n11240164
#include "enumerators.h"
#include "adc.h"
#include "spi.h"
#include "debouncing.h"
#include "adc.h"
#include "tca.h"
#include "tcb.h"
#include "uart.h"
#include "sequencing.h"
#include "serial_protocol.h"
#include "test_mode.h"

mode current_mode = SEQUENCE_SELECT;

int main(void) {
    
    cli();
    spi_init();
    buttons_init();
    tca_init();
    tcb0_init();
    tcb1_init();
    uart_init();
    adc_init();
    sei();

    while(1) {
        parser_service();
        pb_init();
        switch(current_mode) {
            case SEQUENCE_SELECT:
                get_index();
                break;
            case SEQUENCING:
                sequence_init();
                break;
            case TEST:
                test_mode();
                break;
            default: 
                current_mode = SEQUENCE_SELECT;
        } 
    }
}

