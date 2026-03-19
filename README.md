# Overview

Driver library for the **LT7381 display controller** used in TFT modules such as: **ER-TFT-MC070-4**

The LT7381 controller is similar to the LT768x family, which itself is very close in architecture to the RA8876 controller.

This library aims (might never achieve it though) to provides a clean and documented driver implementation designed for ESP32 platforms using the ESP-IDF `esp_lcd` subsystem.

This library was created primarily to support the **ER-TFT-MC070-4 display module**, but it may also work with other displays using the same LT7381 controller.

The OEM of the module provides an example code but it is based on Arduino. I have rewritten this example code in C, relying only on esp-idf, dropping almost all GPU functionality (built-in drawing functions) as I intend to use LVGL.

Thus, the project currently focuses on **framebuffer streaming**, and there is an example implementation of LVGL x LT7381 in the examples folder.

Again, in bullet points:

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
- I80 (8bit & 16bit)
- I2C

However,
**only SPI has been tested so far.**

Currently, I can stream the whole screen [**800** (px. width) **x 480** (px. height) **x 2** (bytes per pixel)] in about **0.1 seconds**. Further speed is not needed for me this time, but from what I see, this could relatively easily be doubled with further SPI optimization and then x10-ed with i80.

One might even hit the LT7381's internal limit. It is recommended to check if input buffer has space before streaming the next byte but this is wasteful.  
I am skipping this check and have seen no issues, thou one can expect problems if the streaming speed is further increased.


# Software Architecture

The drivers has the following source files:
```
lt7381.c
lt7381.h
lt7381_config_default.h
lt7381_Internals.c
lt7381_Internals.h
lt7381_Registers.h
```
- **lt7381** contains the public facing functions intended for users of the driver.
- **lt7381_config_default** contains a default configuration, set up in a way that its simple for users to overwrite it if they wish.
- **lt7381_Internals** contains the internal functions intended for the driver's internals only.
- **lt7381_Registers** contains register map, bit field definitions and helper macros.

The arduino dependency has been minimized but not gone. The driver is based on esp-idf only, though on **version 4.4.7**, the latest esp-idf version included in arduinoespressif32 framework at the time of writing this.

# Example

PlatformIO example projects included under
```
├───examples
│   ├───lvgl
│   │   └───src
│   └───test
│       └───src
```


 - **test** displays a test screen, some color squares and a bitmap drawn by me :)
 
    [![Watch the video](https://img.youtube.com/vi/SUz6q_UQgxw/default.jpg)](https://youtu.be/SUz6q_UQgxw)

 - **lvgl** shows an LVGL compatible implementation with some minimal features

    [![Watch the video](https://img.youtube.com/vi/SUz6q_UQgxw/default.jpg)](https://youtu.be/SUz6q_UQgxw)
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
Flush time: 400 ms  
Frames per sec: ~3  

Typical SPI configuration:  
SPI clock: 40 MHz 
