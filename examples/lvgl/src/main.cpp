#include <Arduino.h>
#include "SPI.h"
#include "esp_heap_caps.h"

#include "main.hpp"
#include "lt7381.h"
#include "lvgl.h"

esp_lcd_panel_handle_t lcd_panel_handle = NULL;
esp_lcd_panel_io_handle_t io_handle = NULL;

static uint8_t *window_buffer = (uint8_t *)heap_caps_malloc(
  LT7381_SDRAM_LAYER_STEP,
  MALLOC_CAP_SPIRAM
);

/* Copy rendered image to screen. */
void my_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_buf)
{
  uint16_t width = area->x2 - area->x1 + 1;
  uint16_t height = area->y2 - area->y1 + 1;
  uint32_t width_height = width * height;

  //printf("Window is: %d, %d, %d, %d\n", area->x1, area->y1, area->x2+1, area->y2+1);
  //printf("Size is: %d, %d, or %d px2\n", width, height, width_height);

  uint8_t *dst = window_buffer;

  for(uint16_t row = 0; row < height; row++) {
    // pointer to start of this row in the full-screen px_buf
    uint8_t *src = px_buf + ((area->y1 + row) * LT7381_LCD_WIDTH + area->x1) * LT7381_SDRAM_BYTE_PER_PIXEL;

    // copy the row into continuous destination buffer
    memcpy(dst, src, width * LT7381_SDRAM_BYTE_PER_PIXEL);

    dst += width * LT7381_SDRAM_BYTE_PER_PIXEL; // advance destination
  }

  lcd_panel_handle->draw_bitmap(
    lcd_panel_handle,
    area->x1,
    area->y1,
    area->x2 + 1,
    area->y2 + 1,
    window_buffer
  );

  // Indicate that the buffer is available.
  // If DMA were used, call in the DMA complete interrupt.
  if (disp != NULL)
  {
    lv_display_flush_ready(disp);
  }
}

static void chart_update_cb(lv_timer_t *t)
{
  lv_obj_t * chart = (lv_obj_t *)lv_timer_get_user_data(t);
  lv_chart_series_t * ser = lv_chart_get_series_next(chart, NULL);

  lv_chart_set_next_value(chart, ser, lv_rand(10, 90));

  uint32_t p = lv_chart_get_point_count(chart);
  uint32_t s = lv_chart_get_x_start_point(chart, ser);
  int32_t * a = lv_chart_get_series_y_array(chart, ser);

  // create gap
  a[(s + 1) % p] = LV_CHART_POINT_NONE;
  a[(s + 2) % p] = LV_CHART_POINT_NONE;

  lv_chart_refresh(chart);
}

static void time_update_cb(lv_timer_t *t)
{
  lv_obj_t * label = (lv_obj_t *)lv_timer_get_user_data(t);

  uint32_t ms = lv_tick_get();
  uint32_t sec = ms / 1000;
  uint32_t min = sec / 60;
  uint32_t hr  = (min / 60) % 24;

  char buf[32];
  snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu",
            hr, min % 60, sec % 60);

  lv_label_set_text(label, buf);
}
#define MAX_VALUE 100
#define MIN_VALUE 0

static void set_value(void * bar, int32_t v)
{
    lv_bar_set_value((lv_obj_t *)bar, v, LV_ANIM_OFF);
}

