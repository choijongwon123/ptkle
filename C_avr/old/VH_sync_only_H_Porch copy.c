#define F_CPU 10000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// 미세 지연 매크로 (0.1us 단위)
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

// 핀 정의
#define VSYNC_PIN PB4
#define HSYNC_PIN PB5
#define DE_PIN    PB7  // 최종 DE 출력

// Horizontal 타이밍 (클럭 기준)
#define H_FP      10
#define H_PULSE   32
#define H_BP      22
#define H_VISIBLE 200
#define H_TOTAL   (H_FP + H_PULSE + H_BP + H_VISIBLE)  // 264 클럭

// 수평 visible 구간: [H_FP+H_PULSE+H_BP, H_TOTAL)
#define H_VISIBLE_START (H_FP + H_PULSE + H_BP)        // 64
#define H_VISIBLE_END   (H_VISIBLE_START + H_VISIBLE)    // 264

// Vertical 타이밍 (라인 기준)
#define V_FP      1
#define V_PULSE   4
#define V_BP      23
#define V_VISIBLE 600
#define V_TOTAL   (V_FP + V_PULSE + V_BP + V_VISIBLE)     // 628 라인

// 수직 visible 구간: [V_FP+V_PULSE+V_BP, V_TOTAL)
#define V_VISIBLE_START (V_FP + V_PULSE + V_BP)          // 28
#define V_VISIBLE_END   (V_VISIBLE_START + V_VISIBLE)      // 628

// 유틸 매크로: 핀 세트/클리어
#define SET_BIT(PORT, PIN)   ((PORT) |= (1 << (PIN)))
#define CLEAR_BIT(PORT, PIN) ((PORT) &= ~(1 << (PIN)))

// 전역 변수
volatile uint16_t linecount = 0;   // vertical line 카운터
volatile uint8_t horiz_DE = 0;       // 수평 DE 플래그 (visible 구간이면 1)
volatile uint8_t vert_DE = 0;        // 수직 DE 플래그 (visible 구간이면 1)
// Timer1 COMPB 상태를 전역 변수로 정의 (매 수평라인마다 재설정)
volatile uint8_t compb_state = 0;

// Timer1 초기화: HSYNC는 OCR1A 하드웨어 출력, 비교 매치 B로 수평 DE 생성
void timer1_init(void)
{
    // 모드 14: ICR1을 TOP으로 사용, OCR1A로 HSYNC 펄스 생성
    TCCR1A = (1 << COM1A1) | (1 << COM1A0) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);
    
    ICR1 = H_TOTAL;         // 전체 수평 클럭 (264)
    OCR1A = H_PULSE - 1;      // HSYNC 펄스 폭: H_PULSE
    
    // 수평 DE 전환을 위해 비교 매치 B 인터럽트 사용
    OCR1B = H_VISIBLE_START - 1; // 초기 visible 시작 시점으로 설정
    TIMSK |= (1 << OCIE1B);   // Timer1 COMPB 인터럽트 활성화
    TIMSK |= (1 << TOIE1);    // Timer1 오버플로우 인터럽트 활성화 (vertical 타이밍)
}

// Timer1 COMPB ISR: visible 구간에서 horiz_DE 토글
ISR(TIMER1_COMPB_vect)
{
    if(compb_state == 0)
    {
        horiz_DE = 1;
        OCR1B = H_VISIBLE_END - 1;  // 다음 비교 매치 시 visible 종료 시점
        compb_state = 1;
    }
    else
    {
        horiz_DE = 0;
        OCR1B = H_VISIBLE_START - 1; // 다음 수평라인용 초기화
        compb_state = 0;
    }
    
    // 최종 DE = horiz_DE AND vert_DE
    if(horiz_DE && vert_DE)
        SET_BIT(PORTB, DE_PIN);
    else
        CLEAR_BIT(PORTB, DE_PIN);
}

// Timer1 OVF ISR: vertical 타이밍 처리 (VSYNC, vertical DE) 및 비교 매치 상태 초기화
ISR(TIMER1_OVF_vect)
{
    // VSYNC 생성: V_PULSE의 절반 동안 LOW
    if(linecount < (V_PULSE / 2))
        CLEAR_BIT(PORTB, VSYNC_PIN);
    else
        SET_BIT(PORTB, VSYNC_PIN);
    
    // vertical DE: 수직 visible 구간이면 HIGH
    if(linecount >= V_VISIBLE_START && linecount < V_VISIBLE_END)
        vert_DE = 1;
    else
        vert_DE = 0;
    
    linecount++;
    if(linecount >= V_TOTAL)
        linecount = 0;
    
    // 각 수평라인의 시작: horiz_DE와 COMPB 상태 재설정
    horiz_DE = 0;
    compb_state = 0;
    OCR1B = H_VISIBLE_START - 1;
}
  
void setup(void)
{
    DDRB = 0xFF;
    DDRE = 0xFF;
    
    // 초기 상태: VSYNC HIGH, DE LOW
    SET_BIT(PORTB, VSYNC_PIN);
    CLEAR_BIT(PORTB, DE_PIN);
    
    timer1_init();
    sei();
}
  
int main(void)
{
    setup();
    while(1)
    {
        // 모든 타이밍 처리는 ISR에서 수행
    }
}