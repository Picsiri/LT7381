#include "lt7381.h"
#include "lt7381_Internals.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* low-level helpers                                                  */
/* ------------------------------------------------------------------ */

esp_err_t lt7381_wait(lt7381_panel_t *lt)
{
  esp_err_t ret = ESP_OK;
  uint64_t  start = esp_timer_get_time();
  uint64_t  now = start;
  bool      passed = false;
  uint8_t   status;

  while ((now - start) <= LT7381_TIMEOUT_US && !passed)
  {
    now = esp_timer_get_time();

    if (lt->wait_gpio_num == GPIO_NUM_NC)
    {
      lt7381_status_read(lt, &status);

      if (GET_BIT(status, STAT_REG_WRITE_MEMORY_FULL_BIT) == 0u)
      {
        passed = true;
      }
    }
    else
    {
      if (gpio_get_level(lt->wait_gpio_num) == lt->wait_level)
      {
        passed = true;
      }
    }
  }
  
  if (!passed)
  {
    ESP_LOGE(PRINT_TAG, "lt7381_wait timeout!");
    ret = ESP_ERR_TIMEOUT;
  }

  return ret;
}

esp_err_t lt7381_cmd_write(lt7381_panel_t *lt, uint8_t cmd)
{
  uint8_t packet[2];
  packet[0] = LT7381_CMD_WRITE_COMMAND;
  packet[1] = cmd;
  
  return esp_lcd_panel_io_tx_param(
    lt->io_handle,
    -1,
    packet,
    2
  );
}

esp_err_t lt7381_data_write(lt7381_panel_t *lt, uint8_t data)
{
  uint8_t packet[2];
  packet[0] = LT7381_CMD_WRITE_DATA;
  packet[1] = data;

  return esp_lcd_panel_io_tx_param(
    lt->io_handle,
    -1,
    packet,
    2
  );
}

esp_err_t lt7381_batch_write(lt7381_panel_t *lt, uint8_t *batch, uint8_t size)
{
  batch[0] = LT7381_CMD_WRITE_DATA;

  // TODO: think about if DMA in would have any benefit?
  //     esp_lcd_panel_io_tx_color()
  return esp_lcd_panel_io_tx_param(
    lt->io_handle,
    -1,
    batch,
    size
  );
}

esp_err_t lt7381_data_read(lt7381_panel_t *lt, uint8_t *data)
{
  return esp_lcd_panel_io_rx_param(
    lt->io_handle,
    LT7381_CMD_READ_DATA,
    data,
    1
  );
}

esp_err_t lt7381_status_read(lt7381_panel_t *lt, uint8_t *status)
{
  return esp_lcd_panel_io_rx_param(
    lt->io_handle,
    LT7381_CMD_READ_STATUS,
    status,
    1
  );
}

/* ------------------------------------------------------------------ */
/* register manipulators                                              */
/* ------------------------------------------------------------------ */

esp_err_t lt7381_reg_write(lt7381_panel_t *lt, uint8_t reg, uint8_t val)
{
  esp_err_t ret;

  ret = lt7381_cmd_write(lt, reg);
  if (ret != ESP_OK) return ret;

  return lt7381_data_write(lt, val);
}

esp_err_t lt7381_reg_read(lt7381_panel_t *lt, uint8_t reg, uint8_t *val)
{
  esp_err_t ret;

  ret = lt7381_cmd_write(lt, reg);
  if (ret != ESP_OK) return ret;

  ret = lt7381_data_read(lt, val);

  return ret;
}

