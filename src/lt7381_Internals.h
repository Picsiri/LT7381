#ifndef __LT7381_INTERNALS_H__
#define __LT7381_INTERNALS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* include user config if available */
#if __has_include("lt7381_config.h")
#include "lt7381_config.h"
#else
#include "lt7381_config_default.h"
#endif

#include "lt7381_Registers.h"
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "esp_check.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "hal/ledc_types.h"


#define LT7381_TIMEOUT_US     (30 * 1000)        /* How long to wait for panel operations (in uS) */

#define PRINT_TAG             "LT7381_driver"

#define Black   0x0000
#define White   0xffff
#define Red     0xf800
#define Green   0x07e0
#define Blue    0x001f
#define Yellow  Red|Green
#define Cyan    Green|Blue
#define Purple  Red|Blue

/**
 * @brief 
 */
typedef struct {
  esp_lcd_panel_t               esp_lcd_panel;
  esp_lcd_panel_io_handle_t     io_handle;
  gpio_num_t                    wait_gpio_num;   /* WAIT gpio pin */
  uint8_t                       wait_level;      /* WAIT level */
  gpio_num_t                    reset_gpio_num;  /* RST gpio pin */
  uint8_t                       reset_level;     /* RST level */
  uint8_t                       x_gap;
  uint8_t                       y_gap;
  uint16_t                      bytes_per_pixel;
  gpio_num_t                    pwm_gpio_num;    /* Backlight pwm pin */
  uint16_t                      backlight_duty;  /* backlight level */
} lt7381_panel_t;

/* ------------------------------------------------------------------ */
/* low-level helpers                                                  */
/* ------------------------------------------------------------------ */

esp_err_t lt7381_wait(lt7381_panel_t *lt);
esp_err_t lt7381_cmd_write(lt7381_panel_t *lt, uint8_t cmd);
esp_err_t lt7381_data_write(lt7381_panel_t *lt, uint8_t data);
esp_err_t lt7381_batch_write(lt7381_panel_t *lt, uint8_t *batch, uint8_t size);
esp_err_t lt7381_data_read(lt7381_panel_t *lt, uint8_t *data);
esp_err_t lt7381_status_read(lt7381_panel_t *lt, uint8_t *status);

/* ------------------------------------------------------------------ */
/* register manipulators                                              */
/* ------------------------------------------------------------------ */

esp_err_t lt7381_system_wait_ready(lt7381_panel_t *lt);
esp_err_t lt7381_pll_init(lt7381_panel_t *lt);
esp_err_t lt7381_SDRAM_init(lt7381_panel_t *lt);
esp_err_t lt7381_tft_panel_setting(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_bus_width_setting(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_image_data_format(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_memwrite_directions_setting(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_graphic_or_text_mode(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_memory_select(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_draw_pixel(lt7381_panel_t *lt, uint16_t color);
esp_err_t lt7381_draw_picture(lt7381_panel_t *lt, const uint8_t* color, uint32_t len);
esp_err_t lt7381_select_main_image_color_depth(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_PLCK_polarity(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_display_on_off(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_display_test_on_off(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_display_vertical_direction(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_display_color_sequence(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_PLCK_polarity(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_hsync_polarity(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_pde_polarity(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_pde_idle_state(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_pclk_idle_state(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_pd_idle_state(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_hsync_idle_state(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_vsync_idle_state(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_lcd_horizontal_width_vertical_height(lt7381_panel_t *lt, uint16_t WX, uint16_t HY);
esp_err_t lt7381_lcd_horizontal_non_display(lt7381_panel_t *lt, uint16_t WX);
esp_err_t lt7381_lcd_hsync_start_position(lt7381_panel_t *lt, uint16_t WX);
esp_err_t lt7381_lcd_hsync_pulse_width(lt7381_panel_t *lt, uint16_t WX);
esp_err_t lt7381_main_image_start_address(lt7381_panel_t *lt, uint32_t start);
esp_err_t lt7381_main_image_width(lt7381_panel_t *lt, uint16_t width);
esp_err_t lt7381_main_window_start_xy(lt7381_panel_t *lt, uint16_t x, uint16_t y);
esp_err_t lt7381_lcd_vertical_non_display(lt7381_panel_t *lt, uint16_t HY);
esp_err_t lt7381_lcd_vsync_start_position(lt7381_panel_t *lt, uint16_t HY);
esp_err_t lt7381_lcd_vsync_pulse_width(lt7381_panel_t *lt, uint16_t HY);
esp_err_t lt7381_memory_xy_mode(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_canvas_image_start_address(lt7381_panel_t *lt, uint32_t start);
esp_err_t lt7381_canvas_image_width(lt7381_panel_t *lt, uint16_t width);
esp_err_t lt7381_active_window_xy(lt7381_panel_t *lt, uint16_t x, uint16_t y);
esp_err_t lt7381_active_window_wh(lt7381_panel_t *lt, uint16_t w, uint16_t h);
esp_err_t lt7381_canvas_color_depth(lt7381_panel_t *lt, uint8_t setting);
esp_err_t lt7381_cursor_xy(lt7381_panel_t *lt, uint16_t x, uint16_t y);
esp_err_t lt7381_draw_start_xy(lt7381_panel_t *lt, uint16_t wx, uint16_t hy);
esp_err_t lt7381_draw_end_xy(lt7381_panel_t *lt, uint16_t wx, uint16_t hy);
esp_err_t lt7381_start_square_fill(lt7381_panel_t *lt);
esp_err_t lt7381_foreground_color(lt7381_panel_t *lt, uint16_t color);
esp_err_t lt7381_background_color(lt7381_panel_t *lt, uint16_t color);
#ifdef __cplusplus
}
#endif

#endif