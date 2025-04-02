#define F_CPU 10000000UL

#define DELAY_0_1US(cycles)            \
    do {                             \
        __asm__ __volatile__(       \
            ".rept %0\n\t"         \
            "nop\n\t"              \
            ".endr\n\t"             \
            :                      \
            : "i" (cycles)         \
       );                          \
    } while(0)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Global variable for counting HSYNC pulses in Timer3's Input Capture ISR.
volatile uint16_t vsync_line_count = 0;

// Macros for bit manipulation
#define SET_BIT(PORT, PIN) (PORT |= (1 << (PIN)))
#define CLEAR_BIT(PORT, PIN) (PORT &= ~(1 << (PIN)))

// Pin definitions; HSYNC is output on OC1A (PB1) and VSYNC on PB0.
#define VSYNC_PIN PB4
#define HSYNC_PIN PB5

#define RED_PIN_0 PB2
#define RED_PIN_1 PB3
#define GREEN_PIN_0 PB4
#define GREEN_PIN_1 PB5
#define BLUE_PIN_0 PB6
#define BLUE_PIN_1 PB7

#define H_SYNC_LOW 32
#define H_SYNC_WHOLE 264
#define V_SYNC_LOW 4
#define V_SYNC_HIGH 624

volatile uint8_t output_state = 0; // 0: LOW, 1: HIGH
volatile uint16_t clock_counter = 0;

// Timer1 initialization to generate HSYNC via Fast PWM mode 14.
// Total period = 264 clock cycles (ICR1=263)
// HSYNC low pulse = 32 clock cycles (OCR1A=31) (active low in inverting mode)
void timer1_init_pwm(void)
{
	// Configure OC1A (HSYNC) as output.
	SET_BIT(DDRB, HSYNC_PIN);

	// Configure Timer1 for Fast PWM mode 14: TOP=ICR1.
	// Inverting mode is used so that the output is low for (OCR1A+1) counts.
	// TCCR1A: set COM1A1 and COM1A0; set WGM11.
	// TCCR1B: set WGM13 and WGM12; and use no prescaling (CS10).
	TCCR1A = (1 << COM1A1) | (1 << COM1A0) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);

	// Set total period: 264 clock cycles.
	ICR1 = H_SYNC_WHOLE;

	// Set pulse width: 32 cycles low (since OCR1A+1 = 32).
	OCR1A = H_SYNC_LOW - 1;
}





// Timer3 initialization to use the external clock on T3 pin (falling edge).
// This configuration enables the Input Capture interrupt (TIMER3_CAPT_vect)
// which occurs when a falling edge is detected. Ensure you have a jumper
// connecting the HSYNC output (OC1A on PB1) to the T3 pin.
void timer3_init_ext(void)
{
	TCCR3A = 0;											// (WGM31:WGM30 = 00)
	TCCR3B = (1 << WGM32)								// Set CTC mode (WGM33 = 0, WGM32 = 1)
			 | (1 << CS32) | (1 << CS31) | (1 << CS30); // External clock source bits (verify in datasheet)

	// Set the compare value for OCR3A to control the VSYNC timing.
	OCR3A = V_SYNC_HIGH - 1; // e.g. such that it represents one horizontal line period

	// Enable the Compare Match A interrupt for Timer3:
	ETIMSK |= (1 << OCIE3A);
}

// Timer3 Input Capture ISR
// Each capture event is triggered at a falling edge of HSYNC.
// VSYNC is forced low for the first 4 lines and then high for the remaining field.
ISR(TIMER3_COMPA_vect)
{
	DELAY_0_1US(202);

	if (OCR3A == V_SYNC_LOW - 1)
	{
		SET_BIT(PORTB, VSYNC_PIN);
		OCR3A = V_SYNC_HIGH - 1;
	}
	else
	{
		CLEAR_BIT(PORTB, VSYNC_PIN);
		OCR3A = V_SYNC_LOW - 1;
		
	}

	// When the full field of 624 lines has been reached, reset the counter.
}

void setup(void)
{
	// Set all Port B pins as output.
	DDRB = 0xFF;

	// Initialize VGA signals:
	// Clear VSYNC and color signals.
	CLEAR_BIT(PORTB, VSYNC_PIN);

	/*
	CLEAR_BIT(PORTB, RED_PIN_0);
	CLEAR_BIT(PORTB, RED_PIN_1);
	CLEAR_BIT(PORTB, GREEN_PIN_0);
	CLEAR_BIT(PORTB, GREEN_PIN_1);
	CLEAR_BIT(PORTB, BLUE_PIN_0);
	CLEAR_BIT(PORTB, BLUE_PIN_1);
	*/

	// Initialize Timer1 to generate HSYNC.
	timer1_init_pwm();

	// Initialize Timer3's input capture for counting HSYNC edges,
	// which will drive VSYNC generation.
	timer3_init_ext();

	sei(); // Enable global interrupts.
}

int main(void)
{
	setup();
	while (1)
	{		
		// Main loop can remain empty. VSYNC generation is handled in the ISR.
	}
}