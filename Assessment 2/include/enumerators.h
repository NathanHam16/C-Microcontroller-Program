#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

typedef enum {
    SEQUENCE_SELECT,
    SEQUENCING,
    TEST
} mode;

typedef enum {
    START,
    PAUSED,
} sequencing;

typedef enum {
    STARTESCAPE,
    BYTE2,
    BYTE3,
    SEQUENCE_INDEX
} serial;