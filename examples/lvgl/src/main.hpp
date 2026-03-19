extern "C" {
    #include "lt7381.h"

    #if (LT7381_BUS_TYPE == LT7381_BUS_I80)
    #include "esp_lcd_io_i80.h"
    #elif (LT7381_BUS_TYPE == LT7381_BUS_SPI)
    #include "driver/spi_common.h"
    #elif (LT7381_BUS_TYPE == LT7381_BUS_IIC)
    #include "esp_lcd_io_iic.h"
    #endif
}

/* ---------------- CONTROL PINS ---------------- */




/* ---------------- I80 DATA PINS ---------------- */

#define LCD_DC          GPIO_NUM_NC
#define LCD_WR          GPIO_NUM_NC
#define LCD_D0          GPIO_NUM_NC
#define LCD_D1          GPIO_NUM_NC
#define LCD_D2          GPIO_NUM_NC
#define LCD_D3          GPIO_NUM_NC
#define LCD_D4          GPIO_NUM_NC
#define LCD_D5          GPIO_NUM_NC
#define LCD_D6          GPIO_NUM_NC
#define LCD_D7          GPIO_NUM_NC
#define LCD_D8          GPIO_NUM_NC
#define LCD_D9          GPIO_NUM_NC
#define LCD_D10         GPIO_NUM_NC
#define LCD_D11         GPIO_NUM_NC
#define LCD_D12         GPIO_NUM_NC
#define LCD_D13         GPIO_NUM_NC
#define LCD_D14         GPIO_NUM_NC
#define LCD_D15         GPIO_NUM_NC


/* ---------------- SPI PINS ---------------- */

#define LCD_CS          GPIO_NUM_10
#define LCD_SPI_MOSI    GPIO_NUM_11
#define LCD_SPI_CLK     GPIO_NUM_12
#define LCD_SPI_MISO    GPIO_NUM_13
#define LCD_RST         GPIO_NUM_16


/* ---------------- I2C PINS ---------------- */

#define LCD_I2C_SDA     GPIO_NUM_NC
#define LCD_I2C_SCL     GPIO_NUM_NC
#define LCD_I2C_ADDR    0x3C