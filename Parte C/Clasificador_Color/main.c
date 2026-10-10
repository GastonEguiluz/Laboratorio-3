#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>

#define UART_UBRR 103
#define LCD_ADDRESS 0x27
#define WS_PIN PD6
#define WS_COUNT 8
#define SENSOR_CHANNEL 0
#define EEPROM_MAGIC 0xA5

typedef struct {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
} ColorSample;

static ColorSample references[4];
static const char *color_names[4] = {"ROJO", "VERDE", "AZUL", "AMARILLO"};
static const uint8_t servo_angles[4] = {20, 65, 110, 155};
static uint8_t ws_data[WS_COUNT][3];

static void uart_init(void) {
    UBRR0H = (uint8_t)(UART_UBRR >> 8);
    UBRR0L = (uint8_t)UART_UBRR;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static uint8_t uart_available(void) {
    return UCSR0A & (1 << RXC0);
}

static uint8_t uart_get(void) {
    while (!uart_available()) {
    }
    return UDR0;
}

static void uart_put(char value) {
    while (!(UCSR0A & (1 << UDRE0))) {
    }
    UDR0 = value;
}

static void uart_text(const char *text) {
    while (*text) {
        uart_put(*text++);
    }
}

static void uart_uint(uint16_t value) {
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
        uart_put(digits[--count]);
    }
}

static uint8_t twi_wait(void) {
    uint16_t timeout = 60000;
    while (!(TWCR & (1 << TWINT))) {
        if (--timeout == 0) {
            return 0;
        }
    }
    return 1;
}

static void twi_init(void) {
    DDRC &= (uint8_t)~((1 << PC4) | (1 << PC5));
    PORTC |= (1 << PC4) | (1 << PC5);
    TWSR = 0;
    TWBR = 72;
    TWCR = (1 << TWEN);
}

static uint8_t twi_start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    return twi_wait();
}

static uint8_t twi_write(uint8_t value) {
    TWDR = value;
    TWCR = (1 << TWINT) | (1 << TWEN);
    return twi_wait();
}

static void twi_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

static void lcd_expander(uint8_t value) {
    if (!twi_start()) {
        return;
    }
    if (!twi_write((uint8_t)(LCD_ADDRESS << 1))) {
        twi_stop();
        return;
    }
    twi_write(value | 0x08);
    twi_stop();
}

static void lcd_nibble(uint8_t value) {
    lcd_expander(value | 0x04);
    _delay_us(1);
    lcd_expander(value & (uint8_t)~0x04);
    _delay_us(50);
}

static void lcd_send(uint8_t value, uint8_t rs) {
    uint8_t control = rs ? 0x01 : 0x00;
    lcd_nibble((value & 0xF0) | control);
    lcd_nibble((uint8_t)(value << 4) | control);
}

static void lcd_command(uint8_t value) {
    lcd_send(value, 0);
    if (value == 0x01 || value == 0x02) {
        _delay_ms(2);
    }
}

static void lcd_char(char value) {
    lcd_send((uint8_t)value, 1);
}

static void lcd_text(const char *text) {
    while (*text) {
        lcd_char(*text++);
    }
}

static void lcd_uint(uint16_t value) {
    char digits[5];
    uint8_t count = 0;
    if (value == 0) {
        lcd_char('0');
        return;
    }
    while (value > 0) {
        digits[count++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (count > 0) {
        lcd_char(digits[--count]);
    }
}

static void lcd_init(void) {
    _delay_ms(50);
    lcd_nibble(0x30);
    _delay_ms(5);
    lcd_nibble(0x30);
    _delay_us(150);
    lcd_nibble(0x30);
    lcd_nibble(0x20);
    lcd_command(0x28);
    lcd_command(0x0C);
    lcd_command(0x06);
    lcd_command(0x01);
}

static void adc_init(void) {
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    DIDR0 = (1 << ADC0D);
}

static uint16_t adc_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC)) {
    }
    return ADC;
}

static uint16_t adc_average(void) {
    uint32_t total = 0;
    uint8_t index;
    adc_read(SENSOR_CHANNEL);
    for (index = 0; index < 16; index++) {
        total += adc_read(SENSOR_CHANNEL);
    }
    return (uint16_t)(total / 16);
}

