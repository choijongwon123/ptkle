#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRD = 0xF0;

    while (1)
    {
        PORTD = 0xFF;
        _delay_ms(100);
        PORTD = 0x00;
        _delay_ms(100);
    }
    return 0;
}