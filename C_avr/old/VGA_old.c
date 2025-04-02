#define F_CPU 10000000UL // External clock frequency: 10MHz

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// 원본 이미지 크기 (64x48), 확대 배율 10을 사용하여 640×480로 확대 후 중앙에 배치
#define IMAGE_WIDTH 64
#define IMAGE_HEIGHT 48
#define VGA_WIDTH 800
#define VGA_HEIGHT 600
#define HORIZONTAL_SCALE_FACTOR 10  // 확대 배율
#define VERTICAL_SCALE_FACTOR 10    // 확대 배율

// 레터박스 오프셋 (좌우: (800-640)/2, 상하: (600-480)/2)
#define LEFT_OFFSET 80
#define TOP_OFFSET 60

// Horizontal timings (pixels):
#define H_VISIBLE_AREA 200
#define H_FRONT_PORCH 10
#define H_SYNC_PULSE 32
#define H_BACK_PORCH 22
#define H_TOTAL (H_SYNC_PULSE + H_BACK_PORCH + H_FRONT_PORCH + H_VISIBLE_AREA)

// Vertical timings (lines):
#define V_VISIBLE_AREA 600
#define V_FRONT_PORCH 1
#define V_SYNC_PULSE 4
#define V_BACK_PORCH 23
#define V_TOTAL (V_SYNC_PULSE + V_BACK_PORCH + V_FRONT_PORCH + V_VISIBLE_AREA)

#define HSYNC_PIN   PB0
#define VSYNC_PIN   PB1
#define RED_PIN_0   PB2
#define RED_PIN_1   PB3
#define GREEN_PIN_0 PB4
#define GREEN_PIN_1 PB5
#define BLUE_PIN_0  PB6
#define BLUE_PIN_1  PB7

#define SET_BIT(PORT, PIN) ((PORT) |= (1 << (PIN)))
#define CLEAR_BIT(PORT, PIN) ((PORT) &= ~(1 << (PIN)))

// colorPalette 배열 (필요하다면 PROGMEM으로 옮기기)
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
	{255, 255, 255}
};

// image_data 배열 (내부 SRAM에 할당되며, 총 64*48 = 3072바이트 사용)
unsigned char image_data[IMAGE_HEIGHT][IMAGE_WIDTH];

void write_image()
{
	int data = 63;
	// 원본 이미지 데이터를 흰색(높은 색상 인덱스)에서 검정색 방향으로 초기화
	for (int y = 0; y < IMAGE_HEIGHT; y++)
	{
		for (int x = 0; x < IMAGE_WIDTH; x++)
		{
			image_data[y][x] = data;
		}
		//data--;
	}
}

volatile unsigned int h_count = 0;
volatile unsigned int v_count = 0;
volatile unsigned int current_x = 0;
volatile unsigned int current_y = 0;

void timer_init()
{
	// Timer1 설정 (HSYNC 생성용, 16비트, 프리스케일러 1, CTC 모드)
	TCCR1A = 0;
	CLEAR_BIT(TCCR1B, WGM10);
	SET_BIT(TCCR1B, WGM12);
	CLEAR_BIT(TCCR1B, CS11);
	CLEAR_BIT(TCCR1B, CS12);
	SET_BIT(TCCR1B, CS10);
	TCCR1C = 0;
	OCR1A = H_TOTAL - 1;
	SET_BIT(TIMSK, OCIE1A);

}