static void ws_send_bit(uint8_t bit_value) {
    if (bit_value) {
        PORTD |= (1 << WS_PIN);
        asm volatile("nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t");
        PORTD &= (uint8_t)~(1 << WS_PIN);
        asm volatile("nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t");
    } else {
        PORTD |= (1 << WS_PIN);
        asm volatile("nop\n\t" "nop\n\t");
        PORTD &= (uint8_t)~(1 << WS_PIN);
        asm volatile("nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t");
    }
}

static void ws_send_byte(uint8_t value) {
    uint8_t mask;
    for (mask = 0x80; mask != 0; mask >>= 1) {
        ws_send_bit(value & mask);
    }
}

static void ws_show(void) {
    uint8_t saved_sreg = SREG;
    uint8_t index;
    cli();
    for (index = 0; index < WS_COUNT; index++) {
        ws_send_byte(ws_data[index][1]);
        ws_send_byte(ws_data[index][0]);
        ws_send_byte(ws_data[index][2]);
    }
    SREG = saved_sreg;
    _delay_us(80);
}

static void ws_color(uint8_t red, uint8_t green, uint8_t blue) {
    uint8_t index;
    for (index = 0; index < WS_COUNT; index++) {
        ws_data[index][0] = red;
        ws_data[index][1] = green;
        ws_data[index][2] = blue;
    }
    ws_show();
}

static ColorSample measure_color(void) {
    ColorSample sample;
    ws_color(255, 0, 0);
    _delay_ms(40);
    sample.red = adc_average();
    ws_color(0, 255, 0);
    _delay_ms(40);
    sample.green = adc_average();
    ws_color(0, 0, 255);
    _delay_ms(40);
    sample.blue = adc_average();
    ws_color(0, 0, 0);
    return sample;
}

static void servo_init(void) {
    DDRB |= (1 << PB1);
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
    ICR1 = 39999;
    OCR1A = 3000;
}

static void servo_angle(uint8_t angle) {
    OCR1A = (uint16_t)(1000UL + ((uint32_t)angle * 4000UL) / 180UL);
}

static uint8_t eeprom_read_byte_raw(uint16_t address) {
    while (EECR & (1 << EEPE)) {
    }
    EEAR = address;
    EECR |= (1 << EERE);
    return EEDR;
}

static void eeprom_write_byte_raw(uint16_t address, uint8_t value) {
    uint8_t saved_sreg;
    while (EECR & (1 << EEPE)) {
    }
    if (eeprom_read_byte_raw(address) == value) {
        return;
    }
    EEAR = address;
    EEDR = value;
    saved_sreg = SREG;
    cli();
    EECR |= (1 << EEMPE);
    EECR |= (1 << EEPE);
    SREG = saved_sreg;
}

static uint16_t eeprom_read_word_raw(uint16_t address) {
    uint16_t value = eeprom_read_byte_raw(address);
    value |= (uint16_t)eeprom_read_byte_raw(address + 1) << 8;
    return value;
}

static void eeprom_write_word_raw(uint16_t address, uint16_t value) {
    eeprom_write_byte_raw(address, (uint8_t)value);
    eeprom_write_byte_raw(address + 1, (uint8_t)(value >> 8));
}

static void references_save(void) {
    uint8_t index;
    uint16_t address = 1;
    for (index = 0; index < 4; index++) {
        eeprom_write_word_raw(address, references[index].red);
        address += 2;
        eeprom_write_word_raw(address, references[index].green);
        address += 2;
        eeprom_write_word_raw(address, references[index].blue);
        address += 2;
    }
    eeprom_write_byte_raw(0, EEPROM_MAGIC);
}

static void references_defaults(void) {
    references[0].red = 760;
    references[0].green = 260;
    references[0].blue = 210;
    references[1].red = 260;
    references[1].green = 760;
    references[1].blue = 250;
    references[2].red = 220;
    references[2].green = 280;
    references[2].blue = 760;
    references[3].red = 710;
    references[3].green = 690;
    references[3].blue = 230;
}

static void references_load(void) {
    uint8_t index;
    uint16_t address = 1;
    if (eeprom_read_byte_raw(0) != EEPROM_MAGIC) {
        references_defaults();
        references_save();
        return;
    }
    for (index = 0; index < 4; index++) {
        references[index].red = eeprom_read_word_raw(address);
        address += 2;
        references[index].green = eeprom_read_word_raw(address);
        address += 2;
        references[index].blue = eeprom_read_word_raw(address);
        address += 2;
    }
}