esp_err_t lt7381_system_wait_ready(lt7381_panel_t *lt)
{
  esp_err_t ret;
  uint8_t status;
  uint8_t ccr;
  uint8_t retry = 0u;
  uint8_t fail = 0u;

  while (1)
  {
    ret |= lt7381_status_read(lt, &status);

    if (GET_BIT(status, STAT_REG_INHIBIT_OPERATION_BIT) == 0u)
    {
      ESP_LOGI(PRINT_TAG, "Status is %d", status);
      /* one might think lt7381_reg_read() can be used but these commands are slow and take time, delays are needed */
      vTaskDelay(pdMS_TO_TICKS(2));
      ret |= lt7381_cmd_write(lt, LT7381_REGISTER_CCR);
      vTaskDelay(pdMS_TO_TICKS(2));
      ret |= lt7381_data_read(lt, &ccr);
      ESP_LOGI(PRINT_TAG, "CCR is %d", ccr);

      if (GET_BIT(ccr, CCR_REG_PLL_READY_BIT) > 0u)
      {
        return ESP_OK;
      }
      else
      {
        vTaskDelay(pdMS_TO_TICKS(2));
        ret |= lt7381_cmd_write(lt, LT7381_REGISTER_CCR);
        vTaskDelay(pdMS_TO_TICKS(2));
        ret |= lt7381_data_write(lt, BIT_TO_VAL(CCR_REG_PLL_READY_BIT));
      }
    }
    else
    {
      ESP_LOGE(PRINT_TAG, "Status is %d", status);
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

  return ret;
}

esp_err_t lt7381_pll_init(lt7381_panel_t *lt)
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
      lt,
      LT7381_REGISTER_PPLLC1,
      (lpllOD_sclk << 6) |
      (lpllR_sclk  << 1) |
      ((lpllN_sclk >> 8) & 0x01)
    );

    ret |= lt7381_reg_write(
      lt,
      LT7381_REGISTER_MPLLC1,
      (lpllOD_mclk << 6) |
      (lpllR_mclk  << 1) |
      ((lpllN_mclk >> 8) & 0x01)
    );

    ret |= lt7381_reg_write(
      lt,
      LT7381_REGISTER_CPLLC1,
      (lpllOD_cclk << 6) |
      (lpllR_cclk  << 1) |
      ((lpllN_cclk >> 8) & 0x01)
    );

    /* -------- N values -------- */

    ret |= lt7381_reg_write(
      lt,
      LT7381_REGISTER_PPLLC2,
      lpllN_sclk
    );

    ret |= lt7381_reg_write(
      lt,
      LT7381_REGISTER_MPLLC2,
      lpllN_mclk
    );

    ret |= lt7381_reg_write(
      lt,
      LT7381_REGISTER_CPLLC2,
      lpllN_cclk
    );

    /* -------- reconfig PLL -------- */

    ret |= lt7381_cmd_write(lt, LT7381_REGISTER_SRR);
    ets_delay_us(1);
    ret |= lt7381_data_write(lt, 0x80);
    vTaskDelay(pdMS_TO_TICKS(1));

    /* -------- default PWM setup -------- */

    ret |= lt7381_reg_write(
      lt,
      LT7381_REGISTER_PMUXR,
      LT7381_PWM_MUX_DEFAULT
    );

    ret |= lt7381_reg_write(
      lt,
      LT7381_REGISTER_TCMPB0_LOW,
      LT7381_PWM_FULL_SCALE
    );

    ret |= lt7381_reg_write(
      lt,
      LT7381_REGISTER_TCNTB0_LOW,
      LT7381_PWM_FULL_SCALE
    );

    ret |= lt7381_reg_write(
      lt,
      LT7381_REGISTER_TCMPB1_LOW,
      LT7381_PWM_FULL_SCALE
    );

    ret |= lt7381_reg_write(
      lt,
      LT7381_REGISTER_TCNTB1_LOW,
      LT7381_PWM_FULL_SCALE
    );

    ret |= lt7381_reg_write(
      lt,
      LT7381_REGISTER_PCFGR,
      LT7381_PWM_CFG_DEFAULT
    );

    return ret;
}