static void event_cb(lv_event_t * e)
{
  lv_obj_t * obj = lv_event_get_target_obj(e);

  lv_draw_label_dsc_t label_dsc;
  lv_draw_label_dsc_init(&label_dsc);
  label_dsc.font = LV_FONT_DEFAULT;

  char buf[8];
  lv_snprintf(buf, sizeof(buf), "%d", (int)lv_bar_get_value(obj));

  lv_point_t txt_size;
  lv_text_get_size(&txt_size, buf, label_dsc.font, label_dsc.letter_space, label_dsc.line_space, LV_COORD_MAX,
                    label_dsc.flag);

  lv_area_t txt_area;
  txt_area.x1 = 0;
  txt_area.x2 = txt_size.x - 1;
  txt_area.y1 = 0;
  txt_area.y2 = txt_size.y - 1;

  lv_area_t indic_area;
  lv_obj_get_coords(obj, &indic_area);
  lv_area_set_width(&indic_area, lv_area_get_width(&indic_area) * lv_bar_get_value(obj) / MAX_VALUE);

  /*If the indicator is long enough put the text inside on the right*/
  if(lv_area_get_width(&indic_area) > txt_size.x + 20) {
      lv_area_align(&indic_area, &txt_area, LV_ALIGN_RIGHT_MID, -10, 0);
      label_dsc.color = lv_color_white();
  }
  /*If the indicator is still short put the text out of it on the right*/
  else {
      lv_area_align(&indic_area, &txt_area, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
      label_dsc.color = lv_color_black();
  }
  label_dsc.text = buf;
  label_dsc.text_local = true;
  lv_layer_t * layer = lv_event_get_layer(e);
  lv_draw_label(layer, &label_dsc, &txt_area);
}

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("Hello!");

  printf("PSRAM free: %d\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  printf("Internal free: %d\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

#if (LT7381_BUS_TYPE == LT7381_BUS_I80)

  esp_lcd_i80_bus_handle_t i80_bus = NULL;

  esp_lcd_i80_bus_config_t bus_config = {
    .clk_src = LCD_CLK_SRC_DEFAULT,
    .dc_gpio_num = LCD_DC,
    .wr_gpio_num = LCD_WR,
    .data_gpio_nums = {
        LCD_D0, LCD_D1, LCD_D2, LCD_D3,
        LCD_D4, LCD_D5, LCD_D6, LCD_D7,
        LCD_D8, LCD_D9, LCD_D10, LCD_D11,
        LCD_D12, LCD_D13, LCD_D14, LCD_D15
    },
    .bus_width = 16,
    .max_transfer_bytes = LCD_WIDTH * 100 * sizeof(uint16_t),
    .psram_trans_align = 64,
    .sram_trans_align = 4
  };

  esp_lcd_new_i80_bus(&bus_config, &i80_bus);

  esp_lcd_panel_io_i80_config_t io_config = {
    .cs_gpio_num = LCD_CS,
    .pclk_hz = 12000000,
    .trans_queue_depth = 32,
    .dc_levels = {
        .dc_idle_level = 0,
        .dc_cmd_level = 0,
        .dc_dummy_level = 0,
        .dc_data_level = 1
    },
    .flags = {
        .swap_color_bytes = 0,
        .pclk_idle_low = 0
    },
    .lcd_cmd_bits = 16,
    .lcd_param_bits = 8
  };

  esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle);

#elif (LT7381_BUS_TYPE == LT7381_BUS_SPI)

  spi_bus_config_t buscfg = {
    .mosi_io_num = LCD_SPI_MOSI,
    .miso_io_num = LCD_SPI_MISO,
    .sclk_io_num = LCD_SPI_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = LT7381_IMAGE_STREAM_CHUNK_SIZE + 10,
  };

  ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

  esp_lcd_panel_io_spi_config_t io_config =
  {
    .cs_gpio_num = LCD_CS,
    .dc_gpio_num = LCD_DC,
    .spi_mode = SPI_MODE0,
    .pclk_hz = 20000000,
    .trans_queue_depth = 16,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8,
    .flags =
    {
      .dc_as_cmd_phase = 0,
      .dc_low_on_data = 0,
      .octal_mode = 0,
      .lsb_first = 0
    }
  };

  esp_lcd_new_panel_io_spi(
    (esp_lcd_spi_bus_handle_t)SPI2_HOST,
    &io_config,
    &io_handle
  );

#elif (LT7381_BUS_TYPE == LT7381_BUS_IIC)

  i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = LCD_I2C_SDA,
    .scl_io_num = LCD_I2C_SCL,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 400000
  };

  i2c_param_config(I2C_NUM_0, &conf);
  i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);

