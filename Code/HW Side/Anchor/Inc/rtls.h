/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTLS_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "gpio.h"
#include "stdio.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "string.h"
#include "stdbool.h"

/* Private defines -----------------------------------------------------------*/
/* Default antenna delay values for 64 MHz PRF */
#define TX_ANT_DLY 16436
#define RX_ANT_DLY 16436
/* Delay between frames, in UWB microseconds */
#define TX_TO_RX_DLY_UUS 500
#define RX_TO_TX_DLY_UUS 900
/* Receive response timeout */
#define RX_TIMEOUT_UUS 2000
/* Preamble timeout, in multiple of PAC size. */
#define PRE_TIMEOUT 150
/* Speed of light in air, in metres per second */
#define SPEED_OF_LIGHT 299702547
/* Indexes to access the fields involved in the process */
#define DES_ADD_IDX 5
#define SRC_ADD_IDX 7
#define POLL_TS_IDX 10
#define RESP_TS_IDX 14
#define LAST_TS_IDX 18
#define DIST_RE_IDX 10
#define ANC_LOC_IDX 14
#define HEADING_IDX 10
#define MSG_COM_LEN 10
/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps)
 * 1 uus = 512 / 499.2 µs and 1 µs = 499.2 * 128 dtu */
#define UUS_TO_DWT_TIME 65536

/* Exported functions prototypes ---------------------------------------------*/
void dwm_init(void);
void dwm_main_ds_twr_anchor(void);
HAL_StatusTypeDef dwm_main_ds_twr_tag(uint8_t, float, float*);

#define __RTLS_H
#endif /* __RTLS_H */
