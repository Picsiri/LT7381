
#include "lt7381.h"
#include "lt7381_Internals.h"

#ifndef LT7381_CONFIG_USER
#warning "Using LT7381 default configuration"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------ */
/* ------ Function headers ------ */
/* ------------------------------ */
static esp_err_t panel_lt7381_init(esp_lcd_panel_t *panel);
static esp_err_t panel_lt7381_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_lt7381_del(esp_lcd_panel_t *panel);

static esp_err_t panel_lt7381_draw_bitmap(
  esp_lcd_panel_t *panel,
  int x_start,
  int y_start,
  int x_end,
  int y_end,
  const void *color_data
);

static esp_err_t panel_lt7381_invert_color(
  esp_lcd_panel_t *panel,
  bool invert_color_data
);

static esp_err_t panel_lt7381_mirror(
  esp_lcd_panel_t *panel,
  bool mirror_x, bool mirror_y
);

static esp_err_t panel_lt7381_swap_xy(
  esp_lcd_panel_t *panel,
  bool swap_axes
);

static esp_err_t panel_lt7381_set_gap(
  esp_lcd_panel_t *panel,
  int x_gap, int y_gap
);

static esp_err_t panel_lt7381_disp_on(
  esp_lcd_panel_t *panel,
  bool on
);

/* ---------------------------- */
/* ----- Global Functions ----- */
/* ---------------------------- */

