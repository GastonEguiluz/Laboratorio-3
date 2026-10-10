#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#define UART_UBRR 103

static void uart_init(void)
{
    UBRR0H = (uint8_t)(UART_UBRR >> 8);
    UBRR0L = (uint8_t)UART_UBRR;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static void uart_put(uint8_t value)
{
    while (!(UCSR0A & (1 << UDRE0))) {
    }
    UDR0 = value;
}

static void uart_text(const char *text)
{
    while (*text) {
        uart_put((uint8_t)*text++);
    }
}

static void uart_uint(uint16_t value)
{
    char digits[5];
    uint8_t count = 0;

    if (value == 0) {
        uart_put('0');
        return;
    }

    while (value > 0) {
        digits[count++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (count > 0) {
        uart_put((uint8_t)digits[--count]);
    }
}

static void adc_init(void)
{
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

static uint16_t adc_read(uint8_t channel)
{
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC)) {
    }
    return ADC;
}

static uint16_t adc_average(uint8_t channel)
{
    uint32_t total = 0;

    for (uint8_t sample = 0; sample < 8; sample++) {
        total += adc_read(channel);
        _delay_ms(2);
    }

    return (uint16_t)(total / 8);
}

int main(void)
{
    uart_init();
    adc_init();

    while (1) {
        uint16_t sensor = adc_average(0);
        uart_text("ADC0=");
        uart_uint(sensor);
        uart_text("\r\n");
        _delay_ms(1000);
    }
}
