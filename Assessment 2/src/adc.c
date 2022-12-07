#include "adc.h"

// This function initialises the ADC in 8-bit single conversion mode.
// The adc samples the voltage on the POT NET, put into freerunning mode and started.
void adc_init(void) {
    ADC0.CTRLA = ADC_ENABLE_bm; // Enable ADC
    ADC0.CTRLB = ADC_PRESC_DIV2_gc; // 2 Clock prescaler
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp) | ADC_REFSEL_VDD_gc; // Need 4 CLK_PER cycles @ 3.3 MHz for 1us, select VDD as ref
    ADC0.CTRLE = 64; // Sample duration of 64
    ADC0.CTRLF = ADC_FREERUN_bm | ADC_LEFTADJ_bm; // Free running, left adjust result
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc; // Select AIN2 (potentiomenter R1)
    ADC0.COMMAND = ADC_MODE_SINGLE_8BIT_gc | ADC_START_IMMEDIATE_gc; // Select 12-bit resolution, single-ended
}