esp_err_t lt7381_wait_sdram_ready(lt7381_panel_t *lt)
{
  esp_err_t ret = ESP_OK;
  uint8_t status;
  uint64_t start = esp_timer_get_time();
  uint64_t now;

  do
  {
    ret = lt7381_status_read(lt, &status);
    if (ret != ESP_OK)
    {
      return ret;
    }
    
    now = esp_timer_get_time();
    if ((now - start) > LT7381_TIMEOUT_US)
    {
      ESP_LOGE(PRINT_TAG, "lt7381_wait_sdram timeout!");
      return ESP_ERR_TIMEOUT;
    }
  }
  while (GET_BIT(status, STAT_REG_DISPLAY_RAM_READY_BIT) == 0u);

  return ESP_OK;
}

esp_err_t lt7381_SDRAM_init(lt7381_panel_t *lt)
{
  esp_err_t ret = ESP_OK;
  uint16_t sdram_itv;

  /* SDRAM configuration */
  ret |= lt7381_reg_write(
    lt,
    LT7381_REGISTER_SDRAR,
    LT7381_SDRAR_DEFAULT
  );

  /* CAS latency configuration */
  ret |= lt7381_reg_write(
    lt,
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
    lt,
    LT7381_REGISTER_SDR_REF_LOW,
    (uint8_t)(sdram_itv & 0xFF)
  );
  ret |= lt7381_reg_write(
    lt,
    LT7381_REGISTER_SDR_REF_HIGH,
    (uint8_t)(sdram_itv >> 8u)
  );

  /* enable SDRAM */
  ret |= lt7381_reg_write(
    lt,
    LT7381_REGISTER_SDRCR,
    0x01
  );

  /* wait until controller reports SDRAM ready */
  ret |= lt7381_wait_sdram_ready(lt);

  vTaskDelay(pdMS_TO_TICKS(1));

  return ret;
}

esp_err_t lt7381_tft_panel_setting(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_CCR, &regdata);
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

  ret |= lt7381_data_write(lt, regdata);

  lt7381_data_read(lt, &regdata);

  return ret;
}

esp_err_t lt7381_bus_width_setting(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_CCR, &regdata);
  
  if (setting == CCR_REG_BUS_8_BIT)
  {
    CLEAR_BIT(regdata, CCR_REG_BUS_WIDTH_SELECT_BIT);
  }
  else if (setting == CCR_REG_BUS_16_BIT)
  {
    SET_BIT(regdata, CCR_REG_BUS_WIDTH_SELECT_BIT);
  }

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_image_data_format(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_MACR, &regdata);
  CLEAR_BIT(regdata, 7);
  CLEAR_BIT(regdata, 6);

  regdata |= setting << 6;

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_memwrite_directions_setting(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_MACR, &regdata);
  CLEAR_BIT(regdata, 5);
  CLEAR_BIT(regdata, 4);
  CLEAR_BIT(regdata, 2);
  CLEAR_BIT(regdata, 1);

  regdata |= setting << 4;
  regdata |= setting << 1;

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_graphic_or_text_mode(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_ICR, &regdata);

  if (setting == ICR_REG_GRAPHIC_MODE)
  {
    CLEAR_BIT(regdata, 2);
  }
  else
  {
    SET_BIT(regdata, 2);
  }

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_memory_select(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_ICR, &regdata);

  CLEAR_BIT(regdata, 1);
  CLEAR_BIT(regdata, 0);

  regdata |= setting << 0;

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_draw_pixel(lt7381_panel_t *lt, uint16_t color)
{
  esp_err_t ret = ESP_OK;

  ret |= lt7381_cmd_write(lt, LT7381_REGISTER_MRWDP);

  ret |= lt7381_wait(lt);
  ret |= lt7381_data_write(lt, color);
  ret |= lt7381_wait(lt);
  ret |= lt7381_data_write(lt, color >> 8);

  return ret;
}

