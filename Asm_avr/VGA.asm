.include "m128def.inc"

; All configuration constants have been inlined.
; F_CPU = 16000000
; VSYNC_PIN = 4, HSYNC_PIN = 3

; Timer1 configuration:
; TCCR1A = 0x33, TCCR1B = 0x1D, OCR1A = 259 (0x0103), OCR1B = 0, TIMSK = 0x04

; Timer3 configuration:
; TCCR3A = 0x32, TCCR3B = 0x1A, ICR3 = 63, OCR3A = 7, ETIMSK = 0x08

; Video timing:
; VERT_BACK_PORCH = 35, VLINE_LIMIT = 471  (471 = 0x01D7, low=215 (0xD7), high=1)

; Data segment
.dseg
    .org 0x0100
backPorchLinesToGo:
    .word 35               ; initial value = 35
vLine:
    .word 0                ; initial value = 0
image_line:
    .word 0                ; initial value = 0

; Reset and interrupt vectors
.cseg
.org 0x0000
    rjmp RESET             ; 2 cycles

; Timer/Counter1 Overflow Interrupt Vector 
.org 0x0020        ; Adjust vector address as needed
    rjmp TIMER1_OVF_ISR    ; 2 cycles

; Timer/Counter3 Overflow Interrupt Vector
.org 0x0028        ; Adjust vector address as needed
    rjmp TIMER3_OVF_ISR    ; 2 cycles

;===============================================================
; RESET vector
RESET:
    cli                    ; 1 cycle
    rcall setup            ; 3 cycles
    sei                    ; 1 cycle
MainLoop:
    sleep                  ; 1 cycle
    rcall doOneScanLine    ; 3 cycles
    rjmp MainLoop          ; 2 cycles

;===============================================================
; setup - configure ports, timers and sleep enable
setup:
    ; Set DDRB, DDRE, DDRC to 0xFF (all outputs)
    ldi   r16, 0xFF        ; 1 cycle
    out   DDRB, r16        ; 1 cycle
    out   DDRE, r16        ; 1 cycle
    out   DDRC, r16        ; 1 cycle

    ; Enable sleep: set SE bit (bit 5 = 0x20) in MCUCR
    in    r16, MCUCR       ; 1 cycle
    ori   r16, 0x20        ; 1 cycle
    out   MCUCR, r16       ; 1 cycle

    ; Initialize timers
    rcall timer1_init      ; 3 cycles
    rcall timer3_init      ; 3 cycles

    ret                    ; 4 cycles

;===============================================================
; timer1_init - configure Timer1 for v sync
timer1_init:
    ; TCCR1A = 0x33
    ldi   r16, 0x33        ; 1 cycle
    out   TCCR1A, r16      ; 1 cycle

    ; TCCR1B = 0x1D
    ldi   r16, 0x1D        ; 1 cycle
    out   TCCR1B, r16      ; 1 cycle

    ; Load OCR1A with 259 (0x0103)
    ldi   r16, 0x03        ; 1 cycle (low byte of 259)
    out   OCR1AL, r16      ; 1 cycle
    ldi   r16, 0x01        ; 1 cycle (high byte of 259)
    out   OCR1AH, r16      ; 1 cycle

    ; Set OCR1B = 0
    ldi   r16, 0           ; 1 cycle
    out   OCR1BL, r16      ; 1 cycle
    out   OCR1BH, r16      ; 1 cycle

    ; Enable Timer1 overflow interrupt: TIMSK = 0x04
    ldi   r16, 0x04        ; 1 cycle
    out   TIMSK, r16       ; 1 cycle

    ret                    ; 4 cycles

;===============================================================
; timer3_init - configure Timer3 for h sync
timer3_init:
    ; TCCR3A = 0x32
    ldi   r16, 0x32        ; 1 cycle
    out   TCCR3A, r16      ; 1 cycle

    ; TCCR3B = 0x1A
    ldi   r16, 0x1A        ; 1 cycle
    out   TCCR3B, r16      ; 1 cycle

    ; Load ICR3 with 63 (0x003F)
    ldi   r16, 63          ; 1 cycle (low byte of 63)
    out   ICR3L, r16       ; 1 cycle
    ldi   r16, 0           ; 1 cycle (high byte is 0)
    out   ICR3H, r16       ; 1 cycle

    ; Load OCR3A with 7
    ldi   r16, 7           ; 1 cycle
    out   OCR3AL, r16      ; 1 cycle
    ldi   r16, 0           ; 1 cycle (upper byte = 0)
    out   OCR3AH, r16      ; 1 cycle

    ; Enable Timer3 overflow interrupt: ETIMSK = 0x08
    ldi   r16, 0x08        ; 1 cycle
    out   ETIMSK, r16      ; 1 cycle

    ret                    ; 4 cycles

