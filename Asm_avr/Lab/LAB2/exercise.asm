	.include "m128def.inc"
	
	.DEF temp = r16              ; Temporary register for I / O operations
	.DEF toggle = r17            ; Toggle flag to prevent multiple toggles per button press
	.DEF LED = r18               ; LED position counter (0 - 3)
	.DEF led_pattern = r19       ; Register to store LED pattern
	
	.ORG 0x0000
	RJMP INIT                    ; Jump to the initialization routine
	
	;===============================================================
INIT:
	LDI temp, 0xF0               ; Configure PORTD[7:4] as outputs
	OUT DDRD, temp				 
	CLR LED                      ; Initialize LED counter to 0
	CLR toggle                   ; Initialize toggle flag to 0
	
	;===============================================================
MAIN_LOOP:
	; Check if LED counter needs reset
	CPI LED, 4                   ; Compare LED counter with 4
	BREQ RESET_LED               ; If LED == 4, reset it
	
	; Read button state
	IN temp, PINA                ; Read Port A
	ANDI temp, 0x02              ; Mask PA1
	BRNE BUTTON_UP               ; If PA1 is high (unpressed), branch
	
	; Button is pressed
	CPI toggle, 0                ; Check if this is a new press
	BRNE MAIN_LOOP               ; If toggle != 0, ignore (debounce)
	
	; Process new button press
	LDI led_pattern, 0x10        ; Start with 0001 0000
	MOV temp, LED                ; Copy LED counter to temp
		
SHIFT_LOOP:
	CPI temp, 0                  ; Check if we need to shift
	BREQ DONE_SHIFT              ; If temp == 0, done shifting
	LSL led_pattern              ; Shift pattern left
	DEC temp                     ; Decrement counter
	RJMP SHIFT_LOOP              ; Continue shifting
	
DONE_SHIFT:
	OUT PORTD, led_pattern       ; Output the LED pattern
	INC LED                      ; Increment LED counter
	LDI toggle, 1                ; Set toggle flag
	RJMP MAIN_LOOP
	
BUTTON_UP:
	CLR toggle                   ; Reset toggle flag when button is up
	RJMP MAIN_LOOP
	
RESET_LED:
	CLR LED                      ; Reset LED counter to 0
	RJMP MAIN_LOOP
