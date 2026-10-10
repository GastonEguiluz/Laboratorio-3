#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

static const uint16_t notes[8] = {262, 294, 330, 349, 392, 440, 494, 523};

static void buttons_init(void)
{
    DDRD &= ~((1 << PD2) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7));
    PORTD |= (1 << PD2) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);
    DDRB &= ~((1 << PB0) | (1 << PB4) | (1 << PB5));
    PORTB |= (1 << PB0) | (1 << PB4) | (1 << PB5);
}

static uint8_t button_pressed(uint8_t index)
{
    switch (index) {
        case 0: return !(PIND & (1 << PD2));
        case 1: return !(PIND & (1 << PD4));
        case 2: return !(PIND & (1 << PD5));
        case 3: return !(PIND & (1 << PD6));
        case 4: return !(PIND & (1 << PD7));
        case 5: return !(PINB & (1 << PB0));
        case 6: return !(PINB & (1 << PB4));
        default: return !(PINB & (1 << PB5));
    }
}

static void tone_start(uint16_t frequency)
{
    DDRB |= (1 << PB1);
    TCCR1A = (1 << COM1A0);
    TCCR1B = (1 << WGM12) | (1 << CS11);
    OCR1A = (uint16_t)(F_CPU / (16UL * frequency) - 1);
}

static void tone_stop(void)
{
    TCCR1A = 0;
    TCCR1B = 0;
    PORTB &= ~(1 << PB1);
}

int main(void)
{
    buttons_init();

    while (1) {
        uint8_t found = 0;

        for (uint8_t key = 0; key < 8; key++) {
            if (button_pressed(key)) {
                tone_start(notes[key]);
                found = 1;
                break;
            }
        }

        if (!found) {
            tone_stop();
        }

        _delay_ms(10);
    }
}