esp_err_t esp_lcd_new_panel_lt7381(
  const esp_lcd_panel_io_handle_t io_handle,          /**< handles IO itself, be it SPI or parallel */
  const esp_lcd_panel_dev_config_t *panel_dev_config, /**< Holds panel configuration data like pins and bpp (bits per pixel) */
  esp_lcd_panel_handle_t *ret_panel                   /**< Panel structure binding all together */
)
{
  esp_err_t                       ret = ESP_OK;
  lt7381_panel_t                 *lt7381 = NULL;
  esp_lcd_panel_lt7381_config_t  *vendor_cfg = 
    (esp_lcd_panel_lt7381_config_t *) panel_dev_config->vendor_config;

  ESP_GOTO_ON_FALSE(io_handle && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, err, PRINT_TAG, "invalid argument");
  ESP_GOTO_ON_FALSE(vendor_cfg, ESP_ERR_INVALID_ARG, err, PRINT_TAG, "vendor config cannot be null");

  lt7381 = calloc(1, sizeof(lt7381_panel_t));
  ESP_GOTO_ON_FALSE(lt7381, ESP_ERR_NO_MEM, err, PRINT_TAG, "no mem for lt7381 panel");

  /* RST GPIO config (if applicable) */
  if (panel_dev_config->reset_gpio_num != GPIO_NUM_NC)
  {
    gpio_config_t io_conf =
    {
      .mode = GPIO_MODE_OUTPUT,
      .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
    };
    ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, PRINT_TAG, "configure GPIO for RST line failed");
  }

  /* WAIT GPIO config (if applicable) */
  if (vendor_cfg->wait_gpio_num != GPIO_NUM_NC)
  {
    gpio_config_t io_conf =
    {
      .mode = GPIO_MODE_INPUT,
      .pin_bit_mask = 1ULL << vendor_cfg->wait_gpio_num,
      .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, PRINT_TAG, "configure GPIO for WAIT line failed");
  }
  
  // Note: Color space used for compatibility with IDF v4.4
  switch (panel_dev_config->color_space)
  {
  case ESP_LCD_COLOR_SPACE_RGB:
    break;
  default:
    ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, PRINT_TAG, "unsupported color space, only RGB supported.");
    break;
  }

  switch (panel_dev_config->bits_per_pixel)
  {
  case 16:
    break;
  default:
    ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, PRINT_TAG, "unsupported pixel width (supports 16-bit 5:6:5 format only)");
    break;
  }

  lt7381->io_handle = io_handle;
  lt7381->reset_gpio_num = panel_dev_config->reset_gpio_num;
  lt7381->wait_gpio_num = vendor_cfg->wait_gpio_num;
  lt7381->bytes_per_pixel = panel_dev_config->bits_per_pixel / 8u;
  lt7381->reset_gpio_num = panel_dev_config->reset_gpio_num;
  lt7381->reset_level = panel_dev_config->flags.reset_active_high;
  /*
  following interface callback setting section is based on
    esp_lcd_panel_interface.h
  these must be implemented by the driver
  */
  lt7381->esp_lcd_panel.reset = panel_lt7381_reset;
  lt7381->esp_lcd_panel.init = panel_lt7381_init;
  lt7381->esp_lcd_panel.del = panel_lt7381_del;
  lt7381->esp_lcd_panel.draw_bitmap = panel_lt7381_draw_bitmap;
  lt7381->esp_lcd_panel.mirror = panel_lt7381_mirror;
  lt7381->esp_lcd_panel.swap_xy = panel_lt7381_swap_xy;
  lt7381->esp_lcd_panel.set_gap = panel_lt7381_set_gap;
  lt7381->esp_lcd_panel.invert_color = panel_lt7381_invert_color;
  lt7381->esp_lcd_panel.disp_off = panel_lt7381_disp_on_off;

 #if (LT7381_BACKLIGHT_TYPE == LT7381_BACKLIGHT_EXT_PWM)
  ledc_timer_config_t ledc_timer = {
      .speed_mode       = LEDC_LOW_SPEED_MODE,
      .duty_resolution  = LT7381_BACKLIGHT_RESOLUTION,
      .timer_num        = LT7381_BACKLIGHT_TIMER,
      .freq_hz          = 5000,  // PWM frequency, adjust as needed
      .clk_cfg          = LEDC_AUTO_CLK
  };
  ledc_timer_config(&ledc_timer);

  ledc_channel_config_t ledc_channel = {
      .gpio_num       = lt7381->pwm_gpio_num,
      .speed_mode     = LEDC_LOW_SPEED_MODE,
      .channel        = LT7381_BACKLIGHT_CHANNEL,
      .intr_type      = LEDC_INTR_DISABLE,
      .timer_sel      = LT7381_BACKLIGHT_TIMER,
      .duty           = 0, // start off
      .hpoint         = 0
  };
  ledc_channel_config(&ledc_channel);
 #endif

  *ret_panel = &(lt7381->esp_lcd_panel);
  ESP_LOGD(PRINT_TAG, "new lt7381 panel @%p", lt7381);

  return ESP_OK;

  /* ------------------------ */
  /* ---- ERROR HANDLING ---- */
  /* ------------------------ */

 err:
  if (lt7381)
  {
    /* tidy up RST and/or WAIT gpio pins */
    if (lt7381->reset_gpio_num != GPIO_NUM_NC)
      gpio_reset_pin(lt7381->reset_gpio_num);
    if (lt7381->wait_gpio_num != GPIO_NUM_NC)
      gpio_reset_pin(lt7381->wait_gpio_num);
    free(lt7381);
  }
  return ret;
}

esp_err_t esp_lcd_panel_set_backlight(esp_lcd_panel_t *panel, uint16_t duty)
{
  esp_err_t       ret = ESP_OK;
  lt7381_panel_t *lt7381 = __containerof(panel, lt7381_panel_t, esp_lcd_panel);
  uint32_t        ledc_duty;

  if (duty == lt7381->backlight_duty) {
    return ret;
  }

#if (LT7381_BACKLIGHT_MODE == LT7381_BACKLIGHT_INT_PWM)
  // Software PWM not implemented yet
  #error "Software PWM backlight not implemented"
#else
  // Scale 16-bit duty to LEDC resolution
  ledc_duty = ((uint32_t)duty * ((1 << LT7381_BACKLIGHT_RESOLUTION) - 1)) / 0xFFFF;

  ret = ledc_set_duty_and_update(
    LT7381_BACKLIGHT_MODE,
    LT7381_BACKLIGHT_CHANNEL,
    ledc_duty,
    0
  );

  if (ret == ESP_OK) {
    lt7381->backlight_duty = duty;
  }
#endif

  return ret;
}

/* ---------------------------------- */
/* - Interface Function Definitions - */
/* ---------------------------------- */