static uint16_t absolute_difference(uint16_t first, uint16_t second) {
    return first > second ? first - second : second - first;
}

static uint16_t color_distance(ColorSample sample, ColorSample reference) {
    return absolute_difference(sample.red, reference.red) +
           absolute_difference(sample.green, reference.green) +
           absolute_difference(sample.blue, reference.blue);
}

static uint8_t classify(ColorSample sample, uint16_t *best_difference) {
    uint8_t index;
    uint8_t best = 0;
    uint16_t difference = color_distance(sample, references[0]);
    for (index = 1; index < 4; index++) {
        uint16_t current = color_distance(sample, references[index]);
        if (current < difference) {
            difference = current;
            best = index;
        }
    }
    *best_difference = difference;
    return best;
}

static void show_detected_color(uint8_t color) {
    if (color == 0) {
        ws_color(255, 0, 0);
    } else if (color == 1) {
        ws_color(0, 255, 0);
    } else if (color == 2) {
        ws_color(0, 0, 255);
    } else {
        ws_color(255, 160, 0);
    }
}

static void lcd_result(ColorSample sample, uint8_t color, uint16_t difference) {
    lcd_command(0x01);
    lcd_command(0x80);
    lcd_text(color_names[color]);
    lcd_text(" D:");
    lcd_uint(difference);
    lcd_command(0xC0);
    lcd_char('R');
    lcd_uint(sample.red);
    lcd_char('G');
    lcd_uint(sample.green);
    lcd_char('B');
    lcd_uint(sample.blue);
}

static void uart_result(ColorSample sample, uint8_t color, uint16_t difference) {
    uart_text("R=");
    uart_uint(sample.red);
    uart_text(" G=");
    uart_uint(sample.green);
    uart_text(" B=");
    uart_uint(sample.blue);
    uart_text(" COLOR=");
    uart_text(color_names[color]);
    uart_text(" PATRON=");
    uart_uint(references[color].red);
    uart_put(',');
    uart_uint(references[color].green);
    uart_put(',');
    uart_uint(references[color].blue);
    uart_text(" DIF=");
    uart_uint(difference);
    uart_text(" ANGULO=");
    uart_uint(servo_angles[color]);
    uart_text("\r\n");
}

static void perform_measurement(void) {
    ColorSample sample = measure_color();
    uint16_t difference;
    uint8_t color = classify(sample, &difference);
    servo_angle(servo_angles[color]);
    show_detected_color(color);
    lcd_result(sample, color, difference);
    uart_result(sample, color, difference);
}

static void calibrate(uint8_t color) {
    lcd_command(0x01);
    lcd_text("CALIBRANDO");
    lcd_command(0xC0);
    lcd_text(color_names[color]);
    references[color] = measure_color();
    references_save();
    uart_text("CALIBRADO ");
    uart_text(color_names[color]);
    uart_text(" R=");
    uart_uint(references[color].red);
    uart_text(" G=");
    uart_uint(references[color].green);
    uart_text(" B=");
    uart_uint(references[color].blue);
    uart_text("\r\n");
}

static void uart_help(void) {
    uart_text("M=MEDIR R=CAL ROJO G=CAL VERDE B=CAL AZUL Y=CAL AMARILLO ?=AYUDA\r\n");
}

static void process_command(uint8_t command) {
    if (command >= 'a' && command <= 'z') {
        command = (uint8_t)(command - ('a' - 'A'));
    }
    if (command == 'R') {
        calibrate(0);
    } else if (command == 'G') {
        calibrate(1);
    } else if (command == 'B') {
        calibrate(2);
    } else if (command == 'Y') {
        calibrate(3);
    } else if (command == 'M') {
        perform_measurement();
    } else if (command == '?') {
        uart_help();
    }
}

int main(void) {
    uint8_t wait_index;
    DDRD |= (1 << WS_PIN);
    PORTD &= (uint8_t)~(1 << WS_PIN);
    uart_init();
    twi_init();
    lcd_init();
    adc_init();
    servo_init();
    references_load();
    ws_color(0, 0, 0);
    lcd_text("CLASIFICADOR");
    lcd_command(0xC0);
    lcd_text("INICIANDO");
    uart_help();
    _delay_ms(800);
    while (1) {
        perform_measurement();
        for (wait_index = 0; wait_index < 100; wait_index++) {
            if (uart_available()) {
                process_command(uart_get());
            }
            _delay_ms(10);
        }
    }
}