esp_err_t lt7381_draw_picture(lt7381_panel_t *lt, const uint8_t* color, uint32_t len)
{
  esp_err_t ret = ESP_OK;

  ret |= lt7381_cmd_write(lt, LT7381_REGISTER_MRWDP);
  uint8_t data[LT7381_IMAGE_STREAM_CHUNK_SIZE + 1];

  for (uint32_t i = 0u; i < len; i += LT7381_IMAGE_STREAM_CHUNK_SIZE)
  {
    /* leave first byte empty for header */
    uint32_t chunk = len - i;
    if (chunk > LT7381_IMAGE_STREAM_CHUNK_SIZE) {
        chunk = LT7381_IMAGE_STREAM_CHUNK_SIZE;
    }
    
    memcpy(&data[1], &color[i], chunk);
    
    ret |= lt7381_batch_write(lt, data, chunk + 1);
  }

  return ret;
}

esp_err_t lt7381_select_main_image_color_depth(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_MPWCTR, &regdata);

  CLEAR_BIT(regdata, 3);
  CLEAR_BIT(regdata, 2);

  regdata |= setting << 2;

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_display_on_off(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_DPCR, &regdata);

  if (setting == DPCR_REG_DISPLAY_OFF)
  {
    CLEAR_BIT(regdata, DPCR_REG_PCLK_DISPLAY_ON_BIT);
  }
  else
  {
    SET_BIT(regdata, DPCR_REG_PCLK_DISPLAY_ON_BIT);
  }

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_display_test_on_off(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_DPCR, &regdata);

  if (setting == DPCR_REG_DISPLAY_TEST_OFF)
  {
    CLEAR_BIT(regdata, DPCR_REG_PCLK_DISPLAY_TEST_BIT);
  }
  else
  {
    SET_BIT(regdata, DPCR_REG_PCLK_DISPLAY_TEST_BIT);
  }

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_display_vertical_direction(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_DPCR, &regdata);

  if (setting == DPCR_REG_TOP_TO_BOTTOM)
  {
    CLEAR_BIT(regdata, DPCR_REG_PCLK_BOT_TO_TOP_BIT);
  }
  else
  {
    SET_BIT(regdata, DPCR_REG_PCLK_BOT_TO_TOP_BIT);
  }

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_display_color_sequence(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_DPCR, &regdata);
  CLEAR_BIT(regdata, 2);
  CLEAR_BIT(regdata, 1);
  CLEAR_BIT(regdata, 0);

  regdata |= setting << 0;

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_PLCK_polarity(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_DPCR, &regdata);

  if (setting == DPCR_REG_PCLK_RISING_EDGE)
  {
    CLEAR_BIT(regdata, DPCR_REG_PCLK_INVERSION_BIT);
  }
  else
  {
    SET_BIT(regdata, DPCR_REG_PCLK_INVERSION_BIT);
  }

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_hsync_polarity(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_PCSR, &regdata);

  if (setting == PCSR_REG_HSYNC_LOW_ACTIVE)
  {
    CLEAR_BIT(regdata, PCSR_REG_HSYNC_POLARITY_BIT);
  }
  else
  {
    SET_BIT(regdata, PCSR_REG_HSYNC_POLARITY_BIT);
  }

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_pde_polarity(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_PCSR, &regdata);

  if (setting == PCSR_REG_PDE_HIGH_ACTIVE)
  {
    CLEAR_BIT(regdata, PCSR_REG_PDE_POLARITY_BIT);
  }
  else
  {
    SET_BIT(regdata, PCSR_REG_PDE_POLARITY_BIT);
  }

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_pde_idle_state(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_PCSR, &regdata);

  if (setting == PCSR_REG_PDE_IDLE_LOW)
  {
    CLEAR_BIT(regdata, PCSR_REG_PDE_IDLE_BIT);
  }
  else
  {
    SET_BIT(regdata, PCSR_REG_PDE_IDLE_BIT);
  }

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_pclk_idle_state(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_PCSR, &regdata);

  if (setting == PCSR_REG_PCLK_IDLE_LOW)
  {
    CLEAR_BIT(regdata, PCSR_REG_PCLK_IDLE_BIT);
  }
  else
  {
    SET_BIT(regdata, PCSR_REG_PCLK_IDLE_BIT);
  }

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_pd_idle_state(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_PCSR, &regdata);

  if (setting == PCSR_REG_PD_IDLE_LOW)
  {
    CLEAR_BIT(regdata, PCSR_REG_PD_IDLE_BIT);
  }
  else
  {
    SET_BIT(regdata, PCSR_REG_PD_IDLE_BIT);
  }

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_hsync_idle_state(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_PCSR, &regdata);

  if (setting == PCSR_REG_HSYNC_IDLE_LOW)
  {
    CLEAR_BIT(regdata, PCSR_REG_HSYNC_IDLE_BIT);
  }
  else
  {
    SET_BIT(regdata, PCSR_REG_HSYNC_IDLE_BIT);
  }

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_vsync_idle_state(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_PCSR, &regdata);

  if (setting == PCSR_REG_VSYNC_IDLE_LOW)
  {
    CLEAR_BIT(regdata, PCSR_REG_VSYNC_IDLE_BIT);
  }
  else
  {
    SET_BIT(regdata, PCSR_REG_VSYNC_IDLE_BIT);
  }

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_lcd_horizontal_width_vertical_height(
  lt7381_panel_t *lt,
  uint16_t WX,
  uint16_t HY)
{
  esp_err_t ret = ESP_OK;
  uint8_t temp;

  if (WX < 8)
  {
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_HDWR, 0x00);
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_HDWFTR, WX);
  }
  else
  {
    temp = (WX / 8) - 1;
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_HDWR, temp);

    temp = WX % 8;
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_HDWFTR, temp);
  }

  temp = HY - 1;
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_VDHR_LOW, temp);

  temp = (HY - 1) >> 8;
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_VDHR_HIGH, temp);

  return ret;
}

