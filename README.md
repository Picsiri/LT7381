# Overview

Driver library for the **LT7381 display controller** used in TFT modules such as: **ER-TFT-MC070-4**

The LT7381 controller is similar to the LT768x family, which itself is very close in architecture to the RA8876 controller.

This library provides a clean and documented driver implementation designed for ESP32 platforms using the ESP-IDF `esp_lcd` subsystem.

This library was created primarily to support the **ER-TFT-MC070-4 display module**, but it may also work with other displays using the same LT7381 controller.

The driver currently focuses on **framebuffer streaming**, making it compatible with rendering engines such as LVGL.

Key design choices:

- Uses ESP-IDF `esp_lcd` as the bus abstraction
- Designed to integrate easily with LVGL
- Clean and well documented driver implementation
- PlatformIO friendly

# Hardware Support

Tested hardware:

ER-TFT-MC070-4  
Controller: LT7381  
Resolution: 800x480

The LT7381 is closely related to:

- LT7680
- LT7683
- RA8876

Because of this similarity, the library **may work with other LT768x / RA8876 based displays**, but this has **not been tested yet**.

# Interfaces

This library relies on the **esp_lcd interface layer**.

Because of this, the display can theoretically be used over:

- SPI
- I80 parallel
- Other interfaces supported by esp_lcd

However:

**Only SPI has been tested so far.**

# Software Architecture

Rendering pipeline:

Application  
↓  
Graphics library (LVGL optional)  
↓  
LT7381 driver  
↓  
ESP-IDF `esp_lcd`  
↓  
SPI / I80 bus  
↓  
Display controller VRAM

The driver does **not rely on the hardware accelerator functions** of the LT7381.  
Instead it focuses on **efficient pixel streaming**, making it compatible with modern GUI frameworks.

# Example

PlatformIO example projects included:



 - examples/test displays a bitmap on the display

 - examples/lvgl shows an lvgl compatible implementation

# Installation

Using PlatformIO:

```
lib_deps =
[https://github.com/](https://github.com/)Picsiri/LT7381
```

or via PIO repository:

```
asdasd
```

# References

BuyDisplay product page:  
https://www.buydisplay.com/low-cost-arduino-7-inch-touch-screen-800x480-i2c-lcd-tft-spi-display

LT7381 Datasheet:  
https://www.levetop.cn/uploadfiles/2023/05/LT7381_BFDS_V11_ENG.pdf

Special thanks to danmeuk. Used his repo as reference:  
https://github.com/danmeuk/esp_lcd_ra8876

# Pin Configuration

I have used an ESP-S3 based module:   
https://github.com/vcc-gnd/YD-ESP32-S3?tab=readme-ov-file

and ER-TFT-MC070-4 display module:  
https://www.buydisplay.com/low-cost-arduino-7-inch-touch-screen-800x480-i2c-lcd-tft-spi-display

SPI wiring:

| PIN | ESP32-S3| Display | PIN |
|-----|---------|---------|-----|
| GND |   GND   |   GND   | 1,2 |
| VCC |   VCC   |   VCC   | 3,4 |
| 10  |   CS    |   CS    |  5  |
| 13  |   MISO  |   SDO   |  6  |
| 11  |   MOSI  |   SDI   |  7  |
| 12  |   SCLK  |   SCLK  |  8  |
| xx  |   WAIT  |   WAIT  |  9  |
| 16  |   RST   |  RESET  |  11 |
| xx  |   PWM   |  BL_PWM |  14 |

---

# Performance

Resolution: 800x480  
Color depth: RGB565  
Frames per sec: ?  

Typical SPI configuration:

SPI clock: <insert> MHz  
Flush time: <insert> ms
