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
        if (PINA & 0x02) // If PA1 is high (switch is unpressed)
        {
            toggle = 0; // Reset toggle
        }
        else // If PA1 is low (switch is pressed)
        {
            if (toggle == 0) // If toggle is 0, toggle the LED
            {
                PORTD ^= 0xF0; // Toggle all LED
                toggle = 1;    // Set toggle to 1
            }
        }
        _delay_ms(10);
    }
    return 0;
}
