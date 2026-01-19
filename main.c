#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"  // Krävs för Pico W Wi-Fi
#include "lwip/apps/http_client.h" // För att skicka data
#include "lwip/altcp_tcp.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/apps/http_client.h"


// https://wokwi.com/projects/453504318387605505

#define LCD_ADDR 0x27

const char* THINGSPEAK_API_KEY = "IBT5YAMEJIWCRWV1";



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

void lcd_print(const char *s) {
    while (*s) lcd_send_byte(*s++, LCD_RS);
}

void http_client_callback(void *arg, httpc_result_t res, u32_t content_len, u32_t status_code, err_t err) {
        printf("HTTP Request klar. Status: %d, Resultat: %d\n", status_code, res);
}

void send_to_thingspeak(int value) {
    char url[128];
    // Skapa URL:en för ThingSpeak
    snprintf(url, sizeof(url), "/update?api_key=%s&field1=LektionIdag&field2=%d", THINGSPEAK_API_KEY, value, value);
    
    struct altcp_pcb *pcb = NULL; // Standard säkerhetsinställning
    
    printf("Anropar ThingSpeak: %s\n", url);
    
    // Detta anrop sköter DNS, anslutning och HTTP-headers åt dig
    err_t err = httpc_get_file_dns(
        "api.thingspeak.com", // Host
        80,                   // Port (HTTP)
        url,                  // Path + Query
        NULL,                 // Inställningar
        http_client_callback, // Callback
        NULL,                 // Argument
        NULL                  // Headers
    );

    if (err != ERR_OK) {
        printf("Kunde inte starta HTTP-anrop: %d\n", err);
    }

    // I en fullständig applikation används http_client_get här. 
    // För Wokwi-demo skriver vi ut resultatet på LCD:n
    lcd_send_byte(0x01, 0); // Clear
    sleep_ms(2);
    lcd_print("Sent to TS: ");
    lcd_send_byte(value + '0', LCD_RS);
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
    if (cyw43_arch_init()) {
        printf("Wi-Fi init misslyckades");
        return -1;
    }
    cyw43_arch_enable_sta_mode(); // Glöm inte denna rad!

    absolute_time_t next_update_time = get_absolute_time();



    // Setup I2C på GP4/GP5
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);

    lcd_init();



// Koppla upp mot Wokwi-GUEST
    printf("Ansluter till Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms("Wokwi-GUEST", "", CYW43_AUTH_OPEN, 30000)) {
        lcd_print("WiFi misslyck");
        printf("Anslutning misslyckades\n");
    } else {
        lcd_print("WiFi conn");
        printf("Ansluten!\n");
    }    




    lcd_print("WiFi Connected");

    while (1) {
        if (absolute_time_diff_us(get_absolute_time(), next_update_time) <= 0) {
            
            int random_val = (rand() % 3) + 1;
            send_to_thingspeak(random_val);

            // Sätt nästa körning om 30 sekunder
            next_update_time = delayed_by_us(get_absolute_time(), 30 * 1000000);
        }

        // Här körs tight_loop_contents() medan vi väntar
        tight_loop_contents();
    }
}