/* Private includes ----------------------------------------------------------*/
#include "rtls.h"

/* Private variables ---------------------------------------------------------*/
static dwt_config_t config = {
    2,               /* Channel number. */
    DWT_PRF_64M,     /* Pulse repetition frequency. */
    DWT_PLEN_128,    /* Preamble length. Used in TX only. */
    DWT_PAC8,        /* Preamble acquisition chunk size. Used in RX only. */
    9,               /* TX preamble code. Used in TX only. */
    9,               /* RX preamble code. Used in RX only. */
    0,               /* 0 to use standard SFD, 1 to use non-standard SFD. */
    DWT_BR_6M8,      /* Data rate. */
    DWT_PHRMODE_STD, /* PHY header mode. */
    (129 + 8 - 8)    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
};

uint8_t poll_msg[] = { 0x41, 0x88, 0, 0xCA, 0xDE, 'A', '0', 'T', '0', 0xE0, 0, 0, 0, 0, 0, 0 };
uint8_t resp_msg[] = { 0x41, 0x88, 0, 0xCA, 0xDE, 'T', '0', 'A', '0', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t last_msg[] = { 0x41, 0x88, 0, 0xCA, 0xDE, 'A', '0', 'T', '0', 0xE2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* Private functions ---------------------------------------------------------*/
void deca_sleep(unsigned int time_ms)
{
  HAL_Delay(time_ms);
}

decaIrqStatus_t decamutexon(void)
{
  return 0;
}

void decamutexoff(decaIrqStatus_t s)
{
  UNUSED(s);
}

int writetospi(uint16 headerLength, const uint8 *headerBuffer, uint32 bodylength, const uint8 *bodyBuffer)
{
  decaIrqStatus_t stat;
  stat = decamutexon();
  while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
  HAL_GPIO_WritePin(DW_CS_GPIO_Port, DW_CS_Pin, GPIO_PIN_RESET); /* Put chip select line low */
  HAL_SPI_Transmit(&hspi1, (uint8_t *)&headerBuffer[0], headerLength, HAL_MAX_DELAY); /* Send header in polling mode */
  HAL_SPI_Transmit(&hspi1, (uint8_t *)&bodyBuffer[0], bodylength, HAL_MAX_DELAY); /* Send data in polling mode */
  HAL_GPIO_WritePin(DW_CS_GPIO_Port, DW_CS_Pin, GPIO_PIN_SET); /* Put chip select line high */
  decamutexoff(stat);
  return 0;
}

int readfromspi(uint16 headerLength, const uint8 *headerBuffer, uint32 readlength, uint8 *readBuffer)
{
  decaIrqStatus_t stat;
  stat = decamutexon();
  /* Blocking: Check whether previous transfer has been finished */
  while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
  HAL_GPIO_WritePin(DW_CS_GPIO_Port, DW_CS_Pin, GPIO_PIN_RESET); /* Put chip select line low */
  /* Send header */
  for (int i = 0; i < headerLength; i++)
    HAL_SPI_Transmit(&hspi1, (uint8_t *)&headerBuffer[i], 1, HAL_MAX_DELAY); //No timeout
  /* for the data buffer use LL functions directly as the HAL SPI read function
   * has issue reading single bytes */
  while (readlength-- > 0)
  {
    /* Wait until TXE flag is set to send data */
    while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_TXE) == RESET);
    hspi1.Instance->DR = 0; /* set output to 0 (MOSI), this is necessary for
    e.g. when waking up DW1000 from DEEPSLEEP via dwt_spicswakeup() function. */
    /* Wait until RXNE flag is set to read data */
    while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_RXNE) == RESET);
    (*readBuffer++) = hspi1.Instance->DR;  /* copy data read form (MISO) */
  }
  HAL_GPIO_WritePin(DW_CS_GPIO_Port, DW_CS_Pin, GPIO_PIN_SET); /* Put chip select line high */
  decamutexoff(stat);
  return 0;
}

