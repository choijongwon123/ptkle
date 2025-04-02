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

volatile uint16_t linecount = 0;

volatile uint8_t color_index = 0;

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

const unsigned char colorPalette[64][3] = {
    {0, 0, 0},
    {85, 0, 0},
    {85, 85, 85},
    {170, 0, 0},
    {170, 85, 85},
    {170, 170, 170},
    {255, 0, 0},
    {255, 85, 85},
    {255, 170, 170},
    {255, 255, 255},
    {255, 85, 0},
    {170, 85, 0},
    {255, 170, 85},
    {255, 170, 0},
    {85, 85, 0},
    {170, 170, 0},
    {170, 170, 85},
    {255, 255, 0},
    {255, 255, 85},
    {255, 255, 170},
    {170, 255, 0},
    {85, 170, 0},
    {170, 255, 85},
    {85, 255, 0},
    {0, 85, 0},
    {0, 170, 0},
    {0, 255, 0},
    {85, 170, 85},
    {85, 255, 85},
    {170, 255, 170},
    {0, 255, 85},
    {0, 170, 85},
    {85, 255, 170},
    {0, 255, 170},
    {0, 85, 85},
    {0, 170, 170},
    {0, 255, 255},
    {85, 170, 170},
    {85, 255, 255},
    {170, 255, 255},
    {0, 170, 255},
    {0, 85, 170},
    {85, 170, 255},
    {0, 85, 255},
    {0, 0, 85},
    {0, 0, 170},
    {0, 0, 255},
    {85, 85, 170},
    {85, 85, 255},
    {170, 170, 255},
    {85, 0, 255},
    {85, 0, 170},
    {170, 85, 255},
    {170, 0, 255},
    {85, 0, 85},
    {170, 0, 170},
    {170, 85, 170},
    {255, 0, 255},
    {255, 85, 255},
    {255, 170, 255},
    {255, 0, 170},
    {170, 0, 85},
    {255, 85, 170},
    {255, 0, 85},
};

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


#define H_SYNC_LOW 32
#define H_SYNC_WHOLE 264
#define V_SYNC_LOW 4
#define V_SYNC_WHOLE 628

#define VFP 1
#define VBP 23

#define HFP 10
#define HBP 22



void timer1_init(void)
{
	TCCR1A = (1 << COM1A1) | (1 << COM1A0) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);

	ICR1 = H_SYNC_WHOLE;

	OCR1A = H_SYNC_LOW - 1;

	TIMSK = (1 << TOIE1);
}

/*
void timer3_init(void)
{
    // OC3A 핀(예: ATmega128에서는 PH3)을 출력으로 설정 (MCU에 따라 수정)
    // 타이머3을 Fast PWM 모드 14 (WGM33:0 = 1110)로 설정하고 inverting 모드 사용:
    // inverting 모드: COM3A1:0 = 11 (출력이 BOTTOM에서 클리어되고, 비교 매치 후 SET됨)
    TCCR3A = (1 << COM3A1) | (1 << COM3A0) | (1 << WGM31); 
    TCCR3B = (1 << WGM33) | (1 << WGM32) | (1 << CS30);  // 분주비 1

    // TOP값 설정: 0 ~ 263 (264 클럭 주기)
    ICR3 = 263;

    // inverting 모드에서 출력은 0부터 OCR3A까지 LOW, 그 다음 HIGH가 됨.
    // 64 클럭 동안 LOW를 원하므로 OCR3A를 63으로 설정
    OCR3A = 63;
}

*/

ISR(TIMER1_OVF_vect)
{
	//SFIOR |= (1 << TSM);

	//SFIOR &= ~(1 << TSM);

	DELAY_0_1US(129);

	//TCNT3 = 255;

	// PORTB ^= (1 << PB0);

	switch (linecount)
	{
	case 0:
		CLEAR_BIT(PORTB, VSYNC_PIN);
		setVGAColor(0);
		linecount++;
		break;
	case V_SYNC_LOW / 2:
		SET_BIT(PORTB, VSYNC_PIN);
		setVGAColor(color_index);
		color_index = (color_index + 1) % 64;	
		linecount++;
		break;
	case V_SYNC_WHOLE / 2:
		linecount = 0;
		break;
	default:
		linecount++;
	}
}

void setup(void)
{

	DDRB = 0xFF;
	DDRC = 0xFF;
	PORTC = 0x00;

	SET_BIT(PORTB, VSYNC_PIN);

	timer1_init();
	//timer3_init();

	sei();
}

int main(void)
{
	setup();
	while (1)
	{
		
	}
}