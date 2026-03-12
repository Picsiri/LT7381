#pragma once

/* Bus selection */
#define BUS_TYPE_I80    0u
#define BUS_TYPE_SPI    1u
#define BUS_TYPE_I2C    2u

/* Select interface here */
#define BUS_TYPE BUS_TYPE_I80

#define LT7381_BACKLIGHT_TYPE LT7381_BACKLIGHT_EXT_PWM

#if (BUS_TYPE == BUS_TYPE_I80)
#include "esp_lcd_io_i80.h"
#elif (BUS_TYPE == BUS_TYPE_SPI)
#include "esp_lcd_io_spi.h"
#elif (BUS_TYPE == BUS_TYPE_SPI)
#include "esp_lcd_io_iic.h"
#endif
/* ---------------- DISPLAY SIZE ---------------- */

#define LCD_WIDTH   800
#define LCD_HEIGHT  480


/* ---------------- CONTROL PINS ---------------- */

#define LCD_CS   10
#define LCD_DC   11
#define LCD_WR   12
#define LCD_RST  13


/* ---------------- I80 DATA PINS ---------------- */

#define LCD_D0   0
#define LCD_D1   1
#define LCD_D2   2
#define LCD_D3   3
#define LCD_D4   4
#define LCD_D5   5
#define LCD_D6   6
#define LCD_D7   7
#define LCD_D8   8
#define LCD_D9   9
#define LCD_D10  14
#define LCD_D11  15
#define LCD_D12  16
#define LCD_D13  17
#define LCD_D14  18
#define LCD_D15  19


/* ---------------- SPI PINS ---------------- */

#define LCD_SPI_MOSI 23
#define LCD_SPI_CLK  18
#define LCD_SPI_MISO -1


/* ---------------- I2C PINS ---------------- */

#define LCD_I2C_SDA 21
#define LCD_I2C_SCL 22
#define LCD_I2C_ADDR 0x3C