void dwm_reset(void)
{
  HAL_GPIO_WritePin(DW_RST_GPIO_Port, DW_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(DW_RST_GPIO_Port, DW_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(2);
}

static void dwm_set_msg_ts(uint8_t *ts_field, uint32_t ts)
{
  for (int i = 0; i < 4; i++)
    ts_field[i] = (uint8_t)(ts >> (i * 8));
}

static void dwm_get_msg_ts(uint8_t *ts_field, uint32_t *ts)
{
  *ts = 0;
  for (int i = 0; i < 4; i++)
    *ts += ts_field[i] << (i * 8);
}

static uint64_t dwm_get_rx_ts_u64(void)
{
  uint8_t ts_tab[5];
  uint64_t ts = 0;
  dwt_readrxtimestamp(ts_tab);
  for (int i = 4; i >= 0; i--)
  {
    ts <<= 8;
    ts |= ts_tab[i];
  }
  return ts;
}

static uint64_t dwm_get_tx_ts_u64(void)
{
  uint8_t ts_tab[5];
  uint64_t ts = 0;
  dwt_readtxtimestamp(ts_tab);
  for (int i = 4; i >= 0; i--)
  {
      ts <<= 8;
      ts |= ts_tab[i];
  }
  return ts;
}

/* Public functions ----------------------------------------------------------*/
void dwm_init(void)
{
  /* Reset DW1000 */
  dwm_reset();
  /* DW1000 clocks temporarily set to crystal speed for initialisation */
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  HAL_SPI_Init(&hspi1);
  /* Initialise DW1000 */
  if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR) // DWT_LOADUCODE DWT_LOADNONE
  {
    printf("INIT FAILED\r\n");
    Error_Handler();
  }
  /* SPI rate increased for optimum performance */
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  HAL_SPI_Init(&hspi1);
  /* Print device ID for device check */
  printf(dwt_readdevid() == DWT_DEVICE_ID ? "Device Checked\r\n" : "Unsupported Device ID");
  /* Configure DW1000 */
  dwt_configure(&config);
  /* Apply default antenna delay value */
  dwt_setrxantennadelay(RX_ANT_DLY);
  dwt_settxantennadelay(TX_ANT_DLY);
  /* Set preamble timeout for expected frames */
  dwt_setpreambledetecttimeout(PRE_TIMEOUT);
}

HAL_StatusTypeDef dwm_tx(uint8_t *pData, uint16_t Size, bool Ranging)
{
  int ret;
  dwt_writetxdata(Size, pData, 0);
  dwt_writetxfctrl(Size, 0, Ranging);
  if (Ranging)
    ret = dwt_starttx(DWT_START_TX_DELAYED);
  else
    ret = dwt_starttx(DWT_START_TX_IMMEDIATE);
  if (ret == DWT_SUCCESS)
  {
    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS));
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
    return HAL_OK;
  }
  else
  {
    return HAL_ERROR;
  }
}

HAL_StatusTypeDef dwm_rx(uint8_t *pData, uint16_t *Size)
{
  uint32_t status_reg = 0;
  dwt_setrxtimeout(0);
  dwt_rxenable(DWT_START_RX_IMMEDIATE);
  while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));
  if (status_reg & SYS_STATUS_RXFCG)
  {
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);
    *Size = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
    dwt_readrxdata(pData, *Size, 0);
    return HAL_OK;
  }
  else
  {
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
    return HAL_ERROR;
  }
}

HAL_StatusTypeDef dwm_tx_rx(uint8_t *pTxData, uint16_t TxSize, uint8_t *pRxData, uint16_t *RxSize, bool Ranging, bool Delayed)
{
  int ret;
  dwt_setrxaftertxdelay(TX_TO_RX_DLY_UUS);
  dwt_setrxtimeout(RX_TIMEOUT_UUS);
  uint32_t status_reg = 0;
  dwt_writetxdata(TxSize, pTxData, 0);
  dwt_writetxfctrl(TxSize, 0, Ranging);
  if (Delayed)
    ret = dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);
  else
    ret = dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
  if (ret == DWT_ERROR)
    return HAL_ERROR;
  while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));
  if (status_reg & SYS_STATUS_RXFCG)
  {
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);
    *RxSize = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
    dwt_readrxdata(pRxData, *RxSize, 0);
    return HAL_OK;
  }
  else
  {
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR | SYS_STATUS_TXFRS);
    dwt_rxreset();
    return HAL_ERROR;
  }
}

