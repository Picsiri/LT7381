/*
 * Publicly offered interface for this driver.
 */

#ifndef __LT7381_H__
#define __LT7381_H__


#ifdef __cplusplus
extern "C" {
#endif


/* include user config if available */
#if __has_include("lt7381_config.h")
#include "lt7381_config.h"
#else
#include "lt7381_config_default.h"
#endif

#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_interface.h"

/**
 * @brief Vendor specific configuration structure for panel device
 */
typedef struct {
  gpio_num_t    wait_gpio_num;      /* GPIO used to indicate busy state */
  gpio_num_t    lcd_backlight_pwm;  /* GPIO used to set backlight duty cycle */
} esp_lcd_panel_lt7381_config_t;

/**
 * @brief Create LCD panel for model LT7381
 *
 * @param[in] io LCD panel IO handle
 * @param[in] panel_dev_config general panel device configuration
 * @param[out] ret_panel Returned LCD panel handle
 * @return
 * 
 *          - ESP_ERR_INVALID_ARG   if parameter is invalid
 * 
 *          - ESP_ERR_NO_MEM        if out of memory
 * 
 *          - ESP_OK                on success
 */
extern esp_err_t esp_lcd_new_panel_lt7381(
  const esp_lcd_panel_io_handle_t io,
  const esp_lcd_panel_dev_config_t *panel_dev_config,
  esp_lcd_panel_handle_t *ret_panel
);

/**
 * @brief Update LCD panel backlight via RA8876/LT768x PWM
 *
 * @param[in] LCD panel handle
 * @param[in] backlight level (%)
 * @return
 * 
 *          - ESP_OK                on success
*/
extern esp_err_t esp_lcd_panel_set_backlight(esp_lcd_panel_t *panel, uint16_t duty);

extern esp_err_t eps_lcd_panel_draw_pixel(esp_lcd_panel_t *panel, uint16_t x, uint16_t y, uint16_t color);

extern esp_err_t esp_lcd_panel_draw_square_filled(esp_lcd_panel_t *panel, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

extern esp_err_t esp_lcd_panel_clear(esp_lcd_panel_t *panel, uint16_t color);

#ifdef __cplusplus
}
#endif


#endif