#endif

  const esp_lcd_panel_lt7381_config_t vendor_config =
  {
    .wait_gpio_num = GPIO_NUM_NC,
    .lcd_backlight_pwm = GPIO_NUM_15,

#if (LT7381_BUS_TYPE == LT7381_BUS_I80)
    .mcu_bit_interface = 16,
#endif
  };


  esp_lcd_panel_dev_config_t panel_config = {
    .reset_gpio_num = LCD_RST,
    .color_space = ESP_LCD_COLOR_SPACE_RGB,
    .bits_per_pixel = 16,
    .flags =
    {
      .reset_active_high = 0
    },
    .vendor_config = (void *)&vendor_config
  };

  esp_lcd_new_panel_lt7381(io_handle, &panel_config, &lcd_panel_handle);
  lcd_panel_handle->reset(lcd_panel_handle);
  lcd_panel_handle->init(lcd_panel_handle);
  Serial.println("Init run");
  lcd_panel_handle->disp_off(lcd_panel_handle, false);
  esp_lcd_panel_clear(lcd_panel_handle, 0x0000);
  delay(100);

  lv_init();
  Serial.println("Init run");

  lv_tick_set_cb(my_get_millis);

  lv_display_t * display = lv_display_create(LT7381_LCD_WIDTH, LT7381_LCD_HEIGHT);

  // Create chart
  lv_obj_t * chart = lv_chart_create(lv_screen_active());
  lv_obj_set_size(chart, 350, 200);
  lv_obj_align(chart, LV_ALIGN_TOP_LEFT, 50, 50);

  lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_CIRCULAR);
  lv_chart_set_point_count(chart, 80);

  // Add series
  lv_chart_series_t * ser = lv_chart_add_series(
      chart,
      lv_palette_main(LV_PALETTE_RED),
      LV_CHART_AXIS_PRIMARY_Y
  );

  // Prefill data
  for (uint32_t i = 0; i < 80; i++) {
      lv_chart_set_next_value(chart, ser, lv_rand(10, 90));
  }

  lv_obj_t * bar = lv_bar_create(lv_screen_active());
  lv_bar_set_range(bar, MIN_VALUE, MAX_VALUE);
  lv_obj_set_size(bar, 200, 20);
  lv_obj_align(bar, LV_ALIGN_TOP_LEFT, 500, 210);
  lv_obj_add_event_cb(bar, event_cb, LV_EVENT_DRAW_MAIN_END, NULL);

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, bar);
  lv_anim_set_values(&a, 0, 100);
  lv_anim_set_exec_cb(&a, set_value);
  lv_anim_set_duration(&a, 4000);
  lv_anim_set_reverse_duration(&a, 4000);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_start(&a);

  /* LVGL will render to this for 2 bytes/pixel */
  static uint8_t *buf1 = (uint8_t *)heap_caps_malloc(
    LT7381_SDRAM_LAYER_STEP,
    MALLOC_CAP_SPIRAM// | MALLOC_CAP_DMA
  );
  static uint8_t *buf2 = (uint8_t *)heap_caps_malloc(
    LT7381_SDRAM_LAYER_STEP,
    MALLOC_CAP_SPIRAM// | MALLOC_CAP_DMA
  );
  Serial.println("malloc done");
  printf("buf1 add is: %lu\n", buf1);
  printf("buf2 add is: %lu\n", buf2);

  lv_display_set_buffers(
    display,
    buf1,
    NULL,
    LT7381_SDRAM_LAYER_STEP,
    LV_DISPLAY_RENDER_MODE_DIRECT
  );
  Serial.println("Buffer done");
  
  /* This callback will display the rendered image */
  lv_display_set_flush_cb(display, my_flush_cb);

  /* Create widgets */
  lv_obj_t * label = lv_label_create(lv_screen_active());
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 10);
  lv_label_set_text(label, "Hello LVGL!");
  lv_obj_set_height(label, 25);

  lv_obj_t * time_label = lv_label_create(lv_screen_active());
  lv_obj_align(time_label, LV_ALIGN_TOP_RIGHT, -10, 10);
  lv_label_set_text(time_label, "00:00:00");
  lv_obj_set_height(time_label, 25);

  lv_timer_create(chart_update_cb, 300, chart);
  lv_timer_create(time_update_cb, 1000, time_label);
  Serial.println("Setup done");
}

void loop()
{
  /* Provide updates to currently-displayed Widgets here. */
  lv_timer_handler();

}