/* ################################ Examples ################################ */
void dwm_main_tx(void)
{
  while(1)
  {
    dwm_tx(poll_msg, sizeof(poll_msg), false);
    HAL_Delay(1000);
  }
}

void dwm_main_rx(void)
{
  uint8_t rx_buf[127]; uint16_t len;
  while (1)
  {
    if (dwm_rx(rx_buf, &len) == HAL_OK)
      printf("%.*s\r\n", len - 2, rx_buf);
  }
}

void dwm_main_txwr(void)
{
  uint8_t rx_buf[127]; uint16_t len;
  while (1)
  {
    memset(rx_buf, 0, sizeof(rx_buf));
    if (dwm_tx_rx(poll_msg, sizeof(poll_msg), rx_buf, &len, false, false) == HAL_OK)
      printf("%.*s\r\n", len - 2, rx_buf);
    HAL_Delay(500);
  }
}

void dwm_main_echo(void)
{
  uint8_t rx_buf[127]; uint16_t len;
  while (1)
  {
    if (dwm_rx(rx_buf, &len) == HAL_OK)
      dwm_tx(rx_buf, len, false);
  }
}

int dwm_main_ssr_tag(void)
{
  uint8_t rx_buf[127]; uint16_t len;
  while (1)
  {
    if (dwm_tx_rx(poll_msg, sizeof(poll_msg), rx_buf, &len, true, false) == HAL_OK)
    {
      if (memcmp(rx_buf, resp_msg, MSG_COM_LEN) == 0)
      {
        uint32_t poll_rx_ts, resp_tx_ts;
        uint32_t poll_tx_ts = dwt_readtxtimestamplo32();
        uint32_t resp_rx_ts = dwt_readrxtimestamplo32();
        dwm_get_msg_ts(&rx_buf[POLL_TS_IDX], &poll_rx_ts);
        dwm_get_msg_ts(&rx_buf[RESP_TS_IDX], &resp_tx_ts);
        int32_t rtd_init = resp_rx_ts - poll_tx_ts;
        int32_t rtd_resp = resp_tx_ts - poll_rx_ts;
        double clockOffsetRatio = dwt_readcarrierintegrator() * (FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_2 / 1.0e6);
        double tof = ((rtd_init - rtd_resp * (1.0f - clockOffsetRatio)) / 2.0f) * DWT_TIME_UNITS;
        double distance = tof * SPEED_OF_LIGHT;
        printf("%0.2f\r\n", distance);
      }
    }
    HAL_Delay(10);
  }
}

int dwm_main_ssr_anchor(void)
{
  uint8_t rx_buf[127]; uint16_t len;
  while (1)
  {
    if (dwm_rx(rx_buf, &len) == HAL_OK)
    {
      if (memcmp(rx_buf, poll_msg, MSG_COM_LEN) == 0)
      {
        uint64_t poll_rx_ts = dwm_get_rx_ts_u64();
        uint64_t resp_tx_time = poll_rx_ts + RX_TO_TX_DLY_UUS * UUS_TO_DWT_TIME;
        dwt_setdelayedtrxtime(resp_tx_time >> 8);
        uint64_t resp_tx_ts = (resp_tx_time & 0xFFFFFFFE00UL) + TX_ANT_DLY;
        dwm_set_msg_ts(&resp_msg[POLL_TS_IDX], poll_rx_ts);
        dwm_set_msg_ts(&resp_msg[RESP_TS_IDX], resp_tx_ts);
        dwm_tx(resp_msg, sizeof(resp_msg), true);
      }
    }
  }
}

