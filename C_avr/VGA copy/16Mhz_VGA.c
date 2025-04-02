#define F_CPU 16000000UL

void phase_adjust(void)
{
    __asm__ __volatile__(
        ".rept 416\n\t" // 416 * 0.0625us ≈ 26us delay
        "nop\n\t"
        ".endr\n\t");
}

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

#define SET_BIT(PORT, PIN) (PORT |= (1 << (PIN)))
#define CLEAR_BIT(PORT, PIN) (PORT &= ~(1 << (PIN)))

#define nop asm volatile("nop\n\t")

#define VSYNC_PIN PB4
#define HSYNC_PIN PB5

#define RED_PIN_0 PC0
#define RED_PIN_1 PC1
#define GREEN_PIN_0 PC2
#define GREEN_PIN_1 PC3
#define BLUE_PIN_0 PC4
#define BLUE_PIN_1 PC5

#define VERTICAL_LINES 525
#define VERTICAL_BACK_PORCH_LINES 35
#define VERTICAL_FRONT_PORCH_LINES 515

#define IMAGE_HEIGHT 30
#define IMAGE_DIWTH

#define VERTICAL_LINES 525

uint8_t image_color[64] = {
    0x00,
    0x01,
    0x15,
    0x02,
    0x16,
    0x2A,
    0x03,
    0x17,
    0x2B,
    0x3F,
    0x07,
    0x06,
    0x1B,
    0x0B,
    0x05,
    0x0A,
    0x1A,
    0x0F,
    0x1F,
    0x2F,
    0x0E,
    0x09,
    0x1E,
    0x0D,
    0x04,
    0x08,
    0x0C,
    0x19,
    0x1D,
    0x2E,
    0x1C,
    0x18,
    0x2D,
    0x2C,
    0x14,
    0x28,
    0x3C,
    0x29,
    0x3D,
    0x3E,
    0x38,
    0x24,
    0x39,
    0x34,
    0x10,
    0x20,
    0x30,
    0x25,
    0x35,
    0x3A,
    0x31,
    0x21,
    0x36,
    0x32,
    0x11,
    0x22,
    0x26,
    0x33,
    0x37,
    0x3B,
    0x23,
    0x12,
    0x27,
    0x13,
};
volatile uint8_t backPorchLinesToGo = VERTICAL_BACK_PORCH_LINES;

volatile int vLine = 0;
volatile uint8_t color = 0;

void timer1_init(void) // v_sync
{
    TCCR1A = (1 << COM1B1) | (1 << COM1B0) | (1 << WGM11) | (1 << WGM10);

    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS12) | (1 << CS10);

    OCR1A = 259;

    OCR1B = 0;

    TIMSK = (1 << TOIE1);
}

void timer3_init(void) // h_sync with 52 high and 8 low clocks
{
    // Non-inverting mode on OC3A (COM3A1=1, COM3A0=0)
    TCCR3A = (1 << COM3A0) | (1 << COM3A1) | (1 << WGM31);
    // WGM33 & WGM32 set for Fast PWM (mode 14), and prescaler=8 (CS31)
    TCCR3B = (1 << WGM33) | (1 << WGM32) | (1 << CS31);

    ICR3 = 63; // TOP value: counter counts 0 to 59 = 60 ticks (30µs)

    OCR3A = 7; // Output remains high for counts 0..51 (52 ticks high) and low for 52..59 (8 ticks low)

    ETIMSK = (1 << TOIE3);
    // Optionally, disable OCR3B usage if not needed.
    // Optionally enable interrupt if required: ETIMSK = (1 << TOIE3);
}

ISR(TIMER1_OVF_vect) // call for generate v sync
{
    vLine = 0;
    backPorchLinesToGo = VERTICAL_BACK_PORCH_LINES; // or your defined back porch value
    TCNT3 = 5;
}

ISR(TIMER3_OVF_vect) // call for generate h sync
{
}

void setup()
{

    cli();

    DDRB = 0xFF;
    DDRE = 0xFF;
    DDRC = 0xFF;

    MCUCR |= (1 << SE);

    timer1_init();
    timer3_init();

    sei();
}

void doOneScanLine(void)
{

    if (backPorchLinesToGo - 2)
    {
        PORTC = 0x00;
        backPorchLinesToGo--;
        return;
    }

    if (vLine >= 480 - 7 + 2)
    {
        PORTC = 0x00;
        // Front Porch delay - 45 NOPs (2.8125µs)
        vLine++;
        return;
    }

    // Back Porch delay before visible area
    __asm__ __volatile__(
        ".rept 30\n\t"
        "nop\n\t"
        ".endr\n\t");

    uint8_t currentLine = vLine / 10; // Every 10 lines, show new image line
    if (currentLine < 64)
    { // Check if within image bounds
        PORTC = image_color[currentLine];
    }
    else
    {
        PORTC = 0x00;
    }

    // Visible Area delay (25.4µs = 406 NOPs)
    __asm__ __volatile__(
        ".rept 406\n\t"
        "nop\n\t"
        ".endr\n\t");

    PORTC = 0x00;

    
    vLine++;
}

int main()
{
    setup();

    while (1)
    {
        sleep_mode();
        doOneScanLine();
    }

    return 0;
}
