#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define LCD_INST (*(volatile unsigned char *)0x8000)
#define LCD_DATA (*(volatile unsigned char *)0x8002)

void LCD_data(char ch)
{ // Write data
    LCD_DATA = ch;
    _delay_us(50);
}

void LCD_comm(char ch)
{ // Write instruction
    LCD_INST = ch;
    _delay_ms(5);
}

void LCD_CHAR(char ch)
{ // Display one character
    LCD_data(ch);
}

void LCD_STR(char *str)
{ // Display a string
    while (*str)
    {
        LCD_CHAR(*str++);
    }
}

void LCD_pos(char col, char row)
{
    LCD_comm(0x80 | (col + row * 0x40));
}

void LCD_clear(void)
{
    LCD_comm(0x01); // Clear display
    _delay_ms(2);
}

void LCD_init(void)
{
    LCD_comm(0x38); // Data line 8bit, 5x8 dot, 2 Line
    LCD_comm(0x0C); // Display ON/OFF
    LCD_comm(0x06); // Entry Mode Set
    LCD_clear();
}

int main(void)
{
    char str[20] = "LCD test.";
    MCUCR = 0x80; // Address and data bus
    LCD_init();
    LCD_pos(0, 0);
    sprintf(str, "Hello");
    LCD_STR(str);
    LCD_pos(0, 1);
    sprintf(str, "World");
    LCD_STR(str);
    return 0;
}