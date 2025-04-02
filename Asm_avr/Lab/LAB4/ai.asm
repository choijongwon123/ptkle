.INCLUDE "m128def.inc"
    
.DSEG
.EQU F_CPU = 16000000
.EQU LCD_INST_PTR = 0x8000
.EQU LCD_DATA_PTR = 0x8002

.DEF CHE = R16
.DEF COL = R17
.DEF ROW = R18
.DEF COUNT = R20
.DEF HUNDREDS = R21
.DEF TENS = R22
.DEF ONES = R23

.MACRO LCD_CHARS
    LDI CHE, @0
    CALL LCD_CHAR
.ENDMACRO

.MACRO LCD_CHARS_DS
    MOV CHE, @0
    CALL LCD_CHAR
.ENDMACRO

.MACRO LCD_POS
    LDI COL, @0
    LDI ROW, @1
    CALL LCD_CURSOR
.ENDMACRO

.MACRO LCD_COMM
    STS LCD_INST_PTR, @0
    CALL D5MS
.ENDMACRO

.CSEG
.ORG 0x0000
RJMP INIT

INIT:
    LDI R16, LOW(RAMEND)
    OUT SPL, R16
    LDI R16, HIGH(RAMEND)
    OUT SPH, R16
    
    LDI R16, 0x80
    OUT MCUCR, R16
    
    ; Initialize counters
    LDI HUNDREDS, 0x30  ; ASCII '0'
    LDI TENS, 0x30      ; ASCII '0'
    LDI ONES, 0x30      ; ASCII '0'
    
    CALL LCD_INIT
    
LOOP:
    LCD_POS 5, 0        ; Position for hundreds
    LCD_CHARS '0'
    
    LCD_POS 6, 0        ; Position for tens
    LCD_CHARS '0'
    
    LCD_POS 7, 0        ; Position for ones
    LCD_CHARS_DS ONES
    
    INC ONES            ; Increment ones
    CPI ONES, 0x3A      ; Check if ones > '9'
    BRNE CONTINUE       ; If not, continue
    
    LDI ONES, 0x30      ; Reset ones to '0'
    
CONTINUE:
    RCALL D500MS        ; Delay
    RJMP LOOP           ; Repeat

LCD_INIT:
    PUSH R16
    LDI R16, 0x38       ; Function set
    LCD_COMM R16
    LDI R16, 0x0C       ; Display ON, cursor OFF
    LCD_COMM R16
    LDI R16, 0x06       ; Entry mode set
    LCD_COMM R16
    LDI R16, 0x01       ; Clear display
    LCD_COMM R16
    POP R16
    RET

LCD_CHAR:
    STS LCD_DATA_PTR, CHE
    CALL D50US
    RET

LCD_CURSOR:
    PUSH R16
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
    LCD_COMM R17
    
    POP R18
    POP R17
    POP R16
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