#pragma once

#include "LT7381_Registers.h"
#include <stdlib.h>

#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "esp_check.h"

#include "driver/gpio.h"
#include "hal/ledc_types.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LT7381_TIMEOUT_US     (30 * 1000)          /* How long to wait for panel operations (in uS) */

#define PRINT_TAG             "LT7381"

/**
 * @brief 
 */
typedef struct {
  esp_lcd_panel_t               esp_lcd_panel;
  esp_lcd_panel_io_handle_t     io_handle;
  gpio_num_t                    wait_gpio_num;   /* WAIT gpio pin (or GPIO_NUM_NC if unused) */
  gpio_num_t                    reset_gpio_num;  /* RST gpio pin (or GPIO_NUM_NC if unused) */
  bool                          reset_level;     /* RST level */
  uint8_t                       x_gap;
  uint8_t                       y_gap;
  uint16_t                      bits_per_pixel;  /* bpp used (should always be 16) */
  uint16_t                      lcd_width;       /* LCD width in pixels */
  uint16_t                      lcd_height;      /* LCD height in pixels */
  uint8_t                       chip_config_register;    /* CCR value */
  uint8_t                       display_config_register;  /* PDCR value */
  uint8_t                       macr;            /* Memory Access Control Register value */
  bool                          swap_axes;       /* whether to swap X/Y axis */
  gpio_num_t                    pwm_gpio_num;    /* Backlight control pwm pin (or GPIO_NUM_NC if unused) */
  uint16_t                      backlight_duty;  /* backlight level (16-bit integer) */
} lt7381_panel_t;


static void panel_lt7381_wait(esp_lcd_panel_t *panel);
static esp_err_t panel_lt7381_tx_param(esp_lcd_panel_t *panel, int lcd_cmd, uint8_t param);
static void panel_lt7381_set_window(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end);
static void panel_lt7381_set_cursor(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end);
static esp_err_t panel_lt7381_check(void);



#ifdef __cplusplus
}
#endif