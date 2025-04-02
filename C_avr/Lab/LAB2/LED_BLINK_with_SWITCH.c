#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRD = 0xF0;

    while (1)
    {
        if (PINA & 0x02) // If PA1 is high (switch is unpressed)
        {
            PORTD |= 0xF0; // Turn on all LED
        }
        else // If PA1 is low (switch is pressed)
        {
            PORTD &= 0x00; // Turn off all LED
        }
        _delay_ms(10);
    }
}