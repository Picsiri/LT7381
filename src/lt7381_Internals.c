#include "lt7381.h"
#include "lt7381_Internals.h"

/* ------------------------------------------------------------------ */
/* low-level helpers                                                  */
/* ------------------------------------------------------------------ */

static esp_err_t lt7381_cmd_write(lt7381_panel_t *lt, uint8_t cmd)
{
  return esp_lcd_panel_io_tx_param(
    lt->io_handle,
    LT7381_CMD_WRITE_COMMAND,
    &cmd,
    1
  );
}

static esp_err_t lt7381_data_write(lt7381_panel_t *lt, uint8_t data)
{
  return esp_lcd_panel_io_tx_param(
    lt->io_handle,
    LT7381_CMD_WRITE_DATA,
    &data,
    1
  );
}

static esp_err_t lt7381_data_read(lt7381_panel_t *lt, uint8_t *data)
{
  return esp_lcd_panel_io_rx_param(
    lt->io_handle,
    LT7381_CMD_READ_DATA,
    data,
    1
  );
}

static esp_err_t lt7381_status_read(lt7381_panel_t *lt, uint8_t *status)
{
  return esp_lcd_panel_io_rx_param(
    lt->io_handle,
    LT7381_CMD_READ_STATUS,
    status,
    1
  );
}

/* ------------------------ */
/* --- register helpers --- */
/* ------------------------ */

static esp_err_t lt7381_reg_write(lt7381_panel_t *lt, uint8_t reg, uint8_t val)
{
  esp_err_t ret;

  ret = lt7381_cmd_write(lt, reg);
  if (ret != ESP_OK) return ret;

  return lt7381_data_write(lt, val);
}

static esp_err_t lt7381_reg_read(lt7381_panel_t *lt, uint8_t reg, uint8_t *val)
{
  lt7381_cmd_write(lt, reg);
  return lt7381_data_read(lt, val);
}

esp_err_t lt7381_system_wait_ready(esp_lcd_panel_t *panel)
{
  lt7381_panel_t *lt = __containerof(panel, lt7381_panel_t, esp_lcd_panel);
  uint8_t status;
  uint8_t ccr;
  uint8_t retry = 0u;
  uint8_t fail = 0u;

  while (1)
  {
    ESP_ERROR_CHECK(lt7381_status_read(lt, &status));

    if (GET_BIT(status, STAT_REG_INHIBIT_OPERATION_BIT) == 0u)
    {
      /* one might think lt7381_reg_read() can be used but these commands are slow and take time, delays are needed */
      vTaskDelay(pdMS_TO_TICKS(2));
      ESP_ERROR_CHECK(lt7381_cmd_write(lt, LT7381_REGISTER_CCR));
      vTaskDelay(pdMS_TO_TICKS(2));
      ESP_ERROR_CHECK(lt7381_data_read(lt, &ccr));

      if (GET_BIT(ccr, CCR_REG_PLL_READY_BIT) > 0u)
      {
        return ESP_OK;
      }
      else
      {
        vTaskDelay(pdMS_TO_TICKS(2));
        ESP_ERROR_CHECK(lt7381_cmd_write(lt, LT7381_REGISTER_CCR));
        vTaskDelay(pdMS_TO_TICKS(2));
        ESP_ERROR_CHECK(lt7381_data_write(lt, BIT_TO_VAL(CCR_REG_PLL_READY_BIT)));
      }
    }
    else
    {
      retry++;

      if (retry >= LT7381_RETRY_LIMIT_RESET)
      {  
        fail++;
        if (fail >= LT7381_RETRY_LIMIT_FAIL)
        {
          return ESP_ERR_TIMEOUT;
        }
        
        ESP_LOGE(PRINT_TAG, "Display failed to initialize, resetting and retrying!");
        lt->esp_lcd_panel.reset(&lt->esp_lcd_panel);

        retry = 0;
      }
    }
  }
}

static esp_err_t lt7381_pll_init(esp_lcd_panel_t *panel)
{
    esp_err_t ret = ESP_OK;

    /* PLL parameters (Fout = Fin * (N/R) / OD) */
    uint16_t lpllOD_sclk = LT7381_PLL_OD_SCLK;
    uint16_t lpllOD_cclk = LT7381_PLL_OD_CCLK;
    uint16_t lpllOD_mclk = LT7381_PLL_OD_MCLK;

    uint16_t lpllR_sclk  = LT7381_PLL_R_SCLK;
    uint16_t lpllR_cclk  = LT7381_PLL_R_CCLK;
    uint16_t lpllR_mclk  = LT7381_PLL_R_MCLK;

    uint16_t lpllN_sclk  = LT7381_PLL_N_SCLK;
    uint16_t lpllN_cclk  = LT7381_PLL_N_CCLK;
    uint16_t lpllN_mclk  = LT7381_PLL_N_MCLK;

    /* -------- PLL config registers -------- */
    
    ret |= lt7381_reg_write(
      panel,
      LT7381_REGISTER_PPLLC1,
      (lpllOD_sclk << 6) |
      (lpllR_sclk  << 1) |
      ((lpllN_sclk >> 8) & 0x01)
    );

    ret |= lt7381_reg_write(
      panel,
      LT7381_REGISTER_MPLLC1,
      (lpllOD_mclk << 6) |
      (lpllR_mclk  << 1) |
      ((lpllN_mclk >> 8) & 0x01)
    );

    ret |= lt7381_reg_write(
      panel,
      LT7381_REGISTER_CPLLC1,
      (lpllOD_cclk << 6) |
      (lpllR_cclk  << 1) |
      ((lpllN_cclk >> 8) & 0x01)
    );

    /* -------- N values -------- */

    ret |= lt7381_reg_write(
      panel,
      LT7381_REGISTER_PPLLC2,
      lpllN_sclk
    );

    ret |= lt7381_reg_write(
      panel,
      LT7381_REGISTER_MPLLC2,
      lpllN_mclk
    );

    ret |= lt7381_reg_write(
      panel,
      LT7381_REGISTER_CPLLC2,
      lpllN_cclk
    );

    /* -------- reconfig PLL -------- */

    ret |= lt7381_cmd_write(panel, LT7381_REGISTER_SRR);
    ets_delay_us(1);
    ret |= lt7381_data_write(panel, 0x80);
    vTaskDelay(pdMS_TO_TICKS(1));

    /* -------- default PWM setup -------- */

    ret |= lt7381_reg_write(
      panel,
      LT7381_REGISTER_PMUXR,
      LT7381_PWM_MUX_DEFAULT
    );

    ret |= lt7381_reg_write(
      panel,
      LT7381_REGISTER_TCMPB0_LOW,
      LT7381_PWM_FULL_SCALE
    );

    ret |= lt7381_reg_write(
      panel,
      LT7381_REGISTER_TCNTB0_LOW,
      LT7381_PWM_FULL_SCALE
    );

    ret |= lt7381_reg_write(
      panel,
      LT7381_REGISTER_TCMPB1_LOW,
      LT7381_PWM_FULL_SCALE
    );

    ret |= lt7381_reg_write(
      panel,
      LT7381_REGISTER_TCNTB1_LOW,
      LT7381_PWM_FULL_SCALE
    );

    ret |= lt7381_reg_write(
      panel,
      LT7381_REGISTER_PCFGR,
      LT7381_PWM_CFG_DEFAULT
    );

    return ret;
}