esp_err_t lt7381_lcd_horizontal_non_display(
  lt7381_panel_t *lt,
  uint16_t WX)
{
  esp_err_t ret = ESP_OK;
  uint8_t temp;

  if (WX < 8)
  {
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_HNDR, 0x00);
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_HNDFTR, WX);
  }
  else
  {
    temp = (WX / 8) - 1;
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_HNDR, temp);

    temp = WX % 8;
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_HNDFTR, temp);
  }

  return ret;
}

esp_err_t lt7381_lcd_hsync_start_position(
  lt7381_panel_t *lt,
  uint16_t WX)
{
  esp_err_t ret = ESP_OK;
  uint8_t temp;

  if (WX < 8)
  {
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_HSTR, 0x00);
  }
  else
  {
    temp = (WX / 8) - 1;
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_HSTR, temp);
  }

  return ret;
}

esp_err_t lt7381_lcd_hsync_pulse_width(
  lt7381_panel_t *lt,
  uint16_t WX)
{
  esp_err_t ret = ESP_OK;
  uint8_t temp;

  if (WX < 8)
  {
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_HPWR, 0x00);
  }
  else
  {
    temp = (WX / 8) - 1;
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_HPWR, temp);
  }

  return ret;
}

esp_err_t lt7381_lcd_vertical_non_display(
  lt7381_panel_t *lt,
  uint16_t HY)
{
  esp_err_t ret = ESP_OK;
  uint8_t temp;

  temp = HY - 1;
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_VNDR_LOW, temp);

  ret |= lt7381_reg_write(lt, LT7381_REGISTER_VNDR_HIGH, temp >> 8);

  return ret;
}

esp_err_t lt7381_lcd_vsync_start_position(
  lt7381_panel_t *lt,
  uint16_t HY)
{
  esp_err_t ret = ESP_OK;

  ret |= lt7381_reg_write(lt, LT7381_REGISTER_VSTR, (HY - 1));

  return ret;
}

