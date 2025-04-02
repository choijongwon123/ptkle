#define F_CPU 10000000UL

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

// 원본 이미지 크기 (64x48), 확대 배율 10을 사용하여 640×480로 확대 후 중앙에 배치

// Macros for bit manipulation
#define SET_BIT(PORT, PIN) (PORT |= (1 << (PIN)))
#define CLEAR_BIT(PORT, PIN) (PORT &= ~(1 << (PIN)))

// V/H pin Definition
#define VSYNC_PIN PB4
#define HSYNC_PIN PB5
// RGB pin DEFINITION

#define RED_PIN_0 PC0
#define RED_PIN_1 PC1
#define GREEN_PIN_0 PC2
#define GREEN_PIN_1 PC3
#define BLUE_PIN_0 PC4
#define BLUE_PIN_1 PC5

// values for v/h sync
#define H_SYNC_LOW 32
#define H_SYNC_WHOLE 264
#define V_SYNC_LOW 4
#define V_SYNC_HIGH 624

#define H_FRONT_PORCH 10
#define H_BACK_PORCH 22
#define H_VISIBLE_AREA 200
#define V_FRONT_PORCH 1
#define V_BACK_PORCH 23
#define V_VISIBLE_AREA 600

const unsigned char colorPalette[64][3] = {
	{0, 0, 0},
	{0, 0, 85},
	{0, 0, 170},
	{0, 0, 255},
	{0, 85, 0},
	{0, 85, 85},
	{0, 85, 170},
	{0, 85, 255},
	{0, 170, 0},
	{0, 170, 85},
	{0, 170, 170},
	{0, 170, 255},
	{0, 255, 0},
	{0, 255, 85},
	{0, 255, 170},
	{0, 255, 255},
	{85, 0, 0},
	{85, 0, 85},
	{85, 0, 170},
	{85, 0, 255},
	{85, 85, 0},
	{85, 85, 85},
	{85, 85, 170},
	{85, 85, 255},
	{85, 170, 0},
	{85, 170, 85},
	{85, 170, 170},
	{85, 170, 255},
	{85, 255, 0},
	{85, 255, 85},
	{85, 255, 170},
	{85, 255, 255},
	{170, 0, 0},
	{170, 0, 85},
	{170, 0, 170},
	{170, 0, 255},
	{170, 85, 0},
	{170, 85, 85},
	{170, 85, 170},
	{170, 85, 255},
	{170, 170, 0},
	{170, 170, 85},
	{170, 170, 170},
	{170, 170, 255},
	{170, 255, 0},
	{170, 255, 85},
	{170, 255, 170},
	{170, 255, 255},
	{255, 0, 0},
	{255, 0, 85},
	{255, 0, 170},
	{255, 0, 255},
	{255, 85, 0},
	{255, 85, 85},
	{255, 85, 170},
	{255, 85, 255},
	{255, 170, 0},
	{255, 170, 85},
	{255, 170, 170},
	{255, 170, 255},
	{255, 255, 0},
	{255, 255, 85},
	{255, 255, 170},
	{255, 255, 255}};

volatile uint16_t hCounter = 0; // horizontal pixel clock counter
volatile uint16_t vCounter = 0; // vertical line counter
volatile uint8_t color = 1;

void setVGAColor(unsigned char colorIndex)
{
	if (colorIndex >= 64)
		colorIndex = 0;

	unsigned char red = colorPalette[colorIndex][0];
	unsigned char green = colorPalette[colorIndex][1];
	unsigned char blue = colorPalette[colorIndex][2];

	// 2비트 Red
	if (red == 85 || red == 255)
		SET_BIT(PORTC, RED_PIN_0);
	else
		CLEAR_BIT(PORTC, RED_PIN_0);
	if (red == 170 || red == 255)
		SET_BIT(PORTC, RED_PIN_1);
	else
		CLEAR_BIT(PORTC, RED_PIN_1);

	// 2비트 Green
	if (green == 85 || green == 255)
		SET_BIT(PORTC, GREEN_PIN_0);
	else
		CLEAR_BIT(PORTC, GREEN_PIN_0);
	if (green == 170 || green == 255)
		SET_BIT(PORTC, GREEN_PIN_1);
	else
		CLEAR_BIT(PORTC, GREEN_PIN_1);

	// 2비트 Blue
	if (blue == 85 || blue == 255)
		SET_BIT(PORTC, BLUE_PIN_0);
	else
		CLEAR_BIT(PORTC, BLUE_PIN_0);
	if (blue == 170 || blue == 255)
		SET_BIT(PORTC, BLUE_PIN_1);
	else
		CLEAR_BIT(PORTC, BLUE_PIN_1);
}

// Timer1 initialization to generate HSYNC via Fast PWM mode 14.
// Total period = 264 clock cycles (ICR1=263)
// HSYNC low pulse = 32 clock cycles (OCR1A=31) (active low in inverting mode)
void timer1_init(void)
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

	TIMSK |= (1 << OCIE1A);
}

void timer3_init(void)
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
	DDRC = 0xFF;

	// Initialize VGA signals:
	// Clear VSYNC and color signals.
	CLEAR_BIT(PORTB, VSYNC_PIN);

	CLEAR_BIT(PORTC, RED_PIN_0);
	CLEAR_BIT(PORTC, RED_PIN_1);
	CLEAR_BIT(PORTC, GREEN_PIN_0);
	CLEAR_BIT(PORTC, GREEN_PIN_1);
	CLEAR_BIT(PORTC, BLUE_PIN_0);
	CLEAR_BIT(PORTC, BLUE_PIN_1);

	// Initialize Timer1 to generate HSYNC.
	timer1_init();
	// timer2_init();
	timer3_init();
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