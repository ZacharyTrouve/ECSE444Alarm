/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "dac.h"
#include "dfsdm.h"
#include "dma.h"
#include "i2c.h"
#include "octospi.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define ARM_MATH_CM4
#include "app_bluenrg_ms.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_it.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "arm_math.h"
#include <stdbool.h>
#include <stdio.h>
#include "b_l4s5i_iot01a.h"
#include <string.h>
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
#define BL_BUFFER_SIZE  (128*8)
int32_t s_baselineBuffer[BL_BUFFER_SIZE];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int blink_red = 0;
volatile int8_t button_pushed = 0;
volatile int8_t done_calibration = 0;

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin)
{
  if (GPIO_Pin == PUSH_BUTTON_Pin)
  {
    button_pushed = 1;
    blink_red = !blink_red;
  }
}

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
  MX_DMA_Init();
  MX_DFSDM1_Init();
  MX_I2C2_Init();
  MX_DAC1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_OCTOSPI1_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
//  MX_BlueNRG_MS_Init();

//  calibrate env sound
  if (HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter0,
		  	  	  	  	  	  	  	   s_baselineBuffer,
									   BL_BUFFER_SIZE) != HAL_OK)
  {
	  Error_Handler();
  }
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();
  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* USER CODE BEGIN Header_StartSensorWriterTracker */
int16_t flash_buffer_r[2048];
int16_t flash_running = 0;
uint32_t samples_taken = 0;
char message[384];

volatile int blockindex = 4096;
volatile int block = -1;

float means[8];
/**
  * @brief  Function implementing the SensorWriterTsk thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartSensorWriterTracker */
void StartSensorWriterTracker(void const * argument)
{
  /* USER CODE BEGIN 5 */
//	MX_BlueNRG_MS_Process();
//	sprintf(message, "Setup complete!?\r\n");
//		 length1 = strlen(message);
//		    HAL_UART_Transmit(&huart1, (uint8_t *) message, length1, 100);
  /* Infinite loop */
	Ext_User_Init();
  for(;;)
  {
	  osDelay(125);
//		sprintf(message, "Calling MX_BlueNRG_MS_Process\r\n");
//		uint16_t length1 = strlen(message);
//		    HAL_UART_Transmit(&huart1, (uint8_t *) message, length1, 100);
    MX_BlueNRG_MS_Process();
    continue;
//    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
//    if (currentState == HUMIDITY) sprintf(message, "The humidity is %0.2f.\r\n", humidity);
//    else if (currentState == PRESSURE) sprintf(message, "The pressure is %0.2f.\r\n", pressure);
//    else if (currentState == ACCELERO) sprintf(message, "The acceleration in the x direction is %d, in the y direction is %d, in the z direction is %d.\r\n", accelero[0], accelero[1], accelero[2]);
//    else if (currentState == MAGNETO) sprintf(message, "The magnetic field in the x direction is %d, in the y direction is %d, in the z direction is %d.\r\n", magneto[0], magneto[1], magneto[2]);
//    else if (currentState == SUMMARY)  {
//    	double variances[8];
//    	for (int i = 0; i < 8; i++) variances[i] = 0;
//    	flash_running = 1;
//    	for (int i = 0; i <= block; i++) {
//    		//if (BSP_QSPI_Read((uint8_t *) flash_buffer_r, 0, 4096) != QSPI_OK) Error_Handler();
//    		int end_index = 2048;
//    		if (i == block) end_index = blockindex/2;
//    		for (int j = 0; j < end_index; j ++) {
//    			double diff = ((double)flash_buffer_r[j] - means[j % 8]);
//    			variances[j % 8] += diff * diff / samples_taken;
//    		}
//    	}
//    	flash_running = 0;
//    	blockindex = 4096;
//    	block = -1; //trigger reset on next read.
//    	sprintf(message, "Summary (%d samples):\r\n\tHumidity was avg=%4.2f, var=%4.2f.\r\n\tPressure was avg=%4.2f, var=%4.2f.\r\n\tAcceleration was \r\n\t\tx: avg=%4.2f, var=%4.2f\r\n\t\ty: avg=%4.2f, var=%4.2f\r\n\t\tz: avg=%4.2f, var=%4.2f\r\n\tMagnetic Field was \r\n\t\tx: avg=%4.2f, var=%4.2f\r\n\t\ty: avg=%4.2f, var=%.2f\r\n\t\tz: avg=%4.2f, var=%.2f\r\n\r\n",
//    			(int)samples_taken, means[0], variances[0], means[1], variances[1], means[2], variances[2], means[3], variances[3], means[4], variances[4], means[5], variances[5], means[6], variances[6], means[7], variances[7]);
//
//    	samples_taken = 0;
//    	for (int i = 0; i < 8; i++) means[i] = 0;
//    	currentState = HUMIDITY;
//
//    }
//    uint16_t length = strlen(message);
//    HAL_UART_Transmit(&huart1, (uint8_t *) message, length, 100);
    //    osDelay(500);
//    if (blockindex >= 4096) {
//    	blockindex = 0;
//    	block++;
//    	flash_running = 1;
//    	BSP_QSPI_Erase_Block(block); //prepare it
//    	flash_running = 0;
//    }
//    BSP_QSPI_Write((uint8_t *)flash_buffer_w, blockindex + block * 4096, 16);

  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartLEDblinker */
int odd = 0;
void toggleLed () {
	blink_red = !blink_red;
}
/**
* @brief Function implementing the ledBlinker thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLEDblinker */
void StartLEDblinker(void const * argument)
{
  /* USER CODE BEGIN StartLEDblinker */
  /* Infinite loop */
  for(;;)
  {
    osDelay(175);
    odd = !odd;
    HAL_GPIO_WritePin(green_LED_GPIO_Port, green_LED_Pin, odd);
    if (blink_red) HAL_GPIO_WritePin(Error_LED_GPIO_Port, Error_LED_Pin, odd);

//    char msg[100];
//    sprintf(msg, "TEST TES TEST\r\n");
//    uint16_t length = strlen(msg);
//        HAL_UART_Transmit(&huart1, (uint8_t *) msg, length, 100);
//    if (flash_running) HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
//    else HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
  }
  /* USER CODE END StartLEDblinker */
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
