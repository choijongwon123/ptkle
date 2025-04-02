#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRD = 0xF0;

    uint8_t toggle = 0;

    uint8_t LED = 0;

    while (1)
    {
        if (LED == 4) // If LED index is 4, reset it to 0
            LED = 0;
        if (PINA & 0x02) // If PA1 is high (switch is unpressed)
        {
            toggle = 0; // Reset toggle
        }
        else // If PA1 is low (switch is pressed)
        {
            if (toggle == 0) // If toggle is 0, toggle the LED
            {
                PORTD = 0x10 << LED;
                LED++;
                toggle = 1; // Set toggle to 1
            }
        }
        _delay_ms(10);
    }
    return 0;
}
