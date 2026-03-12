#pragma once

#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "driver/gpio.h"
#include "hal/ledc_types.h"
#include "LT7381_Registers.h"
//#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef CONFIG_RA8876_PANEL_CUSTOM
#define  RA8876_OSC_FREQ        CONFIG_RA8876_OSC_FREQ    /* crystal clock (MHz) */
#define  RA8876_DRAM_FREQ      CONFIG_RA8876_DRAM_FREQ    /* SDRAM clock frequency (MHz) */
#define RA8876_CORE_FREQ      CONFIG_RA8876_CORE_FREQ    /* core (system) clock frequency (MHz) */
#define RA8876_SCAN_FREQ      CONFIG_RA8876_SCAN_FREQ    /* pixel scan clock frequency (MHz) */

#define RA8876_PANEL_HNDR      CONFIG_RA8876_HNDR      /* horizontal non-display period or back porch */
#define RA8876_PANEL_HSTR      CONFIG_RA8876_HSTR      /* horizontal start position or front porch */
#define RA8876_PANEL_HPWR      CONFIG_RA8876_HPWR      /* HSYNC pulse width */
#define RA8876_PANEL_VNDR      CONFIG_RA8876_VNDR      /* vertical non-display period */
#define RA8876_PANEL_VSTR      CONFIG_RA8876_VSTR      /* vertical start position */
#define RA8876_PANEL_VPWR      CONFIG_RA8876_VPWR      /* VSYNC pulse width */
#endif

/* pre-defined panel definitions here (also add to Kconfig file) */
#ifdef CONFIG_RA8876_PANEL_ER_TFTM070_6
#define  RA8876_OSC_FREQ        10              /* crystal clock (MHz) */
#define  RA8876_DRAM_FREQ      100              /* SDRAM clock frequency (MHz) */
#define RA8876_CORE_FREQ      100              /* core (system) clock frequency (MHz) */
#define RA8876_SCAN_FREQ      50              /* pixel scan clock frequency (MHz) */

#define RA8876_PANEL_HNDR      160              /* horizontal non-display period or back porch */
#define RA8876_PANEL_HSTR      160              /* horizontal start position or front porch */
#define RA8876_PANEL_HPWR      70              /* HSYNC pulse width */
#define RA8876_PANEL_VNDR      23              /* vertical non-display period */
#define RA8876_PANEL_VSTR      12              /* vertical start position */
#define RA8876_PANEL_VPWR      10              /* VSYNC pulse width */
#endif


#define RA8876_PLL_DIV_2      0x02            /* PLL divided by 2 */
#define RA8876_PLL_DIV_4      0x04            /* PLL divided by 4 */
#define RA8876_PLL_DIV_8      0x06            /* PLL divided by 8 */
#define RA8876_PLL_DIV_16     0x16            /* PLL divided by 16 */
#define RA8876_PLL_DIV_32     0x26            /* PLL divided by 32 */
#define RA8876_PLL_DIV_64     0x36            /* PLL divided by 64 */

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