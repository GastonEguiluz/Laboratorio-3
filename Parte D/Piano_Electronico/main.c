#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#define UART_UBRR 103
#define LCD_ADDRESS 0x27

#define NOTE_A2 110
#define NOTE_G2 98
#define NOTE_F2 87
#define NOTE_BB2 117
#define NOTE_EB3 156
#define NOTE_BB4 466
#define NOTE_EB5 622
#define NOTE_BB5 932
#define NOTE_C3 131
#define NOTE_D3 147
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_G3 196
#define NOTE_A3 220
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_D5 587
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_G5 784
#define NOTE_A5 880
#define REST 0

typedef struct {
    uint16_t melody;
    uint16_t bass;
    uint16_t duration;
} SongEvent;

static const uint16_t piano_notes[8] = {
    NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4,
    NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5
};

static const char *piano_names[8] = {
    "DO", "RE", "MI", "FA", "SOL", "LA", "SI", "DO AGUDO"
};

static const SongEvent song_in_da_club[] = {
    {NOTE_D5, NOTE_D3, 180}, {REST, NOTE_D3, 70},
    {NOTE_D5, NOTE_D3, 180}, {NOTE_F5, NOTE_A2, 180},
    {NOTE_G5, NOTE_D3, 180}, {NOTE_A5, NOTE_D3, 360},
    {NOTE_G5, NOTE_A2, 180}, {NOTE_F5, NOTE_D3, 180},
    {NOTE_D5, NOTE_D3, 250}, {REST, NOTE_D3, 90},
    {NOTE_C5, NOTE_A2, 180}, {NOTE_D5, NOTE_D3, 180},
    {NOTE_F5, NOTE_D3, 180}, {NOTE_G5, NOTE_A2, 360},
    {NOTE_F5, NOTE_D3, 180}, {NOTE_D5, NOTE_D3, 180},
    {NOTE_D5, NOTE_D3, 180}, {REST, NOTE_D3, 70},
    {NOTE_D5, NOTE_D3, 180}, {NOTE_F5, NOTE_A2, 180},
    {NOTE_G5, NOTE_D3, 180}, {NOTE_A5, NOTE_D3, 360},
    {NOTE_C5, NOTE_A2, 180}, {NOTE_D5, NOTE_D3, 180},
    {NOTE_F5, NOTE_D3, 250}, {NOTE_E5, NOTE_D3, 180},
    {NOTE_D5, NOTE_A2, 180}, {NOTE_C5, NOTE_D3, 180},
    {NOTE_D5, NOTE_D3, 540}, {REST, REST, 180}
};

static const SongEvent song_loquita[] = {
    {NOTE_BB5, NOTE_G3, 170}, {NOTE_G4, NOTE_G3, 160},
    {NOTE_A5, NOTE_G3, 170}, {NOTE_G5, NOTE_G3, 500},
    {NOTE_F5, NOTE_G3, 170}, {NOTE_D5, NOTE_G3, 660},
    {NOTE_A4, NOTE_G3, 170}, {NOTE_BB4, NOTE_G3, 170},
    {NOTE_F5, NOTE_G3, 160}, {NOTE_A4, NOTE_G3, 170},
    {NOTE_G4, NOTE_G3, 170},
    {NOTE_BB5, NOTE_EB3, 170}, {NOTE_G4, NOTE_EB3, 160},
    {NOTE_A5, NOTE_EB3, 170}, {NOTE_G5, NOTE_EB3, 500},
    {NOTE_F5, NOTE_EB3, 170}, {NOTE_EB5, NOTE_EB3, 1330},
    {NOTE_EB5, NOTE_EB3, 170},
    {NOTE_EB5, NOTE_BB2, 170}, {NOTE_D5, NOTE_BB2, 160},
    {NOTE_BB4, NOTE_BB2, 170}, {NOTE_F4, NOTE_BB2, 330},
    {NOTE_F4, NOTE_BB2, 170}, {NOTE_BB4, NOTE_BB2, 170},
    {NOTE_D5, NOTE_BB2, 660}, {NOTE_EB5, NOTE_BB2, 170},
    {NOTE_EB5, NOTE_BB2, 170}, {NOTE_D5, NOTE_BB2, 160},
    {NOTE_BB4, NOTE_BB2, 340},
    {NOTE_BB5, NOTE_F2, 170}, {NOTE_A5, NOTE_F2, 160},
    {NOTE_F5, NOTE_F2, 170}, {NOTE_C5, NOTE_F2, 170},
    {NOTE_BB4, NOTE_F2, 160}, {NOTE_A4, NOTE_F2, 170},
    {NOTE_F4, NOTE_F2, 170}, {NOTE_C4, NOTE_F2, 160},
    {NOTE_A3, NOTE_F2, 1340}
};

