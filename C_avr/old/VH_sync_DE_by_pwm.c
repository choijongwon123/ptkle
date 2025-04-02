#define F_CPU 16000000UL

#define DELAY_0_1US(cycles)   \
	do                        \
	{                         \
		__asm__ __volatile__( \
			".rept %0\n\t"    \
			"nop\n\t"         \
			".endr\n\t"       \
			:                 \
			: "i"(cycles));   \
	} while (0)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile uint16_t linecount = 0;
volatile uint16_t vcount = 0;

#define SET_BIT(PORT, PIN) (PORT |= (1 << (PIN)))
#define CLEAR_BIT(PORT, PIN) (PORT &= ~(1 << (PIN)))

#define VSYNC_PIN PB4
#define HSYNC_PIN PB5

#define RED_PIN_0 PC0
#define RED_PIN_1 PC1
#define GREEN_PIN_0 PC2
#define GREEN_PIN_1 PC3
#define BLUE_PIN_0 PC4
#define BLUE_PIN_1 PC5

volatile uint8_t output_state = 0;
volatile uint16_t clock_counter = 0;

void timer1_init(void)
{
	TCCR1A = (1 << COM1A1) | (1 << COM1A0) | (1 << WGM11);

	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS31);

	ICR1 = 63;

	OCR1A = 7;

	TIMSK = (1 << TOIE1);
}

ISR(TIMER1_OVF_vect) // one line is over
{
	
	// PORTB ^= (1 << PB0);

	switch (linecount) // generating v sync
	{
	case 0:
		CLEAR_BIT(PORTB, VSYNC_PIN);
		linecount++;
		break;
	case 1:
		SET_BIT(PORTB, VSYNC_PIN);
		linecount++;
		break;
	case 525:
		linecount = 0;
		break;
	default:
		linecount++;
	}




}

void setup(void)
{

	DDRB = 0xFF;
	DDRE = 0xFF;

	MCUCR |= (1 << SE);

	SET_BIT(PORTB, VSYNC_PIN);

	timer1_init();

	sei();
}

int main(void)
{
	setup();
	while (1)
	{
	}
}