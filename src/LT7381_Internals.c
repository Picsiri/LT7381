

#include "LT7381_Internals.h"
#include "LT7381.h"

static void panel_lt7381_wait(esp_lcd_panel_t *panel)
{
  lt7381_panel_t *lt7381 = __containerof(panel, lt7381_panel_t, esp_lcd_panel);
  uint64_t        start = esp_timer_get_time();
  uint64_t        now = start;

  if (lt7381->wait_gpio_num >= GPIO_NUM_0)
  {
    while (
      gpio_get_level(lt7381->wait_gpio_num) == 0 &&
      ((now = esp_timer_get_time()) - start) < LT7381_TIMEOUT_US
    );
    if ((now - start) > LT7381_TIMEOUT_US)
    {
      ESP_LOGE(PRINT_TAG, "lt7381 Timeout!");
      //            ESP_ERROR_CHECK(ESP_ERR_TIMEOUT);
    }
  }
}

static esp_err_t panel_lt7381_tx_param(esp_lcd_panel_t *panel, int lcd_cmd, uint8_t param)
{
  lt7381_panel_t *lt7381 = __containerof(panel, lt7381_panel_t, esp_lcd_panel);
  esp_lcd_panel_io_handle_t io_handle = lt7381->io_handle;

  panel_lt7381_wait(panel);
  return esp_lcd_panel_io_tx_param(io_handle, lcd_cmd, (uint8_t[]) {  param }, 1);
}


static void panel_lt7381_set_window(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end)
{
  /* set active window start X/Y */
  panel_lt7381_tx_param(panel, lt7381_REG_AWUL_X0, x_start & 0xff);
  panel_lt7381_tx_param(panel, lt7381_REG_AWUL_X1, (x_start >> 8) & 0xff);
  panel_lt7381_tx_param(panel, lt7381_REG_AWUL_Y0, y_start & 0xff);
  panel_lt7381_tx_param(panel, lt7381_REG_AWUL_Y1, (y_start >> 8) & 0xff);

  /* set active window width and height */
  panel_lt7381_tx_param(panel, lt7381_REG_AW_WTH0, (x_end - x_start) & 0xff);
  panel_lt7381_tx_param(panel, lt7381_REG_AW_WTH1, ((x_end - x_start) >> 8) & 0xff);
  panel_lt7381_tx_param(panel, lt7381_REG_AW_HT0, (y_end - y_start) & 0xff);
  panel_lt7381_tx_param(panel, lt7381_REG_AW_HT1, ((y_end - y_start) >> 8) & 0xff);
}

static void panel_lt7381_set_cursor(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end)
{
  panel_lt7381_tx_param(panel, lt7381_REG_CURH0, x_start & 0xff);
  panel_lt7381_tx_param(panel, lt7381_REG_CURH1, (x_start >> 8) & 0xff);
  panel_lt7381_tx_param(panel, lt7381_REG_CURV0, y_start & 0xff);
  panel_lt7381_tx_param(panel, lt7381_REG_CURV1, (y_start >> 8) & 0xff);
}