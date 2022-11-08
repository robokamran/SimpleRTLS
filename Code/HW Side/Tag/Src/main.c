/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "crc.h"
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "ahrs.h"
#include "math.h"
#include "motion_fx.h"
#include "rtls.h"
#include "stdbool.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, 10);
	return ch;
}

void setup(void)
{
  HAL_Delay(100);
	for (int i = 1; i < 128; i++)
  if (HAL_I2C_IsDeviceReady(&hi2c3, i << 1, 5, 100) == HAL_OK)
    printf("addr: 0x%X\r\n", i);
  bmp_init();
  imu_init();
  ahrs_init();
  dwm_init();
  printf("init done\r\n");
}

void loop(void)
{
  /* loop variables */
  static uint8_t anchor_addr = '1';
  short imu_status = 0;
  
  /* loop code */
  mpu_get_int_status(&imu_status);
  if (imu_status & MPU_INT_STATUS_DATA_READY)
  {
    /* loop time measurement */
    uint32_t start = HAL_GetTick();
    
    /* data acquisition */
    short mag[3], acc[3], gyr[3];
    mpu_get_compass_reg(mag, NULL);
    mpu_get_accel_reg(acc, NULL);
    mpu_get_gyro_reg(gyr, NULL);
    
    /* ahrs data build */
    MFX_input_t data_in;
    MFX_output_t data_out;
    for (int i = 0; i < 3; i++)
    {
      data_in.mag[i] = (mRes * mag[i]) / 50.0f;
      data_in.acc[i] = aRes * acc[i];
      data_in.gyro[i] = gRes * gyr[i];
    }
    
    /* magnetometer calibration check */
    MFX_MagCal_output_t magcal_data_out;
    MotionFX_MagCal_getParams(&magcal_data_out);

    /* run sensor fusion algorithm */
    if (magcal_data_out.cal_quality == MFX_MAGCALGOOD)
    {
      for (int i = 0; i < 3; i++)
        data_in.mag[i] = data_in.mag[i] - magcal_data_out.hi_bias[i];
      MotionFX_propagate(&data_out, &data_in, (float*)&SampleTime);
      MotionFX_update(&data_out, &data_in, (float*)&SampleTime, NULL);
      MotionFX_MagCal_init(1, 0); // disable magnetometer calibration
      HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    }
    /* run magnetometer calibration */
    else
    {
      MFX_MagCal_input_t magcal_data_in = {
        .mag = { data_in.mag[0], data_in.mag[1], data_in.mag[2] },
        .time_stamp = HAL_GetTick(),
      };
      MotionFX_MagCal_run(&magcal_data_in);
      HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
    }
    
    /* measure distance */
    float distance;
    HAL_StatusTypeDef ranging_status = dwm_main_ds_twr_tag(anchor_addr, data_out.heading_9X, &distance);
    anchor_addr = anchor_addr == '3' ? '1' : anchor_addr + 1;
    
    /* system report */
    printf("%d\t %0.2f\t %d\t %0.2f\r\n", ranging_status, distance, HAL_GetTick() - start, data_out.heading_9X);
  }
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_I2C3_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */
  setup();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    loop();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