static const SongEvent song_la_cucaracha[] = {
    {NOTE_C4, NOTE_C3, 220}, {NOTE_C4, NOTE_G3, 220}, {NOTE_C4, NOTE_C3, 220},
    {NOTE_F4, NOTE_F3, 420}, {NOTE_A4, NOTE_C3, 420},
    {NOTE_C4, NOTE_C3, 220}, {NOTE_C4, NOTE_G3, 220}, {NOTE_C4, NOTE_C3, 220},
    {NOTE_F4, NOTE_F3, 420}, {NOTE_A4, NOTE_C3, 420},
    {NOTE_F4, NOTE_F3, 220}, {NOTE_F4, NOTE_C3, 220}, {NOTE_E4, NOTE_G3, 220}, {NOTE_E4, NOTE_C3, 220},
    {NOTE_D4, NOTE_G3, 220}, {NOTE_D4, NOTE_C3, 220}, {NOTE_C4, NOTE_C3, 600},
    {NOTE_C4, NOTE_C3, 220}, {NOTE_C4, NOTE_G3, 220}, {NOTE_C4, NOTE_C3, 220},
    {NOTE_E4, NOTE_E3, 420}, {NOTE_G4, NOTE_C3, 420},
    {NOTE_C4, NOTE_C3, 220}, {NOTE_C4, NOTE_G3, 220}, {NOTE_C4, NOTE_C3, 220},
    {NOTE_E4, NOTE_E3, 420}, {NOTE_G4, NOTE_C3, 420},
    {NOTE_C5, NOTE_C3, 220}, {NOTE_D5, NOTE_G3, 220}, {NOTE_C5, NOTE_C3, 220}, {NOTE_A4, NOTE_G3, 220},
    {NOTE_G4, NOTE_C3, 220}, {NOTE_F4, NOTE_G3, 220}, {NOTE_E4, NOTE_C3, 220}, {NOTE_D4, NOTE_G3, 220},
    {NOTE_C4, NOTE_C3, 600}, {REST, REST, 180}
};

static uint8_t uart_prefix = 0;
static uint8_t requested_song = 0;
static uint8_t stop_song = 0;

static void uart_init(void) {
    UBRR0H = (uint8_t)(UART_UBRR >> 8);
    UBRR0L = (uint8_t)UART_UBRR;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static uint8_t uart_available(void) {
    return UCSR0A & (1 << RXC0);
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

static void lcd_two_lines(const char *first, const char *second) {
    lcd_command(0x01);
    lcd_command(0x80);
    lcd_text(first);
    lcd_command(0xC0);
    lcd_text(second);
}

static void melody_off(void) {
    TCCR1A = 0;
    TCCR1B = 0;
    PORTB &= (uint8_t)~(1 << PB1);
}

static void bass_off(void) {
    TCCR2A = 0;
    TCCR2B = 0;
    PORTB &= (uint8_t)~(1 << PB3);
}

static void melody_tone(uint16_t frequency) {
    uint32_t compare;
    if (frequency == 0) {
        melody_off();
        return;
    }
    compare = (F_CPU / (2UL * 8UL * frequency)) - 1UL;
    OCR1A = (uint16_t)compare;
    TCNT1 = 0;
    TCCR1A = (1 << COM1A0);
    TCCR1B = (1 << WGM12) | (1 << CS11);
}

static void bass_tone(uint16_t frequency) {
    uint32_t compare;
    if (frequency == 0) {
        bass_off();
        return;
    }
    compare = (F_CPU / (2UL * 1024UL * frequency)) - 1UL;
    if (compare > 255) {
        compare = 255;
    }
    OCR2A = (uint8_t)compare;
    TCNT2 = 0;
    TCCR2A = (1 << COM2A0) | (1 << WGM21);
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);
}

static void sound_off(void) {
    melody_off();
    bass_off();
}

static void process_uart(uint8_t value) {
    if (value >= 'a' && value <= 'z') {
        value = (uint8_t)(value - ('a' - 'A'));
    }
    if (uart_prefix) {
        uart_prefix = 0;
        if (value >= '1' && value <= '3') {
            requested_song = (uint8_t)(value - '0');
            stop_song = 1;
        }
        return;
    }
    if (value == 'C') {
        uart_prefix = 1;
    } else if (value == 'S') {
        stop_song = 1;
    } else if (value == '?') {
        uart_text("C1=IN DA CLUB C2=LOQUITA C3=LA CUCARACHA S=DETENER\r\n");
    }
}

static void service_uart(void) {
    while (uart_available()) {
        process_uart(UDR0);
    }
}

static uint8_t button_pressed(uint8_t index) {
    if (index == 0) {
        return !(PIND & (1 << PD2));
    }
    if (index == 1) {
        return !(PIND & (1 << PD4));
    }
    if (index == 2) {
        return !(PIND & (1 << PD5));
    }
    if (index == 3) {
        return !(PIND & (1 << PD6));
    }
    if (index == 4) {
        return !(PIND & (1 << PD7));
    }
    if (index == 5) {
        return !(PINB & (1 << PB0));
    }
    if (index == 6) {
        return !(PINB & (1 << PB4));
    }
    return !(PINB & (1 << PB5));
}

static void buttons_init(void) {
    DDRD &= (uint8_t)~((1 << PD2) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7));
    PORTD |= (1 << PD2) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);
    DDRB &= (uint8_t)~((1 << PB0) | (1 << PB4) | (1 << PB5));
    PORTB |= (1 << PB0) | (1 << PB4) | (1 << PB5);
}

