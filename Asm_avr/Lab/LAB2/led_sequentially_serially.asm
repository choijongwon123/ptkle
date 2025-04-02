.include "m128def.inc"

.DEF temp = r16         
.DEF toggle = r17       
.DEF LED = r18
.DEF LOOP_TIME = r19

.ORG 0x0000
RJMP INIT              

INIT:
    LDI temp, 0xF0     
    OUT DDRD, temp     
    CLR LED
    CLR toggle   
    RJMP LOOP

LOOP:
    IN temp, PINA     

    CPI LED, 4 
    BREQ LED_RESET

    ANDI temp, 0x02    
    BREQ TOGGLE_RESET  
    
    CPI toggle, 0      
    BRNE LOOP          
	
    LDI temp, 0x10
    MOV LOOP_TIME, LED
	CPI LED, 0
	BREQ FOR_ZERO
    CALL SHIFT_LOOP

    OUT PORTD, temp    
    INC LED
    LDI toggle, 1      

    RJMP LOOP          
	
TOGGLE_RESET:
    CLR toggle         
    RJMP LOOP          

LED_RESET:
    CLR LED
    RJMP LOOP
	
SHIFT_LOOP: 
    LSL temp 
    DEC LOOP_TIME
    BRNE SHIFT_LOOP
    RET

FOR_ZERO:
	OUT PORTD, temp    
    INC LED
    LDI toggle, 1
	RJMP LOOP
	

       

    
