#include "pressure_functions.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>

#define INA219_INTERCEPT -23.76
#define INA219_GRADIENT 5.75

#define I2C_PORT i2c0 
#define PAC1934_ADDR 0x20

#define SDA_PIN 4
#define SCL_PIN 5

bool pac1934_init(void){
    i2c_init(I2C_PORT, 400000);

    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(10);   // allow PAC1934 power-up time

    uint8_t test;

    int result = i2c_write_blocking(I2C_PORT, PAC1934_ADDR, NULL, 0, false);

    if (result < 0)
    {
      printf("PAC1934 init FAILED (no ACK)\n");
      return 0;
    } 
    printf("PAC1934 detected successfully!\n");
    return 1;
}

bool pac1934_refresh(void){
    // wait at least 2ms after calling this so it refreshes
    uint8_t cmd = 0x00;
    int result = i2c_write_blocking(I2C_PORT, PAC1934_ADDR, &cmd, 1, false);
    if (result < 0) {
      printf("PAC1934 refresh failed\n");
      return 0;
    }
    else {
      printf("PAC1934 refresh successful!\n");
      return 1;
    }
}