static esp_err_t panel_lt7381_reset(esp_lcd_panel_t *panel)
{
  lt7381_panel_t *lt7381 = __containerof(panel, lt7381_panel_t, esp_lcd_panel);
  esp_lcd_panel_io_handle_t    io_handle = lt7381->io_handle;

  if (lt7381->reset_gpio_num != GPIO_NUM_NC)
  {
    /* perform hardware reset */
    gpio_set_level(lt7381->reset_gpio_num, lt7381->reset_level);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(lt7381->reset_gpio_num, !lt7381->reset_level);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  else
  {
    /* perform software reset */
    esp_lcd_panel_io_tx_param(io_handle, LT7381_REGISTER_SRR, (uint8_t[]){ 0x01 }, 1u);
    vTaskDelay(pdMS_TO_TICKS(20));
  }

  return ESP_OK;
}

static esp_err_t panel_lt7381_init(esp_lcd_panel_t *panel)
{
  lt7381_panel_t *lt7381 = __containerof(panel, lt7381_panel_t, esp_lcd_panel);
  esp_err_t       ret = ESP_OK;
  uint8_t status;
  
  // TODO: wrapp all calls on error handler
  ESP_GOTO_ON_ERROR(lt7381_system_wait_ready(panel), err, PRINT_TAG, "Panel failed to settle in time. Display unavailable.");
  
  vTaskDelay(pdMS_TO_TICKS(100));

  // TODO: might not even needed as this check is part of the lt7381_system_wait_ready() call a few lines up. No capacity to test though.
  do
  {
    lt7381_status_read(panel, &status);
  }
  while(GET_BIT(status, STAT_REG_INHIBIT_OPERATION_BIT) > 0u);

  lt7381_pll_init(panel);

  lt7381_SDRAM_init(panel);
  
  // commented calls are skipped because they set reset-default values

  lt7381_tft_panel_setting(panel, CCR_REG_TFT_16_BIT);
 #if (LT7381_BUS_TYPE == LT7381_BUS_I80)
  lt7381_bus_width_setting(panel, CCR_REG_BUS_16_BIT);
 #endif

  //lt7381_image_data_format(panel, MACR_REG_DIRECT_WRITE);
  //lt7381_memwrite_directions_setting(panel, MACR_REG_LEFT_RIGHT_TOP_BOT);

  //lt7381_graphic_or_text_mode(panel, ICR_REG_GRAPHIC_MODE);
  //lt7381_memory_select(panel, ICR_REG_MEMORY_AS_IMAGE_BUFFER);

  lt7381_PLCK_polarity(panel, DPCR_REG_PCLK_FALLING_EDGE);
  lt7381_display_test_on_off(panel, DPCR_REG_DISPLAY_TEST_ON);
  //lt7381_display_vertical_direction(panel, DPCR_REG_TOP_TO_BOTTOM);
  //lt7381_display_color_sequence(panel, DPCR_REG_COLOR_SEQ_RGB);

  //lt7381_hsync_polarity(panel, PCSR_REG_HSYNC_LOW_ACTIVE);
  //lt7381_vsync_polarity(panel, PCSR_REG_VSYNC_LOW_ACTIVE);
  lt7381_pde_polarity(panel, PCSR_REG_PDE_LOW_ACTIVE);

  lt7381_lcd_horizontal_width_vertical_height(panel, LT7381_LCD_WIDTH, LT7381_LCD_HEIGHT);
  lt7381_lcd_horizontal_non_display(panel, LT7381_LCD_HORIZONTAL_NON_DISP);
  lt7381_lcd_hsync_start_position(panel, LT7381_LCD_HSYNC_START_POS);
  lt7381_lcd_hsync_pulse_width(panel, LT7381_LCD_HSYNC_PULSE_WIDTH);
  lt7381_lcd_vertical_non_display(panel, LT7381_LCD_VERTICAL_NON_DISP);
  lt7381_lcd_vsync_start_position(panel, LT7381_LCD_VSYNC_START_POS);
  lt7381_lcd_vsync_pulse_width(panel, LT7381_LCD_VSYNC_PULSE_WIDTH);

  //lt7381_select_main_image_color_depth(panel, MPWCTR_REG_COLOR_DEPTH_16BPP);

  //lt7381_memory_xy_mode(panel, AW_COLOR_REG_BLOCK_MODE);
  lt7381_canvas_color_depth(panel, AW_COLOR_REG_16BPP_MODE);
  
  //lt7381_select_main_image_color_depth(panel, MPWCTR_REG_COLOR_DEPTH_16BPP);

  lt7381_display_on_off(panel, DPCR_REG_DISPLAY_ON);

/* ------------------------ */
/* ---- ERROR HANDLING ---- */
/* ------------------------ */

err:
  if (lt7381)
  {
    /* tidy up RST and/or WAIT gpio pins */
    if (lt7381->reset_gpio_num != GPIO_NUM_NC)
      gpio_reset_pin(lt7381->reset_gpio_num);
    if (lt7381->wait_gpio_num != GPIO_NUM_NC)
      gpio_reset_pin(lt7381->wait_gpio_num);
    free(lt7381);
  }
  return ret;
}

static esp_err_t panel_lt7381_del(esp_lcd_panel_t *panel)
{
  lt7381_panel_t *lt7381 = __containerof(panel, lt7381_panel_t, esp_lcd_panel);
  
  ESP_LOGD(PRINT_TAG, "del lt7381 panel @%p", lt7381);
  free(lt7381);
  return ESP_OK;
}

static esp_err_t panel_lt7381_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
  lt7381_panel_t *lt7381 = __containerof(panel, lt7381_panel_t, esp_lcd_panel);
  esp_lcd_panel_io_handle_t io_handle = lt7381->io_handle;
  assert((x_start < x_end) && (y_start < y_end) && "start position must be smaller than end position");
  uint16_t width, height;

  x_start += lt7381->x_gap;
  x_end += lt7381->x_gap;
  y_start += lt7381->y_gap;
  y_end += lt7381->y_gap;

  width = x_end - x_start;
  height = y_end - y_start;

  /* define an area of frame memory where MCU can access */
  lt7381_active_window_xy(panel, x_start, y_start);
  lt7381_active_window_wh(panel, width, height);

  /* set cursor */
  lt7381_cursor_xy(panel, x_start, y_start);

  /* Write to graphic RAM */
  uint32_t len = (x_end - x_start) * (y_end - y_start) * lt7381->bytes_per_pixel;
  
  const uint8_t *data = (const uint8_t *)color_data;

  lt7381_cmd_write(panel, LT7381_REGISTER_MRWDP);
  for (uint32_t i = 0u; i < len; i++)
  {
    lt7381_data_write(panel, data[i]);
    panel_lt7381_wait(panel);
  }
  
  return ESP_OK;
}

