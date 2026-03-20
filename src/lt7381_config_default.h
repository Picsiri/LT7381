#ifndef __LT7381_CONFIG_DEFAULT_H__
#define __LT7381_CONFIG_DEFAULT_H__

/* bus selection */

#define LT7381_BUS_SPI                  1
#define LT7381_BUS_I80                  2
#define LT7381_BUS_IIC                  3

#define LT7381_BUS_TYPE                 LT7381_BUS_SPI

#define LT7381_IMAGE_STREAM_CHUNK_SIZE  64

/* display size */

#define LT7381_LCD_WIDTH                800u
#define LT7381_LCD_HEIGHT               480u
#define LT7381_LCD_HORIZONTAL_NON_DISP  140u
#define LT7381_LCD_HSYNC_START_POS      160u
#define LT7381_LCD_HSYNC_PULSE_WIDTH    20u
#define LT7381_LCD_VERTICAL_NON_DISP    20u
#define LT7381_LCD_VSYNC_START_POS      12u
#define LT7381_LCD_VSYNC_PULSE_WIDTH    3u

/* backlight */

#define LT7381_BACKLIGHT_INT_PWM        1 /**< use the LT7381's PWM generator */
#define LT7381_BACKLIGHT_EXT_PWM        2 /**< use an externally applied PWM */

#define LT7381_BACKLIGHT_RESOLUTION     8
#define LT7381_BACKLIGHT_FREQUENCY      20000
#define LT7381_BACKLIGHT_TIMER          LEDC_TIMER_0
#define LT7381_BACKLIGHT_MODE           LEDC_LOW_SPEED_MODE
#define LT7381_BACKLIGHT_CHANNEL        LEDC_CHANNEL_2
#define LT7381_BACKLIGHT_FULL_SCALE     0xFFFF

#define LT7381_BACKLIGHT_TYPE           LT7381_BACKLIGHT_EXT_PWM

/* misc */

#define LT7381_RETRY_LIMIT_RESET        5
#define LT7381_RETRY_LIMIT_FAIL         25

/* clocks */
/*  example configuration for 800x480, 10MHz XI
 *  PCLK  -   OD        11b -   2 - 0x02
 *  25MHz -   R       1010b -  10 - 0x0A
 *        -   N   01100100b - 100 - 0x64
 * 
 *  MCLK  -   OD        11b -   2 - 0x02
 *  50MHz -   R       1010b -  10 - 0x0A
 *        -   N   11001000b - 200 - 0xC8
 * 
 *  CCLK  -   OD        11b -   2 - 0x02
 *  50MHz -   R       1010b -  10 - 0x0A
 *        -   N   11001000b - 200 - 0xC8
 */
#define LT7381_PLL_OD_SCLK              0x02
#define LT7381_PLL_OD_CCLK              0x02
#define LT7381_PLL_OD_MCLK              0x02

#define LT7381_PLL_R_SCLK               0x0A
#define LT7381_PLL_R_CCLK               0x0A
#define LT7381_PLL_R_MCLK               0x0A

#define LT7381_PLL_N_SCLK               0x64
#define LT7381_PLL_N_CCLK               0xC8
#define LT7381_PLL_N_MCLK               0xC8

/* pwm s - setting full scale for panels with internal pwm */
// disable this in your config to save power
#define LT7381_PWM_MUX_DEFAULT          0x0A /* output pwm0 and pwm1 on the pins */
#define LT7381_PWM_CFG_DEFAULT          0x33 /* auto reload and start booth pwms */
#define LT7381_PWM_MUX_OFF              0x00
#define LT7381_PWM_CFG_OFF              0x00
#define LT7381_PWM_FULL_SCALE           0x64 /* 100 is base 10 */

/* SDRAM */
#define LT7381_SDRAR_DEFAULT            0x29 /* datasheet says 0x20 but this was in example from the OEM */
#define LT7381_SDRMD_DEFAULT            0x03 /* CAS:2=0x02 ACAS:3=0x03 */

#define LT7381_SDRAM_REFRESH_TIME       0x01E6 /* comes from vendor example, more might be possible */
#define LT7381_SDRAM_BYTE_PER_PIXEL     2u
#define LT7381_SDRAM_LAYER_STEP         LT7381_SDRAM_BYTE_PER_PIXEL * LT7381_LCD_WIDTH * LT7381_LCD_HEIGHT
#define LT7381_SDRAM_LAYER_0            0u
#define LT7381_SDRAM_LAYER_1            LT7381_SDRAM_LAYER_0 + LT7381_SDRAM_LAYER_STEP
#define LT7381_SDRAM_LAYER_2            LT7381_SDRAM_LAYER_1 + LT7381_SDRAM_LAYER_STEP
#define LT7381_SDRAM_LAYER_3            LT7381_SDRAM_LAYER_2 + LT7381_SDRAM_LAYER_STEP








#endif