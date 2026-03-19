#include <Arduino.h>
#include "SPI.h"

#include "main.hpp"
#include "lt7381.h"
#include "image.h"

esp_lcd_panel_handle_t lcd_panel_handle = NULL;
esp_lcd_panel_io_handle_t io_handle = NULL;

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("Hello!");

#if (LT7381_BUS_TYPE == LT7381_BUS_I80)

  esp_lcd_i80_bus_handle_t i80_bus = NULL;

  esp_lcd_i80_bus_config_t bus_config = {
    .clk_src = LCD_CLK_SRC_DEFAULT,
    .dc_gpio_num = LCD_DC,
    .wr_gpio_num = LCD_WR,
    .data_gpio_nums = {
        LCD_D0, LCD_D1, LCD_D2, LCD_D3,
        LCD_D4, LCD_D5, LCD_D6, LCD_D7,
        LCD_D8, LCD_D9, LCD_D10, LCD_D11,
        LCD_D12, LCD_D13, LCD_D14, LCD_D15
    },
    .bus_width = 16,
    .max_transfer_bytes = LCD_WIDTH * 100 * sizeof(uint16_t),
    .psram_trans_align = 64,
    .sram_trans_align = 4
  };

  esp_lcd_new_i80_bus(&bus_config, &i80_bus);

  esp_lcd_panel_io_i80_config_t io_config = {
    .cs_gpio_num = LCD_CS,
    .pclk_hz = 12000000,
    .trans_queue_depth = 32,
    .dc_levels = {
        .dc_idle_level = 0,
        .dc_cmd_level = 0,
        .dc_dummy_level = 0,
        .dc_data_level = 1
    },
    .flags = {
        .swap_color_bytes = 0,
        .pclk_idle_low = 0
    },
    .lcd_cmd_bits = 16,
    .lcd_param_bits = 8
  };

  esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle);

#elif (LT7381_BUS_TYPE == LT7381_BUS_SPI)

  spi_bus_config_t buscfg = {
    .mosi_io_num = LCD_SPI_MOSI,
    .miso_io_num = LCD_SPI_MISO,
    .sclk_io_num = LCD_SPI_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = LT7381_IMAGE_STREAM_CHUNK_SIZE + 10,
  };

  ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

  esp_lcd_panel_io_spi_config_t io_config =
  {
    .cs_gpio_num = LCD_CS,
    .dc_gpio_num = LCD_DC,
    .spi_mode = SPI_MODE0,
    .pclk_hz = 40000000,
    .trans_queue_depth = 16,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8,
    .flags =
    {
      .dc_as_cmd_phase = 0,
      .dc_low_on_data = 0,
      .octal_mode = 0,
      .lsb_first = 0
    }
  };

  esp_lcd_new_panel_io_spi(
    (esp_lcd_spi_bus_handle_t)SPI2_HOST,
    &io_config,
    &io_handle
  );

#elif (LT7381_BUS_TYPE == LT7381_BUS_IIC)

  i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = LCD_I2C_SDA,
    .scl_io_num = LCD_I2C_SCL,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 400000
  };

  i2c_param_config(I2C_NUM_0, &conf);
  i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);

#endif

  const esp_lcd_panel_lt7381_config_t vendor_config =
  {
    .wait_gpio_num = GPIO_NUM_NC,
    .lcd_backlight_pwm = GPIO_NUM_15,

#if (LT7381_BUS_TYPE == LT7381_BUS_I80)
    .mcu_bit_interface = 16,
#endif
  };


  esp_lcd_panel_dev_config_t panel_config = {
    .reset_gpio_num = LCD_RST,
    .color_space = ESP_LCD_COLOR_SPACE_RGB,
    .bits_per_pixel = 16,
    .flags =
    {
      .reset_active_high = 0
    },
    .vendor_config = (void *)&vendor_config
  };

  esp_lcd_new_panel_lt7381(io_handle, &panel_config, &lcd_panel_handle);
  lcd_panel_handle->reset(lcd_panel_handle);
  lcd_panel_handle->init(lcd_panel_handle);
  Serial.println("Init run");
  lcd_panel_handle->disp_off(lcd_panel_handle, false);

  esp_lcd_panel_display_test(lcd_panel_handle, 1u);
  delay(1000);
  esp_lcd_panel_display_test(lcd_panel_handle, 0u);
  esp_lcd_panel_clear(lcd_panel_handle, 0x0000);
  delay(1000);

  esp_lcd_panel_draw_square_filled(lcd_panel_handle, 100, 100, 100, 100, 0xf800);
  delay(1000);
  esp_lcd_panel_draw_square_filled(lcd_panel_handle, 200, 200, 100, 100, 0x07e0);
  delay(1000);
  esp_lcd_panel_draw_square_filled(lcd_panel_handle, 300, 300, 100, 100, 0x001f);
  delay(1000);
}

void loop()
{
  lcd_panel_handle->draw_bitmap(lcd_panel_handle, 0, 0, LT7381_LCD_WIDTH, LT7381_LCD_HEIGHT, &test_image);
  delay(2500);
  
  esp_lcd_panel_clear(lcd_panel_handle, 0x0000);
  delay(1000);
}