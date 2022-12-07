#include "serial_protocol.h"
#include "enumerators.h"
#include "uart.h"
#include "tcb.h"
#include "tca.h"
#include "decoding.h"
#include "sequencing.h"
#include "test_mode.h"
#include "descrambling.h"

// Declare variables, and using external variables to control sequence termination
uint8_t getc;
serial serial_mode = STARTESCAPE;
extern uint8_t sequence_index;
extern mode current_mode;
extern uint8_t descrambled[3];
extern uint16_t duration_counter;;
extern uint8_t disable_tcb1;
extern uint8_t sequence_complete;
extern uint8_t sync_enable;
extern uint16_t sync_counter;
uint32_t payload[33];
uint32_t payload32[8];
uint8_t decoded_bytes[3];
uint8_t descrambled_bytes[3];
uint8_t payload_sequence[32];

// Parser service for serial protocol. Uses unblocking function uart_has_data
void parser_service(void) {
    if (uart_has_data()) {
        uint8_t getc = uart_getc();
        
        switch(serial_mode) {
            // Interpreted from APPENDIX 3.2 to stay in STARTESCAPE if invalid input and do not respond,
            // as the programme only responds to messages 'beginning with a valid start-escape sequence'.
            case STARTESCAPE:
                if (getc == '\\') {
                    if (uart_getc() == '\\') { // Two \\ in a row to transition serial_mode to BYTE2.
                        serial_mode = BYTE2;
                        break;
                    }
                    else serial_mode = STARTESCAPE;
                    break;
                }
                else serial_mode = STARTESCAPE;
                break;
            case BYTE2:
                // CMD_SYNC Toggles PORTB Pin 5 at 1Hz, and uses read-modify-write to not overwrite the existing PB1 data.
                if (getc == 'y') { // CMD_SYNC
                    uart_puts("#ACK\n");
                    sync_enable = 1; // Enable sync in TCB1
                    sync_counter = 0; // reset counter
                    PORTB.DIR |= PIN5_bm;
                    PORTB.OUTCLR |= PIN5_bm; // Turn PB5 On
                    serial_mode = STARTESCAPE;
                    break;
                }
                if (current_mode == SEQUENCE_SELECT) { // Only in sequence select mode
                    if (getc == 's') { // CMD_SEQ, Change cucrent mode to sequencing and serial mode to byte0
                        uart_puts("#ACK\n");
                        current_mode = SEQUENCING;
                        serial_mode = STARTESCAPE;
                        break;
                    }
                    else if (getc == 't') { // CMD_TEST, change mode and serial mode to byte0
                        uart_puts("#ACK\n");
                        current_mode = TEST;
                        serial_mode = STARTESCAPE;
                        break;
                    }
                    else if (getc == 'i') { // CMD_SEQIDX, parse user input and convert to hex
                        serial_mode = SEQUENCE_INDEX;
                        break;
                    }
                    else { 
                        uart_puts("#NACK\n");
                        serial_mode = STARTESCAPE;
                        break;
                    }
                }
                else if (current_mode == SEQUENCING) { // Only in sequencing mode
                    if (getc == 'e') { // CMD_EXIT
                        uart_puts("#ACK\n");
                        serial_mode = STARTESCAPE;
                        descrambled[2] = 0; // Set note and octave to 0
                        tca_step(); // Execute step
                        duration_counter = 0; // Reset duration_counter
                        disable_tcb1 = 1; // Disable tcb1
                        sequence_complete = 1; // Exits while loop in sequencing.c
                        break;
                    }
                    else if (getc == 'p') { // CMD_PAUSE, disable tcb1
                        disable_tcb1 = 1;
                        uart_puts("#ACK\n");
                        serial_mode = STARTESCAPE;
                        break;
                    }
                    else if (getc == 'n') { // CMD_STEP
                        if (disable_tcb1 == 1) {
                            fetch_bytes(); // Fetch new bytes and execute step, reset duration_counter
                            tca_step(); 
                            duration_counter = 0;
                            if (descrambled[0] == 0) { // If duration = 0, terminate sequencing
                                disable_tcb1 = 1; 
                                sequence_complete = 1;
                            }
                            else serial_mode = STARTESCAPE;
                        }
                        else {
                            uart_puts("#NACK\n");
                            serial_mode = STARTESCAPE;
                        }
                        break;
                    }
                    else if (getc == 's') { // CMD_SEQ, fetch next byte, reset duration and enable tcb1
                        if (disable_tcb1 == 1) {
                            fetch_bytes();
                            duration_counter = 0;
                            disable_tcb1 = 0;
                            uart_puts("#ACK\n");
                            serial_mode = STARTESCAPE;
                            break;
                        }
                    }
                    else { 
                        uart_puts("#NACK\n");
                        serial_mode = STARTESCAPE;
                        break;
                    }
                } 
                else if (current_mode == TEST) { // Only in testing mode
                    // CMD_TEST specifications deleted
                    // Gave up because 3.3MHz is too difficult to achieve 0.2% accuracy.
                    if (getc == 'e') { // CMD_EXIT
                        uart_puts("#ACK\n");
                        serial_mode = STARTESCAPE;
                        descrambled[2] = 0; // Set note and octave to 0
                        tca_step(); // Execute step
                        duration_counter = 0; // Reset duration_counter
                        disable_tcb1 = 1; // Disable tcb1
                        sequence_complete = 1; // Exits while loop in sequencing.c
                        break;
                    }
                }
            case SEQUENCE_INDEX: ;
                // validates two chars and sets the value to sequence index and display.
                uint8_t char1 = getc;
                if (((char1 >= 48) && (char1 <= 57)) | ((char1 >= 97) && (char1 <= 102))) { // valid hex a-f 0-9
                    uint8_t hex1 = char_to_hex(char1); // converts char to hex
                    uint8_t char2 = uart_getc();
                    if (((char2 >= 48) && (char2 <= 57)) | ((char2 >= 97) && (char2 <= 102))) {
                        uint8_t hex2 = char_to_hex(char2);
                        sequence_index = (hex1 << 4) | hex2; 
                        display_hex(sequence_index);
                        uart_puts("#ACK\n");
                        serial_mode = STARTESCAPE;
                    }
                    else {
                        uart_puts("#NACK\n");
                        serial_mode = STARTESCAPE;
                    }
                }
                else {
                    uart_puts("#NACK\n");
                    serial_mode = STARTESCAPE;
                }
            default:
                serial_mode = STARTESCAPE;
        
            



        //     else if (getc == 'e') { // CMD_EXIT
        //     }
        //     else if (getc == 'p') { // CMD_PAUSE
        //     }
        //     else if (getc == 'n') { // CMD_STEP
        //     }
        //     else if (getc == 'y') { // CMD_SYNC
        //     }
        //     else if (getc == 'd') { // CMD_TEST_SEQ
        //     }
        //     else if (getc == 'u') { // CMD_TEST_SEQNS
        //     }
        //     else { 
        //         uart_puts("\n, #NACK");
        //         serial_mode = BYTE0;
        //     }
        //     break;            
        // case BYTE3:
        //     break;

        }
    }       
}
