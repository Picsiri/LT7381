/*
 * Publicly offered interface for this driver.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* include user config if available */
#if __has_include("lt7381_config.h")
#include "lt7381_config.h"
#else
#include "lt7381_config_default.h"
#endif

/**
 * @brief Vendor specific configuration structure for panel device
 */
typedef struct {
  gpio_num_t    wait_gpio_num;      /* GPIO used to indicate busy state */
  gpio_num_t    lcd_backlight_pwm;  /* GPIO used to set backlight duty cycle */
  uint16_t      lcd_width;          /* Width of the LCD panel in pixels */
  uint16_t      lcd_height;         /* Height of the LCD panel in pixels */
  uint8_t       mcu_bit_interface;  /* Selection between 8-bit and 16-bit MCU interface */
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

#ifdef __cplusplus
}
#endif