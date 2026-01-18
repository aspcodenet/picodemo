#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"



// https://wokwi.com/projects/453503809458041857
#define I2C_PORT i2c0
#define MPU_ADDR 0x68

// Registeradresser från databladet
#define REG_PWR_MGMT_1 0x6B
#define REG_ACCEL_XOUT_H 0x3B

void mpu6050_reset() {
    // För att väcka MPU6050: Skriv 0 till PWR_MGMT_1
    // Vi skickar en array: [Register, Värde]
    uint8_t buf[] = {REG_PWR_MGMT_1, 0x00};
    i2c_write_blocking(I2C_PORT, MPU_ADDR, buf, 2, false);
}

int main() {
    stdio_init_all();

    // Initiera I2C på 400kHz
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);

    mpu6050_reset();

    uint8_t buffer[2];
    uint8_t val = REG_ACCEL_XOUT_H;

    // const float GRAVITY_SCALE = 16384.0;

    while (true) {
        // 1. Tala om vilket register vi vill läsa (X-axel High)
        i2c_write_blocking(I2C_PORT, MPU_ADDR, &val, 1, true);
        
        // 2. Läs 2 bytes (High sen Low)
        i2c_read_blocking(I2C_PORT, MPU_ADDR, buffer, 2, false);

        // 3. BIT-PUSSLET
        // Vi tar High byte (8 bitar), skiftar den 8 steg åt vänster
        // och "OR:ar" in Low byte i de tomma nollorna.
        int16_t x_raw = (buffer[0] << 8) | buffer[1];


        // float x_g = (float)x_raw / GRAVITY_SCALE;

        printf("Raw X: %d\n", x_raw);
        sleep_ms(200);
    }
}