HAL_StatusTypeDef dwm_main_ds_twr_tag(uint8_t dest_addr, float heading, float *distance)
{
  /* setup messages */
  *(float *)&poll_msg[HEADING_IDX] = heading;
  poll_msg[DES_ADD_IDX + 1] = dest_addr;
  resp_msg[SRC_ADD_IDX + 1] = dest_addr;
  last_msg[DES_ADD_IDX + 1] = dest_addr;

  /* initiate ranging */
  uint8_t rx_buf[30]; uint16_t len;
  if (dwm_tx_rx(poll_msg, sizeof(poll_msg), rx_buf, &len, true, false) == HAL_OK)
  {
    if (memcmp(rx_buf, resp_msg, MSG_COM_LEN) == 0)
    {
      uint64_t poll_tx_ts = dwm_get_tx_ts_u64();
      uint64_t resp_rx_ts = dwm_get_rx_ts_u64();

      uint64_t last_tx_time = resp_rx_ts + RX_TO_TX_DLY_UUS * UUS_TO_DWT_TIME;
      dwt_setdelayedtrxtime(last_tx_time >> 8);
      uint64_t last_tx_ts = (last_tx_time & 0xFFFFFFFE00UL) + TX_ANT_DLY;

      dwm_set_msg_ts(&last_msg[POLL_TS_IDX], poll_tx_ts);
      dwm_set_msg_ts(&last_msg[RESP_TS_IDX], resp_rx_ts);
      dwm_set_msg_ts(&last_msg[LAST_TS_IDX], last_tx_ts);
      
      dwm_tx(last_msg, sizeof(last_msg), true);
      
//      float distance = ((float*)&rx_buf[DIST_RE_IDX])[0];
      memcpy(distance, &rx_buf[DIST_RE_IDX], 4);
      return HAL_OK;
    }
  }
  return HAL_ERROR;
}

void dwm_main_ds_twr_anchor(void)
{
  uint8_t rx_buf[127]; uint16_t len; double distance;
  while (1)
  {
    if(dwm_rx(rx_buf, &len) == HAL_OK)
    {
      if (memcmp(rx_buf, poll_msg, MSG_COM_LEN) == 0)
      {
        uint64_t poll_rx_ts = dwm_get_rx_ts_u64();
        uint64_t resp_tx_time = poll_rx_ts + RX_TO_TX_DLY_UUS * UUS_TO_DWT_TIME;
        dwt_setdelayedtrxtime(resp_tx_time >> 8);
        ((float*)&resp_msg[DIST_RE_IDX])[0] = (float)distance;
//        ((uint32_t*)&resp_msg[ANC_LOC_IDX])[0] = *(uint32_t*)LOCATION_ADDR;
        if (dwm_tx_rx(resp_msg, sizeof(resp_msg), rx_buf, &len, true, true) == HAL_OK)
        {
          if (memcmp(rx_buf, last_msg, MSG_COM_LEN) == 0)
          {
            uint32_t poll_tx_ts, resp_rx_ts, last_tx_ts;
            uint32_t poll_rx_ts_32, resp_tx_ts_32, last_rx_ts_32;

            uint64_t resp_tx_ts = dwm_get_tx_ts_u64();
            uint64_t last_rx_ts = dwm_get_rx_ts_u64();

            dwm_get_msg_ts(&rx_buf[POLL_TS_IDX], &poll_tx_ts);
            dwm_get_msg_ts(&rx_buf[RESP_TS_IDX], &resp_rx_ts);
            dwm_get_msg_ts(&rx_buf[LAST_TS_IDX], &last_tx_ts);

            poll_rx_ts_32 = poll_rx_ts;
            resp_tx_ts_32 = resp_tx_ts;
            last_rx_ts_32 = last_rx_ts;
            double Ra = resp_rx_ts - poll_tx_ts;
            double Rb = last_rx_ts_32 - resp_tx_ts_32;
            double Da = last_tx_ts - resp_rx_ts;
            double Db = resp_tx_ts_32 - poll_rx_ts_32;
            int64_t tof_dtu = (Ra * Rb - Da * Db) / (Ra + Rb + Da + Db);

            double tof = tof_dtu * DWT_TIME_UNITS;
            distance = tof * SPEED_OF_LIGHT;
          }
        }
      }
    }
  }
}
