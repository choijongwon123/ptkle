#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRD = 0xF0;

    uint8_t LED = 0; // Bit nunber of led to be turned on

    while (1)
    {

        PORTD = 0x10 << LED; // Turn on LED PORTD[4:7]
        _delay_ms(200); // Delay 200ms
        PORTD = 0x00; // Turn off all LED
        LED++; // Increment LED index
        if (LED == 4) // If LED index is 4, reset it to 0
            LED = 0;
    }
    return 0;
}