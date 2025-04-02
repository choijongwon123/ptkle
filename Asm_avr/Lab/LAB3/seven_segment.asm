.INCLUDE "m128def.inc"

.CSEG
.ORG 0x0000
    RJMP INIT

INIT:
    LDI R16, LOW(RAMEND)    // 스택 포인터 초기화 (하위 바이트)
    OUT SPL, R16
    LDI R16, HIGH(RAMEND)   // 스택 포인터 초기화 (상위 바이트)
    OUT SPH, R16

    LDI R16, 0x0F           // PORTF의 하위 4비트를 출력으로 설정
    OUT DDRF, R16
    LDI R16, 0xFF           // PORTD를 출력으로 설정
    OUT DDRD, R16

    LDI R16, 0xFB           // PORTE의 PE2를 입력으로 설정
    OUT PORTE, R16
    LDI R16, 1              // 7-세그먼트 디스플레이에 숫자 1 출력
    STS PORTF, R16

    LDI R16, 0xF7           // PORTE의 PE3를 입력으로 설정
    OUT PORTE, R16
    LDI R16, 2              // 7-세그먼트 디스플레이에 숫자 2 출력
    STS PORTF, R16

    LDI R16, 0xEF           // PORTE의 PE4를 입력으로 설정
    OUT PORTE, R16
    LDI R16, 3              // 7-세그먼트 디스플레이에 숫자 3 출력
    STS PORTF, R16

    LDI R16, 0xDF           // PORTE의 PE5를 입력으로 설정
    OUT PORTE, R16
    LDI R16, 4              // 7-세그먼트 디스플레이에 숫자 4 출력
    STS PORTF, R16

LOOP:
    LDI R16, 0xFE           // PORTE의 PE0를 입력으로 설정
    OUT PORTE, R16
    LDI R16, 1              // 7-세그먼트 디스플레이에 숫자 1 출력
    STS PORTF, R16

    LDI R16, 0xFD           // PORTE의 PE1를 입력으로 설정
    OUT PORTE, R16
    LDI R16, 2              // 7-세그먼트 디스플레이에 숫자 2 출력
    STS PORTF, R16

    LDI R16, 0xFB           // PORTE의 PE2를 입력으로 설정
    OUT PORTE, R16
    LDI R16, 3              // 7-세그먼트 디스플레이에 숫자 3 출력
    STS PORTF, R16

    LDI R16, 0xF7           // PORTE의 PE3를 입력으로 설정
    OUT PORTE, R16
    LDI R16, 4              // 7-세그먼트 디스플레이에 숫자 4 출력
    STS PORTF, R16

    LDI R16, 0xEF           // PORTE의 PE4를 입력으로 설정
    OUT PORTE, R16
    LDI R16, 5              // 7-세그먼트 디스플레이에 숫자 5 출력
    STS PORTF, R16

    LDI R16, 0xDF           // PORTE의 PE5를 입력으로 설정
    OUT PORTE, R16
    LDI R16, 6              // 7-세그먼트 디스플레이에 숫자 6 출력
    STS PORTF, R16

    RJMP LOOP               // 무한 루프