;===============================================================
; TIMER1 Overflow Interrupt Service Routine
TIMER1_OVF_ISR:
    push  r16              ; 2 cycles
    push  r17              ; 2 cycles

    ; Reset vLine = 0
    clr   r16              ; 1 cycle
    sts   vLine, r16       ; 2 cycles
    sts   vLine+1, r16     ; 2 cycles

    ; Reset image_line = 0
    sts   image_line, r16  ; 2 cycles
    sts   image_line+1, r16; 2 cycles

    ; Reset backPorchLinesToGo = 35
    ldi   r16, 35          ; 1 cycle (low byte of 35)
    sts   backPorchLinesToGo, r16  ; 2 cycles
    ldi   r16, 0           ; 1 cycle (high byte, since 35<256)
    sts   backPorchLinesToGo+1, r16; 2 cycles

    ; Set Timer3 counter: TCNT3 = 5
    ldi   r16, 5           ; 1 cycle
    out   TCNT3, r16       ; 1 cycle

    pop   r17              ; 2 cycles
    pop   r16              ; 2 cycles
    reti                   ; 4 cycles

;===============================================================
; TIMER3 Overflow Interrupt Service Routine
TIMER3_OVF_ISR:
    reti                   ; 4 cycles

;===============================================================
; doOneScanLine - generate one horizontal scan line
doOneScanLine:
    push  r18              ; 2 cycles
    push  r19              ; 2 cycles
    push  r20              ; 2 cycles
    push  r21              ; 2 cycles

    ; Load backPorchLinesToGo (16-bit)
    lds   r20, backPorchLinesToGo  ; 2 cycles
    lds   r21, backPorchLinesToGo+1; 2 cycles
    ; Check if backPorchLinesToGo > 0 
    tst   r20              ; 1 cycle
    brne  DoBackPorch      ; 1 or 2 cycles (if branch is taken)

    ; Else, check if vLine >= 471
    lds   r18, vLine       ; 2 cycles
    lds   r19, vLine+1     ; 2 cycles
    ; Inline constant 471 = 0x01D7 (low=215, high=1)
    ldi   r22, 215         ; 1 cycle
    ldi   r23, 1           ; 1 cycle
    ; Compare: vLine - 471 using temporary registers
    mov   r24, r18         ; 1 cycle
    mov   r25, r19         ; 1 cycle
    sub   r24, r22         ; 1 cycle
    sbc   r25, r23         ; 1 cycle
    brmi  DoVisible        ; 1 or 2 cycles if branch taken

    ; If vLine >= 471: clear PORTC and increment vLine
    ldi   r16, 0x00        ; 1 cycle
    out   PORTC, r16       ; 1 cycle
    lds   r18, vLine       ; 2 cycles
    lds   r19, vLine+1     ; 2 cycles
    inc   r18              ; 1 cycle
    brne  SkipVLineCarry   ; 1 or 2 cycles if branch taken
    inc   r19              ; 1 cycle (if branch not taken, executed after branch failure)
SkipVLineCarry:
    sts   vLine, r18       ; 2 cycles
    sts   vLine+1, r19     ; 2 cycles
    rjmp  EndDoOneScan     ; 2 cycles

DoBackPorch:
    ; backPorchLinesToGo > 0: clear PORTC and decrement counter
    ldi   r16, 0x00        ; 1 cycle
    out   PORTC, r16       ; 1 cycle
    lds   r20, backPorchLinesToGo  ; 2 cycles
    lds   r21, backPorchLinesToGo+1; 2 cycles
    subi  r20, 1           ; 1 cycle
    sbci  r21, 0           ; 1 cycle
    sts   backPorchLinesToGo, r20   ; 2 cycles
    sts   backPorchLinesToGo+1, r21 ; 2 cycles
    rjmp  EndDoOneScan     ; 2 cycles

DoVisible:
    ; Back Porch delay: 30 nops (30 cycles)
    .rept 30
        nop              ; 1 cycle each
    .endr
    ; Set PORTC to visible image data 0x3F
    ldi   r16, 0x3F        ; 1 cycle
    out   PORTC, r16       ; 1 cycle
    ; Visible area delay: 406 nops (406 cycles)
    .rept 406
        nop              ; 1 cycle each
    .endr
    ; Front Porch: clear PORTC
    ldi   r16, 0x00        ; 1 cycle
    out   PORTC, r16       ; 1 cycle
    ; Increment vLine (16-bit)
    lds   r18, vLine       ; 2 cycles
    lds   r19, vLine+1     ; 2 cycles
    inc   r18              ; 1 cycle
    brne  SkipVLineCarry2  ; 1 or 2 cycles if branch taken
    inc   r19              ; 1 cycle
SkipVLineCarry2:
    sts   vLine, r18       ; 2 cycles
    sts   vLine+1, r19     ; 2 cycles

EndDoOneScan:
    pop   r21              ; 2 cycles
    pop   r20              ; 2 cycles
    pop   r19              ; 2 cycles
    pop   r18              ; 2 cycles
    ret                    ; 4 cycles