static esp_err_t panel_lt7381_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
  ESP_LOGE(PRINT_TAG, "mirror is unsupported");
  return ESP_ERR_NOT_SUPPORTED;
}

static esp_err_t panel_lt7381_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
{
  ESP_LOGE(PRINT_TAG, "swap_xy is unsupported");
  return ESP_ERR_NOT_SUPPORTED;
}

static esp_err_t panel_lt7381_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
  lt7381_panel_t *lt7381 = __containerof(panel, lt7381_panel_t, esp_lcd_panel);
  
  lt7381->x_gap = x_gap;
  lt7381->y_gap = y_gap;
  
  return ESP_OK;
}

static esp_err_t panel_lt7381_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
  ESP_LOGE(PRINT_TAG, "invert color is unsupported");
  return ESP_ERR_NOT_SUPPORTED;
}

static esp_err_t panel_lt7381_disp_on_off(esp_lcd_panel_t *panel, bool on)
{
  lt7381_panel_t *lt7381 = __containerof(panel, lt7381_panel_t, esp_lcd_panel);

  if (on)
  {
    return lt7381_display_on_off(panel, DPCR_REG_DISPLAY_ON);
  }
  else
  {
    return lt7381_display_on_off(panel, DPCR_REG_DISPLAY_OFF);
  }
}

#ifdef __cplusplus
}
#endif