static esp_err_t lt7381_wait_sdram_ready(esp_lcd_panel_t *panel)
{
  esp_err_t ret = ESP_OK;
  uint8_t status;
  uint64_t start = esp_timer_get_time();
  uint64_t now;

  do
  {
    ret = lt7381_status_read(panel, &status);
    if (ret != ESP_OK)
    {
      return ret;
    }
    
    now = esp_timer_get_time();
    if ((now - start) > LT7381_TIMEOUT_US)
    {
      ESP_LOGE(PRINT_TAG, "lt7381 Timeout!");
      return ESP_ERR_TIMEOUT;
    }
  }
  while (GET_BIT(status, STAT_REG_DISPLAY_RAM_READY_BIT) == 0u);

  return ESP_OK;
}

static esp_err_t lt7381_sdram_init(esp_lcd_panel_t *panel)
{
  esp_err_t ret = ESP_OK;
  uint16_t sdram_itv;

  /* SDRAM configuration */
  ret |= lt7381_reg_write(
    panel,
    LT7381_REGISTER_SDRAR,
    LT7381_SDRAR_DEFAULT
  );

  /* CAS latency configuration */
  ret |= lt7381_reg_write(
    panel,
    LT7381_REGISTER_SDRMD,
    LT7381_SDRMD_DEFAULT
  );

  /*
  Refresh interval calculation from vendor code:
  sdram_itv = 
    MEM_REFRESH_IN_MS / 1000 /
    MEM_ROW_SIZE *
    LT7381_PLL_N_MCLK^6
  */

  sdram_itv = LT7381_SDRAM_REFRESH_TIME;

  /* refresh interval registers */
  ret |= lt7381_reg_write(
    panel,
    LT7381_REGISTER_SDR_REF_LOW,
    (uint8_t)(sdram_itv & 0xFF)
  );
  ret |= lt7381_reg_write(
    panel,
    LT7381_REGISTER_SDR_REF_HIGH,
    (uint8_t)(sdram_itv >> 8u)
  );

  /* enable SDRAM */
  ret |= lt7381_reg_write(
    panel,
    LT7381_REGISTER_SDRCR,
    0x01
  );

  /* wait until controller reports SDRAM ready */
  ret |= lt7381_wait_sdram_ready(panel);

  vTaskDelay(pdMS_TO_TICKS(1));

  return ret;
}

static esp_err_t lt7381_tft_panel_setting(esp_lcd_panel_t *panel, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(panel, LT7381_REGISTER_CCR, regdata);
  CLEAR_BIT(regdata, CCR_REG_PANEL_IF_SETTING_LOW);
  CLEAR_BIT(regdata, CCR_REG_PANEL_IF_SETTING_HIGH);
  
  if (setting == CCR_REG_TFT_24_BIT)
  {}
  else if (setting == CCR_REG_TFT_18_BIT)
  {
    SET_BIT(regdata, CCR_REG_PANEL_IF_SETTING_HIGH);
  }
  else if (setting == CCR_REG_TFT_16_BIT)
  {
    SET_BIT(regdata, CCR_REG_PANEL_IF_SETTING_LOW);
  }
  else if (setting == CCR_REG_TFT_NO_BIT)
  {
    SET_BIT(regdata, CCR_REG_PANEL_IF_SETTING_LOW);
    SET_BIT(regdata, CCR_REG_PANEL_IF_SETTING_HIGH);
  }

  ret |= lt7381_data_write(panel, regdata);

  return ret;
}

static esp_err_t lt7381_bus_width_setting(esp_lcd_panel_t *panel, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(panel, LT7381_REGISTER_CCR, regdata);
  
  if (setting == CCR_REG_BUS_8_BIT)
  {
    CLEAR_BIT(regdata, CCR_REG_BUS_WIDTH_SELECT_BIT);
  }
  else if (setting == CCR_REG_BUS_16_BIT)
  {
    SET_BIT(regdata, CCR_REG_BUS_WIDTH_SELECT_BIT);
  }

  ret |= lt7381_data_write(panel, regdata);

  return ret;
}

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