esp_err_t lt7381_lcd_vsync_pulse_width(
  lt7381_panel_t *lt,
  uint16_t HY)
{
  esp_err_t ret = ESP_OK;

  ret |= lt7381_reg_write(lt, LT7381_REGISTER_VPWR, (HY - 1));

  return ret;
}

esp_err_t lt7381_main_image_start_address(lt7381_panel_t *lt, uint32_t start)
{
  esp_err_t ret = ESP_OK;

  ret |= lt7381_reg_write(lt, LT7381_REGISTER_MISA_LOW, start >> 0);
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_MISA_DOWN, start >> 8);
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_MISA_UP, start >> 16);
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_MISA_HIGH, start >> 24);

  return ret;
}

esp_err_t lt7381_main_image_width(lt7381_panel_t *lt, uint16_t width)
{
  esp_err_t ret = ESP_OK;

  if (width % 4 != 0u)
  {
    ret = ESP_ERR_INVALID_ARG;
  }
  else
  {
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_MIW_LOW, width >> 0);
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_MIW_HIGH, width >> 8);
  }

  return ret;
}

esp_err_t lt7381_main_window_start_xy(lt7381_panel_t *lt, uint16_t x, uint16_t y)
{
  esp_err_t ret = ESP_OK;

  if (x > 8191u)
  {
    ret = ESP_ERR_INVALID_ARG;
  }
  else
  {
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_MWULX_LOW, x >> 0);
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_MWULX_HIGH, x >> 8);
  }

  if (y > 8191u)
  {
    ret |= ESP_ERR_INVALID_ARG;
  }
  else
  {
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_MWULY_LOW, y >> 0);
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_MWULY_HIGH, y >> 8);
  }
  
  return ret;
}

esp_err_t lt7381_canvas_image_start_address(lt7381_panel_t *lt, uint32_t start)
{
  esp_err_t ret = ESP_OK;

  ret |= lt7381_reg_write(lt, LT7381_REGISTER_CVSSA_LOW, start >> 0);
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_CVSSA_DOWN, start >> 8);
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_CVSSA_UP, start >> 16);
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_CVSSA_HIGH, start >> 24);

  return ret;
}

esp_err_t lt7381_canvas_image_width(lt7381_panel_t *lt, uint16_t width)
{
  esp_err_t ret = ESP_OK;

  if (width > 8191u)
  {
    ret = ESP_ERR_INVALID_ARG;
  }
  else
  {
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_CVS_IMWTH_LOW, width >> 0);
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_CVS_IMWTH_HIGH, width >> 8);
  }
  
  return ret;
}

esp_err_t lt7381_active_window_xy(lt7381_panel_t *lt, uint16_t x, uint16_t y)
{
  esp_err_t ret = ESP_OK;

  if (x > 8191u)
  {
    ret = ESP_ERR_INVALID_ARG;
  }
  else
  {
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_AWUL_X_LOW, x >> 0);
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_AWUL_X_HIGH, x >> 8);
  }

  if (y > 8191u)
  {
    ret |= ESP_ERR_INVALID_ARG;
  }
  else
  {
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_AWUL_Y_LOW, y >> 0);
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_AWUL_Y_HIGH, y >> 8);
  }
  
  return ret;
}

esp_err_t lt7381_active_window_wh(lt7381_panel_t *lt, uint16_t w, uint16_t h)
{
  esp_err_t ret = ESP_OK;

  if (w > 8192u)
  {
    ret = ESP_ERR_INVALID_ARG;
  }
  else
  {
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_AW_WTH_LOW, w >> 0);
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_AW_WTH_HIGH, w >> 8);
  }

  if (h > 8192u)
  {
    ret |= ESP_ERR_INVALID_ARG;
  }
  else
  {
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_AW_HT_LOW, h >> 0);
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_AW_HT_HIGH, h >> 8);
  }
  
  return ret;
}