void setup()
{
	// VGA 신호 핀 설정
	SET_BIT(DDRB, HSYNC_PIN);
	SET_BIT(DDRB, VSYNC_PIN);
	SET_BIT(DDRB, RED_PIN_0);
	SET_BIT(DDRB, RED_PIN_1);
	SET_BIT(DDRB, GREEN_PIN_0);
	SET_BIT(DDRB, GREEN_PIN_1);
	SET_BIT(DDRC, BLUE_PIN_0);
	SET_BIT(DDRC, BLUE_PIN_1);

	// 초기 VGA 신호 설정
	SET_BIT(PORTB, HSYNC_PIN);
	SET_BIT(PORTB, VSYNC_PIN);
	CLEAR_BIT(PORTB, RED_PIN_0);
	CLEAR_BIT(PORTB, RED_PIN_1);
	CLEAR_BIT(PORTB, GREEN_PIN_0);
	CLEAR_BIT(PORTB, GREEN_PIN_1);
	CLEAR_BIT(PORTC, BLUE_PIN_0);
	CLEAR_BIT(PORTC, BLUE_PIN_1);

	timer_init();
	sei(); // 전역 인터럽트 활성화
}

void setVGAColor(unsigned char colorIndex)
{
	if (colorIndex >= 64)
	colorIndex = 0;

	unsigned char red = colorPalette[colorIndex][0];
	unsigned char green = colorPalette[colorIndex][1];
	unsigned char blue = colorPalette[colorIndex][2];

	// 2비트 Red
	if (red == 85 || red == 255)
	SET_BIT(PORTB, RED_PIN_0);
	else
	CLEAR_BIT(PORTB, RED_PIN_0);
	if (red == 170 || red == 255)
	SET_BIT(PORTB, RED_PIN_1);
	else
	CLEAR_BIT(PORTB, RED_PIN_1);

	// 2비트 Green
	if (green == 85 || green == 255)
	SET_BIT(PORTB, GREEN_PIN_0);
	else
	CLEAR_BIT(PORTB, GREEN_PIN_0);
	if (green == 170 || green == 255)
	SET_BIT(PORTB, GREEN_PIN_1);
	else
	CLEAR_BIT(PORTB, GREEN_PIN_1);

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

void display_pixel()
{
	// 이미지 영역: 가로 80~719, 세로 60~539 (확대 이미지: 640x480)
	if (current_x >= LEFT_OFFSET && current_x < (LEFT_OFFSET + IMAGE_WIDTH * HORIZONTAL_SCALE_FACTOR) &&
	current_y >= TOP_OFFSET && current_y < (TOP_OFFSET + IMAGE_HEIGHT * VERTICAL_SCALE_FACTOR))
	{
		unsigned int image_x = (current_x - LEFT_OFFSET) / HORIZONTAL_SCALE_FACTOR;
		unsigned int image_y = (current_y - TOP_OFFSET) / VERTICAL_SCALE_FACTOR;
		if (image_x < IMAGE_WIDTH && image_y < IMAGE_HEIGHT)
		{
			unsigned char colorIndex = image_data[image_y][image_x];
			setVGAColor(colorIndex);
		}
		else
		{
			setVGAColor(0);
		}
	}
	else
	{
		// 이미지 영역이 아니면 검정색 출력
		setVGAColor(0);
	}
}

// (HSYNC) 인터럽트 - 수평 타이밍 처리
ISR(TIMER1_COMPA_vect)
{
	h_count++;

	if (h_count <= H_SYNC_PULSE)
	{
		CLEAR_BIT(PORTB, HSYNC_PIN);
	}
	else if (h_count <= H_SYNC_PULSE + H_BACK_PORCH)
	{
		SET_BIT(PORTB, HSYNC_PIN);
	}
	else if (h_count <= H_SYNC_PULSE + H_BACK_PORCH + H_VISIBLE_AREA)
	{
		SET_BIT(PORTB, HSYNC_PIN);
		display_pixel();
		current_x++;
		if (current_x >= VGA_WIDTH)
		current_x = 0;
	}
	else if (h_count <= H_TOTAL)
	{
		SET_BIT(PORTB, HSYNC_PIN);
	}
	else
	{
		h_count = 0;
		current_x = 0;
		current_y++;
        v_count++;
        if (current_y >= VGA_HEIGHT)
		current_y = 0;

	}







    
    
        
}



void loop()
{
	while (1)
	{
	}
}

int main(void)
{
	setup();
	write_image();
	loop();
	return 0;
}






