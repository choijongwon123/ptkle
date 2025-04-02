	.include "m128def.inc"
	
	.DSEG
	.EQU F_CPU = 16000000
	.EQU MS = 100
	
	.DEF RegLEDs = r17
	.DEF RegCnt = r18
	.DEF xRegL = r24
	.DEF xRegH = r25
	
	
.CSEG
.ORG 0x0000
	LDI r16, LOW(RAMEND)
	OUT SPL, r16
	LDI r16, HIGH(RAMEND)
	OUT SPH, r16
	
	CLR r16
	
	LDI RegLEDs, 0xF0
	OUT DDRD, RegLEDs
	
LOOP:
	EOR r16, RegLEDs
	OUT PORTD, r16
	
	LDI RegCnt, MS
	RCALL DELAY_10MS
	
	RJMP LOOP
	
DELAY_10MS:                   ; 1 sec = 10 ms * 100
	LDI xRegL, LOW(F_CPU / 4000 * 10)
	LDI xRegH, HIGH(F_CPU / 4000 * 10)
	
DELAY_LOOP:
	SBIW xRegL, 1
	BRNE DELAY_LOOP
	
	DEC RegCnt
	BRNE DELAY_10MS
	
	RET
