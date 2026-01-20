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


#define BIT_SET(var, pos)   ((var) |= (1U << (pos)))
#define BIT_CLEAR(var, pos) ((var) &= ~(1U << (pos)))



// gloval  variabel - REGISTER
byte dReg;

// siffran 32 binört = 00100000

// dReg = 00000010;
// d
BIT_SET(dReg,5);
BIT_CLEAR(dReg,3);

//dReg = dReg | ( 1         << 5); // sätter bit 5 till 1
// 00000010
// 00100000
// 00100010

// 00001111
// Programmera så att bit 2 släcks på dReg
dReg = dReg & ~( 1 << 2); // släcker bit 2
//             00000001
//             00000100
//  ~            11111011




            // 00000001  
             // 1 flyttas 5 steg till vänster
                // 1 << 5 = 00100000

dRe = 32;


// Funktion för att skicka en "nibble" (4 bitar) och pulsa Enable
void lcd_pulse_enable(uint8_t val) {
    uint8_t buf[2];
    buf[0] = val | LCD_ENABLE; // Sätt Enable hög
    buf[1] = val;              // Sätt Enable låg (skärmen läser nu)
    i2c_write_blocking(I2C_PORT, LCD_ADDR, buf, 2, false);
}


// lcd_send_byte
//Här är ett exempel på hur logiken i funktionen brukar se ut bakom kulisserna:
// Dela upp byten: Den tar de översta 4 bitarna (High Nibble) och skickar dem först, sedan de nedersta 4 bitarna (Low Nibble).
// Sätt RS-pinnen: Om du skickar ett kommando (som i din initiering) sätts RS till 0. Om du skickar text sätts RS till 1.
// Klockpulsen (Enable): För varje nibble måste vi "tala om" för displayen att läsa av databussen. Detta görs genom att dra Enable (E) pinnen hög, vänta en kort stund, och dra den låg igen.
// void lcd_send_byte(uint8_t val, int rs) {
//     // Sätt RS-pinnen (0 för instruktion, 1 för data)
//     gpio_put(LCD_RS_PIN, rs);

//     // 1. Skicka hög nibble (bit 4-7)
//     lcd_send_nibble(val >> 4);
    
//     // 2. Skicka låg nibble (bit 0-3)
//     lcd_send_nibble(val & 0x0F);
    
//     // En kort paus så att LCD:n hinner bearbeta (ca 100us)
//     sleep_us(100);
// }
// Det är här den faktiska "handskakningen" sker enligt databladet:
// Lägg ut 4 bitar på datalinjerna (D4, D5, D6, D7).
// Enable = 1 (Hög).
// Vänta (enligt databladet krävs en Pulse Width på minst 450ns).
// Enable = 0 (Låg). Vid denna "fallande flank" läser LCD:n av vad som 
// finns på pinnarna.
// void lcd_pulse_enable(uint8_t val) {
//     // Vi lägger till Enable-biten (t.ex. mask 0x04) till värdet
//     // Här skickar vi data till en I2C-expander eller direkt till GPIO
//     i2c_write_byte(val | LCD_ENABLE_BIT); 
//     sleep_us(1); // Enligt databladet: min 450ns puls
    
//     i2c_write_byte(val & ~LCD_ENABLE_BIT);
//     sleep_us(50); // Vänta på att kommandot processas
// }

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
    // https://api.thingspeak.com/update?api_key=IBT5YAMEJIWCRWV1&field2=2
    snprintf(url, sizeof(url), "/update?api_key=%s&field1=LektionIdag&field2=%d", 
            THINGSPEAK_API_KEY, value);
    
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
    sleep_ms(50); // Wait for more than 15 ms after Vcc rises to 4.5V

    // Initieringssekvens enligt databladet för 4-bitars läge
    // Initializing by Instruction

    // Varför 0x03? I databladets flödesschema för 
    // "Initialization by Instruction" skickas bitarna DB5=1, DB4=1 
    // (vilket är 0011 i binär form, eller 0x03 i den höga nibbeln). 
    // Detta sätter DL=1 (8-bitars interface).

    lcd_send_byte(0x03, 0); // Försök 1: Sätt till 8-bitars läge
    lcd_send_byte(0x03, 0); // Försök 2: Upprepa (om den var ur synk)
    lcd_send_byte(0x03, 0); // Försök 3: Nu är vi garanterat i 8-bitars
    lcd_send_byte(0x02, 0); // Tvinga till 4-bitars läge
    //Varför 0x02? Genom att skicka 0010 binärt sätts DL=0, 
    // vilket aktiverar 4-bitars läge. Från och med nu 
    // förväntar sig LCD:n att varje kommando skickas i 
    // två delar (två "nibbles").


    // 0x28 i binär form är 0010 1000.
    // Enligt databladet för Function Set:
    // DL = 0: 4-bitars läge (redan satt, men bekräftas här).
    // N = 1: 2 raders display (bit 3).
    // F = 0: 5x8 punkters teckenstorlek (bit 2).

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