static void piano_key(uint8_t index) {
    _delay_ms(20);
    if (!button_pressed(index)) {
        return;
    }
    melody_tone(piano_notes[index]);
    bass_off();
    lcd_two_lines("MODO PIANO", piano_names[index]);
    uart_text("NOTA: ");
    uart_text(piano_names[index]);
    uart_text("\r\n");
    while (button_pressed(index)) {
        service_uart();
        if (requested_song) {
            break;
        }
        _delay_ms(5);
    }
    melody_off();
    _delay_ms(20);
}

static uint8_t wait_song_time(uint16_t duration) {
    uint16_t elapsed = 0;
    while (elapsed < duration) {
        service_uart();
        if (stop_song || requested_song) {
            return 0;
        }
        _delay_ms(10);
        elapsed += 10;
    }
    return 1;
}

static void play_events(const SongEvent *events, uint8_t count) {
    uint8_t index;
    for (index = 0; index < count; index++) {
        melody_tone(events[index].melody);
        bass_tone(events[index].bass);
        if (!wait_song_time(events[index].duration)) {
            break;
        }
        sound_off();
        if (!wait_song_time(25)) {
            break;
        }
    }
    sound_off();
}

static void play_loquita(void) {
    uint8_t repeat;
    uint8_t index;
    uint16_t current_bass = 0;
    for (repeat = 0; repeat < 2; repeat++) {
        for (index = 0; index < sizeof(song_loquita) / sizeof(song_loquita[0]); index++) {
            melody_tone(song_loquita[index].melody);
            if (song_loquita[index].bass != current_bass) {
                current_bass = song_loquita[index].bass;
                bass_tone(current_bass);
            }
            if (!wait_song_time(song_loquita[index].duration - 20)) {
                sound_off();
                return;
            }
            melody_off();
            if (!wait_song_time(20)) {
                sound_off();
                return;
            }
        }
    }
    sound_off();
}

static void play_song(uint8_t song) {
    stop_song = 0;
    if (song == 1) {
        lcd_two_lines("REPRODUCIENDO", "IN DA CLUB");
        uart_text("REPRODUCIENDO C1: IN DA CLUB - INTRO\r\n");
        play_events(song_in_da_club, (uint8_t)(sizeof(song_in_da_club) / sizeof(song_in_da_club[0])));
    } else if (song == 2) {
        lcd_two_lines("LOQUITA", "LOS TURROS");
        uart_text("REPRODUCIENDO C2: LOQUITA - LOS TURROS\r\n");
        play_loquita();
    } else if (song == 3) {
        lcd_two_lines("REPRODUCIENDO", "LA CUCARACHA");
        uart_text("REPRODUCIENDO C3: LA CUCARACHA\r\n");
        play_events(song_la_cucaracha, (uint8_t)(sizeof(song_la_cucaracha) / sizeof(song_la_cucaracha[0])));
    }
    stop_song = 0;
}

int main(void) {
    uint8_t index;
    DDRB |= (1 << PB1) | (1 << PB3);
    PORTB &= (uint8_t)~((1 << PB1) | (1 << PB3));
    uart_init();
    twi_init();
    lcd_init();
    buttons_init();
    sound_off();
    lcd_two_lines("PIANO 8 NOTAS", "LISTO");
    uart_text("PIANO LISTO. C1=IN DA CLUB C2=LOQUITA C3=LA CUCARACHA S=DETENER\r\n");
    while (1) {
        service_uart();
        if (requested_song) {
            uint8_t song = requested_song;
            requested_song = 0;
            play_song(song);
            if (!requested_song) {
                lcd_two_lines("MODO PIANO", "LISTO");
            }
            continue;
        }
        for (index = 0; index < 8; index++) {
            if (button_pressed(index)) {
                piano_key(index);
                break;
            }
        }
    }
}