esp_err_t lt7381_memory_xy_mode(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_AW_COLOR, &regdata);

  if (setting == PCSR_REG_VSYNC_IDLE_LOW)
  {
    CLEAR_BIT(regdata, AW_COLOR_REG_ADDRESS_MODE_BIT);
  }
  else
  {
    SET_BIT(regdata, AW_COLOR_REG_ADDRESS_MODE_BIT);
  }

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_canvas_color_depth(lt7381_panel_t *lt, uint8_t setting)
{
  esp_err_t ret = ESP_OK;
  uint8_t regdata;

  ret |= lt7381_reg_read(lt, LT7381_REGISTER_AW_COLOR, &regdata);
  CLEAR_BIT(regdata, 1);
  CLEAR_BIT(regdata, 0);

  regdata |= setting << 0;

  ret |= lt7381_data_write(lt, regdata);

  return ret;
}

esp_err_t lt7381_cursor_xy(lt7381_panel_t *lt, uint16_t x, uint16_t y)
{
  esp_err_t ret = ESP_OK;

  if (x > 8191u)
  {
    ret = ESP_ERR_INVALID_ARG;
  }
  else
  {
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_CURH_LOW, x >> 0);
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_CURH_HIGH, x >> 8);
  }

  if (y > 8191u)
  {
    ret |= ESP_ERR_INVALID_ARG;
  }
  else
  {
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_CURV_LOW, y >> 0);
    ret |= lt7381_reg_write(lt, LT7381_REGISTER_CURV_HIGH, y >> 8);
  }
  
  return ret;
}

esp_err_t lt7381_draw_start_xy(lt7381_panel_t *lt, uint16_t wx, uint16_t hy)
{
  esp_err_t ret = ESP_OK;

  ret |= lt7381_reg_write(lt, LT7381_REGISTER_DLHSR_LOW,  wx & 0xFF);
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_DLHSR_HIGH, (wx >> 8) & 0x1F);

  ret |= lt7381_reg_write(lt, LT7381_REGISTER_DLVSR_LOW,  hy & 0xFF);
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_DLVSR_HIGH, (hy >> 8) & 0x1F);

  return ret;
}

esp_err_t lt7381_draw_end_xy(lt7381_panel_t *lt, uint16_t wx, uint16_t hy)
{
  esp_err_t ret = ESP_OK;

  ret |= lt7381_reg_write(lt, LT7381_REGISTER_DLHER_LOW,  wx & 0xFF);
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_DLHER_HIGH, (wx >> 8) & 0x1F);

  ret |= lt7381_reg_write(lt, LT7381_REGISTER_DLVER_LOW,  hy & 0xFF);
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_DLVER_HIGH, (hy >> 8) & 0x1F);

  return ret;
}

esp_err_t lt7381_start_square_fill(lt7381_panel_t *lt)
{
  esp_err_t ret = ESP_OK;

  ret |= lt7381_cmd_write(lt, LT7381_REGISTER_DCR1);
  ret |= lt7381_data_write(lt, DCR_REG_START_SQUARE_FILL);

  return ret;
}

esp_err_t lt7381_foreground_color(lt7381_panel_t *lt, uint16_t color)
{
  esp_err_t ret = ESP_OK;

  ret |= lt7381_reg_write(lt, LT7381_REGISTER_FGCR, color >> 8);
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_FGCG, color >> 3);
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_FGCB, color << 3);

  return ret;
}

esp_err_t lt7381_background_color(lt7381_panel_t *lt, uint16_t color)
{
  esp_err_t ret = ESP_OK;

  ret |= lt7381_reg_write(lt, LT7381_REGISTER_BGCR, color >> 8);
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_BGCG, color >> 3);
  ret |= lt7381_reg_write(lt, LT7381_REGISTER_BGCB, color << 3);

  return ret;
}