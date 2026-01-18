#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"


// https://wokwi.com/projects/453504318387605505

#define LCD_ADDR 0x27



#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define LCD_ADDR 0x27

// Bit-definitioner för I2C-ryggsäcken (PCF8574)
#define LCD_RS        0x01 // Bit 0: Register Select (0=Kommando, 1=Data)
#define LCD_RW        0x02 // Bit 1: Read/Write (används sällan, oftast 0)
#define LCD_ENABLE    0x04 // Bit 2: Enable (Enter-knappen)
#define LCD_BACKLIGHT 0x08 // Bit 3: Bakgrundsbelysning (1=På)

// Funktion för att skicka en "nibble" (4 bitar) och pulsa Enable
void lcd_pulse_enable(uint8_t val) {
    uint8_t buf[2];
    buf[0] = val | LCD_ENABLE; // Sätt Enable hög
    buf[1] = val;              // Sätt Enable låg (skärmen läser nu)
    i2c_write_blocking(I2C_PORT, LCD_ADDR, buf, 2, false);
}

void lcd_send_byte(uint8_t val, int mode) {
    uint8_t high_nibble = mode | (val & 0xF0) | LCD_BACKLIGHT;
    uint8_t low_nibble  = mode | ((val << 4) & 0xF0) | LCD_BACKLIGHT;

    lcd_pulse_enable(high_nibble);
    lcd_pulse_enable(low_nibble);
}

void lcd_init() {
    sleep_ms(50);
    // Initieringssekvens enligt databladet för 4-bitars läge
    lcd_send_byte(0x03, 0); 
    lcd_send_byte(0x03, 0);
    lcd_send_byte(0x03, 0);
    lcd_send_byte(0x02, 0); // Tvinga till 4-bitars läge

    lcd_send_byte(0x28, 0); // 2 rader, 5x8 punkter
    lcd_send_byte(0x0C, 0); // Display på, markör av
    lcd_send_byte(0x01, 0); // Rensa skärmen
    sleep_ms(2);
}

int main() {
    stdio_init_all();

    // Setup I2C på GP4/GP5
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);

    lcd_init();

    // Skriv bokstaven 'P' (ASCII 0x50)
    lcd_send_byte(0x50, LCD_RS); 
    // Skriv bokstaven 'i' (ASCII 0x69)
    lcd_send_byte(0x69, LCD_RS);
    // Skriv bokstaven 'c' (ASCII 0x63)
    lcd_send_byte(0x63, LCD_RS);
    // Skriv bokstaven 'o' (ASCII 0x6F)
    lcd_send_byte(0x6F, LCD_RS);

    while (1) { tight_loop_contents(); }
}