.include "m128def.inc"

.CSEG
.ORG 0x00

RESET :
    LDI R16, 0xFF
    OUT DDRD, R16

LOOP :
    LDI R16, 0xF0
    OUT PORTD, R16
    
    LDI R16 , 0x0F
    OUT PORTD, R16

    RJMP LOOP 
 