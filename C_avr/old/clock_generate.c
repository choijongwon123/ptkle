#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 10000000UL // 외부 클럭 주파수: 10MHz
#define SET_BIT(PORT, PIN) PORT |= (1 << PIN)
#define CLEAR_BIT(PORT, PIN) PORT &= ~(1 << PIN)

#define LOW_CLOCKS 64    // 6.4us = 64 클럭
#define HIGH_CLOCKS 200  // 20us = 200 클럭



// 타이머1 초기화 (16비트 타이머)
void initTimer3() {
    // 출력 핀 설정
    DDRB |= (1 << PB7);
    CLEAR_BIT(PORTB, PB7);
    
    // 타이머1 레지스터 초기화
    TCCR3A = 0;
    TCCR3B = 0;
    TCNT3 = 0;
    
    // CTC 모드 설정 (WGM12=1, WGM13=0, WGM11=0, WGM10=0)
    TCCR3B |= (1 << WGM32);
    
    // 분주비 1 설정 (CS11=0, CS12=0, CS10=1)
    TCCR3B |= (1 << CS30);
    
    // 초기 비교 값 설정 (LOW 구간)
    OCR3A = LOW_CLOCKS - 1;
    
    // 타이머1 비교 A 인터럽트 활성화
    ETIMSK |= (1 << OCIE3A);
    
    // 전역 인터럽트 활성화
    sei();
}

// 타이머1 비교 A 인터럽트 서비스 루틴
ISR(TIMER3_COMPA_vect) {
    // 현재 타이머 값을 명시적으로 0으로 설정
    
    if (OCR3A == LOW_CLOCKS - 1) {  // 현재 LOW 상태
        // 출력을 HIGH로 변경
        SET_BIT(PORTB, PB7);
        
        // 다음 인터럽트를 위해 OCR1A 값 변경
        OCR3A = HIGH_CLOCKS - 1;
        
        // 상태 업데이트
    } else {  // 현재 HIGH 상태
        // 출력을 LOW로 변경
        CLEAR_BIT(PORTB, PB7);
        
        // 다음 인터럽트를 위해 OCR1A 값 변경
        OCR3A = LOW_CLOCKS - 1;
        
        // 상태 업데이트
    }
}

int main(void) {
    // PORTB를 출력으로 설정
    DDRB = 0xFF;
    
    // 타이머1 초기화
    initTimer3();
    
    // 무한 루프
    while (1) {
        // 모든 작업은 인터럽트에서 처리됨
    }
    
    return 0;
}