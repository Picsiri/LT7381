#pragma once

/* bus selection */

#define LT7381_BUS_SPI                  1
#define LT7381_BUS_I80                  2
#define LT7381_BUS_IIC                  3

#define LT7381_BUS_TYPE                 LT7381_BUS_SPI

/* display size */

#define LT7381_LCD_WIDTH                800
#define LT7381_LCD_HEIGHT               480

/* backlight */

#define LT7381_BACKLIGHT_INT_PWM        1 /**< use the LT7381's PWM generator */
#define LT7381_BACKLIGHT_EXT_PWM        2 /**< use an externally applied PWM */

#define LT7381_BACKLIGHT_RESOLUTION     16  
#define LT7381_BACKLIGHT_TIMER          LEDC_HIGH_SPEED_TIMER_0
#define LT7381_BACKLIGHT_MODE           LEDC_LOW_SPEED_MODE
#define LT7381_BACKLIGHT_CHANNEL        LEDC_CHANNEL_2
#define LT7381_BACKLIGHT_FULL_SCALE     0xFFFF

#define LT7381_BACKLIGHT_TYPE           LT7381_BACKLIGHT_EXT_PWM

/* misc */

#define LT7381_RETRY_LIMIT_RESET        5
#define LT7381_RETRY_LIMIT_FAIL         25

/* clocks */
#define LT7381_PLL_OD_SCLK              2
#define LT7381_PLL_OD_CCLK              2
#define LT7381_PLL_OD_MCLK              2

#define LT7381_PLL_R_SCLK               5
#define LT7381_PLL_R_CCLK               5
#define LT7381_PLL_R_MCLK               5

#define LT7381_PLL_N_SCLK               60
#define LT7381_PLL_N_CCLK               100
#define LT7381_PLL_N_MCLK               100

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

#define LT7381_SDRAM_REFRESH_TIME       0x01D2 /* comes from vendor example, more might be possible */
