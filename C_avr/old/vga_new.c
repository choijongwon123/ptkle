#define F_CPU 10000000UL // External clock frequency: 10MHz

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// 원본 이미지 크기 (64x48), 확대 배율 10을 사용하여 640×480로 확대 후 중앙 배치
#define IMAGE_WIDTH 64
#define IMAGE_HEIGHT 48
#define VGA_WIDTH 800
#define VGA_HEIGHT 600
#define HORIZONTAL_SCALE_FACTOR 10 // 확대 배율
#define VERTICAL_SCALE_FACTOR 10   // 확대 배율

// 레터박스 오프셋 (좌우: (800-640)/2, 상하: (600-480)/2)
#define LEFT_OFFSET 80
#define TOP_OFFSET 60

// Horizontal timings (pixels)
// (원래 40MHz용 값들을 10MHz에 맞게 대략 1/4로 조정)
#define H_VISIBLE_AREA 200
#define H_FRONT_PORCH 10
#define H_SYNC_PULSE 32
#define H_BACK_PORCH 22
#define H_TOTAL (H_SYNC_PULSE + H_BACK_PORCH + H_FRONT_PORCH + H_VISIBLE_AREA)

// Vertical timings (lines) – 그대로 사용
#define V_VISIBLE_AREA 600
#define V_FRONT_PORCH 1
#define V_SYNC_PULSE 4
#define V_BACK_PORCH 23
#define V_TOTAL (V_SYNC_PULSE + V_BACK_PORCH + V_FRONT_PORCH + V_VISIBLE_AREA)

#define HSYNC_PIN PB0
#define VSYNC_PIN PB1
#define RED_PIN_0 PB2
#define RED_PIN_1 PB3
#define GREEN_PIN_0 PB4
#define GREEN_PIN_1 PB5
#define BLUE_PIN_0 PB6
#define BLUE_PIN_1 PB7

#define SET_BIT(PORT, PIN) ((PORT) |= (1 << (PIN)))
#define CLEAR_BIT(PORT, PIN) ((PORT) &= ~(1 << (PIN)))

const unsigned char colorPalette[64][3] = {
    {0, 0, 0}, {0, 0, 85}, {0, 0, 170}, {0, 0, 255}, {0, 85, 0}, {0, 85, 85}, {0, 85, 170}, {0, 85, 255}, {0, 170, 0}, {0, 170, 85}, {0, 170, 170}, {0, 170, 255}, {0, 255, 0}, {0, 255, 85}, {0, 255, 170}, {0, 255, 255}, {85, 0, 0}, {85, 0, 85}, {85, 0, 170}, {85, 0, 255}, {85, 85, 0}, {85, 85, 85}, {85, 85, 170}, {85, 85, 255}, {85, 170, 0}, {85, 170, 85}, {85, 170, 170}, {85, 170, 255}, {85, 255, 0}, {85, 255, 85}, {85, 255, 170}, {85, 255, 255}, {170, 0, 0}, {170, 0, 85}, {170, 0, 170}, {170, 0, 255}, {170, 85, 0}, {170, 85, 85}, {170, 85, 170}, {170, 85, 255}, {170, 170, 0}, {170, 170, 85}, {170, 170, 170}, {170, 170, 255}, {170, 255, 0}, {170, 255, 85}, {170, 255, 170}, {170, 255, 255}, {255, 0, 0}, {255, 0, 85}, {255, 0, 170}, {255, 0, 255}, {255, 85, 0}, {255, 85, 85}, {255, 85, 170}, {255, 85, 255}, {255, 170, 0}, {255, 170, 85}, {255, 170, 170}, {255, 170, 255}, {255, 255, 0}, {255, 255, 85}, {255, 255, 170}, {255, 255, 255}};

volatile unsigned int h_count = 0;
volatile unsigned int current_x = 0;
volatile unsigned int current_y = 0;
volatile unsigned int h_sync_state = 0; // 0: back porch, 1: sync pulse, 2: front porch, 3: visible area
volatile unsigned int v_sync_state = 0; // 0: back porch, 1: sync pulse, 2: front porch, 3: visible area


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
    OCR1A = H_FRONT_PORCH;
    SET_BIT(TIMSK, OCIE1A);
}

void setup()
{
    DDRB |= 0xFF; // Port B의 모든 핀 출력

    // 초기 VGA 신호 설정
    SET_BIT(PORTB, HSYNC_PIN);
    SET_BIT(PORTB, VSYNC_PIN);
    CLEAR_BIT(PORTB, RED_PIN_0);
    CLEAR_BIT(PORTB, RED_PIN_1);
    CLEAR_BIT(PORTB, GREEN_PIN_0);
    CLEAR_BIT(PORTB, GREEN_PIN_1);
    CLEAR_BIT(PORTB, BLUE_PIN_0);
    CLEAR_BIT(PORTB, BLUE_PIN_1);

    timer_init();
    sei(); // 전역 인터럽트 활성화
}

ISR(TIMER1_COMPA_vect)
{
   if (h_sync_state == 0) { // 수평 백포치상태일 때 
    OCR1A = H_SYNC_PULSE;
    CLEAR_BIT(PORTB, HSYNC_PIN);
    h_sync_state = 1;
  } else if (h_sync_state == 1) { // 수평 싱크 펄스
    OCR1A = H_FRONT_PORCH;
    SET_BIT(PORTB, HSYNC_PIN);
    h_sync_state = 2;
  } else if (h_sync_state == 2) { // 수평 프론트 포치
    OCR1A = H_VISIBLE_AREA;
    SET_BIT(PORTB, HSYNC_PIN);
    h_sync_state = 3;
    current_x = 0;
   } elese if (h_sync_state == 3) { // Visible Area 상태일 떄 
    OCRA1A = H_BACK_PORCH;
    SET_BIT(PORTB, HSYNC_PIN);
    h_sync__state = 0;
    }

}

void loop()
{
    while (1)
    {
        // 메인 루프: 인터럽트에 의해 VGA 신호가 생성됨
    }
}

int main(void)
{
    setup();
    loop();
    return 0;
}