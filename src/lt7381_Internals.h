#pragma once

/* include user config if available */
#if __has_include("lt7381_config.h")
#include "lt7381_config.h"
#else
#include "lt7381_config_default.h"
#endif

#include "lt7381_Registers.h"
#include <stdlib.h>

#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "esp_check.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "hal/ledc_types.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LT7381_TIMEOUT_US     (30 * 1000)        /* How long to wait for panel operations (in uS) */

#define PRINT_TAG             "LT7381_driver"

/**
 * @brief 
 */
typedef struct {
  esp_lcd_panel_t               esp_lcd_panel;
  esp_lcd_panel_io_handle_t     io_handle;
  gpio_num_t                    wait_gpio_num;   /* WAIT gpio pin */
  bool                          wait_level;      /* WAIT level */
  gpio_num_t                    reset_gpio_num;  /* RST gpio pin */
  bool                          reset_level;     /* RST level */
  uint8_t                       x_gap;
  uint8_t                       y_gap;
  uint16_t                      bytes_per_pixel;
  gpio_num_t                    pwm_gpio_num;    /* Backlight pwm pin */
  uint16_t                      backlight_duty;  /* backlight level */
} lt7381_panel_t;

/* ------------------------------------------------------------------ */
/* low-level helpers                                                  */
/* ------------------------------------------------------------------ */

static esp_err_t panel_lt7381_wait(esp_lcd_panel_t *panel);
static esp_err_t lt7381_cmd_write(lt7381_panel_t *lt, uint8_t cmd);
static esp_err_t lt7381_data_write(lt7381_panel_t *lt, uint8_t data);
static esp_err_t lt7381_data_read(lt7381_panel_t *lt, uint8_t *data);
static esp_err_t lt7381_status_read(lt7381_panel_t *lt, uint8_t *status);

/* ------------------------------------------------------------------ */
/* register manipulators                                              */
/* ------------------------------------------------------------------ */

static esp_err_t lt7381_system_wait_ready(esp_lcd_panel_t *panel);
static esp_err_t lt7381_pll_init(esp_lcd_panel_t *panel);
static esp_err_t lt7381_SDRAM_init(esp_lcd_panel_t *panel);
static esp_err_t lt7381_tft_panel_setting(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_bus_width_setting(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_image_data_format(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_memwrite_directions_setting(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_graphic_or_text_mode(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_memory_select(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_select_main_image_color_depth(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_PLCK_polarity(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_display_on_off(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_display_test_on_off(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_display_vertical_direction(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_display_color_sequence(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_PLCK_polarity(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_hsync_polarity(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_pde_polarity(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_pde_idle_state(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_pclk_idle_state(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_pd_idle_state(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_hsync_idle_state(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_vsync_idle_state(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_lcd_horizontal_width_vertical_height(esp_lcd_panel_t *panel, uint16_t WX, uint16_t HY);
static esp_err_t lt7381_lcd_horizontal_non_display(esp_lcd_panel_t *panel, uint16_t WX);
static esp_err_t lt7381_lcd_hsync_start_position(esp_lcd_panel_t *panel, uint16_t WX);
static esp_err_t lt7381_lcd_hsync_pulse_width(esp_lcd_panel_t *panel, uint16_t WX);
static esp_err_t lt7381_main_image_start_address(esp_lcd_panel_t *panel, uint32_t start);
static esp_err_t lt7381_main_image_width(esp_lcd_panel_t *panel, uint16_t width);
static esp_err_t lt7381_main_window_start_xy(esp_lcd_panel_t *panel, uint16_t x, uint16_t y);
static esp_err_t lt7381_lcd_vertical_non_display(esp_lcd_panel_t *panel, uint16_t HY);
static esp_err_t lt7381_lcd_vsync_start_position(esp_lcd_panel_t *panel, uint16_t HY);
static esp_err_t lt7381_lcd_vsync_pulse_width(esp_lcd_panel_t *panel, uint16_t HY);
static esp_err_t lt7381_memory_xy_mode(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_canvas_image_start_address(esp_lcd_panel_t *panel, uint32_t start);
static esp_err_t lt7381_canvas_image_width(esp_lcd_panel_t *panel, uint16_t width);
static esp_err_t lt7381_active_window_xy(esp_lcd_panel_t *panel, uint16_t x, uint16_t y);
static esp_err_t lt7381_active_window_wh(esp_lcd_panel_t *panel, uint16_t w, uint16_t h);
static esp_err_t lt7381_canvas_color_depth(esp_lcd_panel_t *panel, uint8_t setting);
static esp_err_t lt7381_cursor_xy(esp_lcd_panel_t *panel, uint16_t x, uint16_t y);

#ifdef __cplusplus
}
#endif