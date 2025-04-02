#define F_CPU 10000000UL // External clock frequency: 10MHz

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define LINE_CYCLES 264
#define HSYNC_CYLCLES 32
#define VSYNC_LINES 4
#define FRAME_LINES 628

volatile int linecount;

void setup()
{

    DDRD = 0x03;        // PD0, PD1 as output
    PORTD = 0x03;       // PD0, PD1 high

    TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(WGM11); // Clear OC1A/OC1B on Compare Match, set OC1A/OC1B at BOTTOM (non-inverting mode)
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10); // Fast PWM, 8-bit, no prescaler

    ICR1 = LINE_CYCLES;    // Overflow at Cycles per line
    OCR1A = HSYNC_CYLCLES; // Compare high after HSync cycles
}

void loop() {}