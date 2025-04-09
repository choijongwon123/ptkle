.INCLUDE "m128def.inc"

.DSEG
.EQU F_CPU = 16000000

.DEF CH = R16
.DEF COL = R17
.DEF ROW = R18
.DEF ONES = R23      ; 1의 자리 카운터
.DEF TENS = R24      ; 10의 자리 카운터
.DEF HUNDREDS = R25  ; 100의 자리 카운터
.DEF TEMP = R22      ; 임시 저장용 레지스터

.MACRO LCD_CHAR
CALL D200US
MOV CH, @0
CALL LCD_DATA
.ENDMACRO

.MACRO LCD_POS
LDI COL, @0
LDI ROW, @1
CALL LCD_CURSOR
.ENDMACRO

.CSEG
.ORG 0x0000
RJMP LOOP

.ORG 0x0020
LOOP:
    LDI R16, LOW(RAMEND)
    OUT SPL, R16
    LDI R16, HIGH(RAMEND)    
    OUT SPH, R16

	LDI R16, 0xFF
	OUT DDRD, R16
	    
    LDI R16, 0xF0
    OUT DDRA, R16

    LDI R16, 0x03
    OUT DDRC, R16

    ; 카운터 초기화 (000)
    LDI ONES, 0
    LDI TENS, 0
    LDI HUNDREDS, 0

    CALL LCD_INIT
    CALL DISPLAY_COUNTER

MAIN_LOOP:
    CALL D100MS        ; 100ms 딜레이
    
    ; 카운터 증가
    INC ONES
    CPI ONES, 10
    BRNE DISPLAY_UPDATE    ; 1의 자리가 10이 아니면 바로 표시
    
    ; 1의 자리가 10이면
    CLR ONES
    INC TENS
    CPI TENS, 10
    BRNE DISPLAY_UPDATE    ; 10의 자리가 10이 아니면 바로 표시
    
    ; 10의 자리가 10이면
    CLR TENS
    INC HUNDREDS
    CPI HUNDREDS, 10
    BRNE DISPLAY_UPDATE    ; 100의 자리가 10이 아니면 바로 표시
    
    ; 100의 자리가 10이면 (999 이후)
    CLR HUNDREDS
    
DISPLAY_UPDATE:
    CALL DISPLAY_COUNTER
    RJMP MAIN_LOOP






; 카운터 값을 LCD에 표시하는 함수
DISPLAY_COUNTER:
    LCD_POS 13, 0       ; 커서를 첫 번째 줄 첫 번째 위치로 이동
    ; 100의 자리 표시
    MOV TEMP, HUNDREDS
    ORI TEMP, 0x30     ; ASCII 변환 (0x30 = '0')
    LCD_CHAR TEMP
 
    ; 10의 자리 표시
    MOV TEMP, TENS
    ORI TEMP, 0x30     ; ASCII 변환
    LCD_CHAR TEMP
   
    ; 1의 자리 표시
    MOV TEMP, ONES
    ORI TEMP, 0x30     ; ASCII 변환
    LCD_CHAR TEMP
    
    RET

LCD_INIT:
    LDI CH, 0x20
    CALL LCD_COMM
    CALL D5MS
    LDI CH, 0x28
    CALL LCD_COMM
    CALL D5MS
    LDI CH, 0x0C
    CALL LCD_COMM
    CALL D5MS
    LDI CH, 0x06
    CALL LCD_COMM
    CALL D5MS
    LDI CH, 0x01
    CALL LCD_COMM
    RET
    
LCD_COMM:
    PUSH R17
    LDI R17, PORTC
    
    RCALL FLIP_BITS
    
    OUT PORTA, R20
    
    ANDI R17, 0xFD
    OUT PORTC, R17
    ORI R17, 0x01
    OUT PORTC, R17
    CALL D1US
    ANDI R17, 0xFE
    OUT PORTC, R17
    CALL D20US
    
    LSL CH
    LSL CH
    LSL CH
    LSL CH
    RCALL FLIP_BITS
    
    OUT PORTA, R20
    
    ANDI R17, 0xFD
    OUT PORTC, R17
    ORI R17, 0x01
    OUT PORTC, R17
    CALL D1US
    ANDI R17, 0xFE
    OUT PORTC, R17
    CALL D5MS
    
    POP R17
    
    RET
    
LCD_DATA:
    PUSH R17
    PUSH R18
    
    LDI R17, PORTC
    
    RCALL FLIP_BITS
    
    OUT PORTA, R20
    
    ORI R17, 0x02
    OUT PORTC, R17
    ORI R17, 0x01
    OUT PORTC, R17
    CALL D1US
    ANDI R17, 0xFE
    OUT PORTC, R17
    CALL D20US
    
    LSL CH
    LSL CH
    LSL CH
    LSL CH
    RCALL FLIP_BITS
    
    OUT PORTA, R20
    
    ORI R17, 0x02
    OUT PORTC, R17
    ORI R17, 0x01
    OUT PORTC, R17
    CALL D1US
    ANDI R17, 0xFE
    OUT PORTC, R17
    CALL D50US
        
    POP R18
    POP R17
    RET
    
LCD_CURSOR:
    PUSH R17
    PUSH R18
    
    MOV R17, COL
    MOV R18, ROW
    
    LSL R18
    LSL R18
    LSL R18
    LSL R18
    LSL R18
    LSL R18
    
    ADD R17, R18
    ORI R17, 0x80
    MOV CH, R17
    CALL LCD_COMM
    
    POP R18
    POP R17

    RET
    
FLIP_BITS:
    PUSH R17
    PUSH R18
    PUSH R19

    LDI R19, 0x00
    
    LDI R17, 0x10
    MOV R18, CH
    LSR R18
    LSR R18
    LSR R18
    AND R18, R17
    MOV R19, R18
    
    LDI R17, 0x20
    MOV R18, CH
    LSR R18
    AND R18, R17
    OR R19, R18
    
    LDI R17, 0x40
    MOV R18, CH
    LSL R18
    AND R18, R17
    OR R19, R18
    
    LDI R17, 0x80
    MOV R18, CH
    LSL R18
    LSL R18
    LSL R18
    AND R18, R17
    OR R19, R18
    
    MOV R20, R19

    POP R19
    POP R18
    POP R17
    RET
    
    ; Delay Subroutines
D500MS:
    RCALL D100MS
    RCALL D200MS
    RCALL D200MS
    RET
    
D5MS:
    LDI R18, 5
    RJMP BASE1MS

D100MS:
    LDI R18, 100
    RJMP BASE1MS
    
D200MS:
    LDI R18, 200
BASE1MS:
    RCALL D200US
    RCALL D200US
    RCALL D200US
    RCALL D200US
    RCALL D200US
    DEC R18
    BRNE BASE1MS
    RET

D1US:
    LDI R19, 1
    RCALL BASE1US
    RET

D20US:
    LDI R19, 20
    RCALL BASE1US
    RET

D50US:
    LDI R19, 50
    RCALL BASE1US
    RET
D200US:
    LDI R19, 200
BASE1US:
    NOP
    PUSH R19
    POP R19
    PUSH R19
    POP R19
    PUSH R19
    POP R19
    DEC R19
    BRNE BASE1US
    RET
