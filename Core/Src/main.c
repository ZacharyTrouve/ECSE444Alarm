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
#include "app_bluenrg_ms.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "hts221.h"
//#include "lis3mdl.h"
//#include "lsm6dsl.h"
//#include "lps22hb.h"
//#include "stm32l4s5i_iot01_accelero.h"
//#include "stm32l4s5i_iot01_magneto.h"
//#include "stm32l4s5i_iot01_hsensor.h"
//#include "stm32l4s5i_iot01_psensor.h"
//#include "stm32l4s5i_iot01_qspi.h"
#include <string.h>
#include <stdio.h>
#include "b_l4s5i_iot01a.h"


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
I2C_HandleTypeDef hi2c2;

OSPI_HandleTypeDef hospi1;

osThreadId SensorWriterTskHandle;
osThreadId ledBlinkerHandle;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C2_Init(void);
static void MX_OCTOSPI1_Init(void);
void StartSensorWriterTracker(void const * argument);
void StartLEDblinker(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int blink_red = 0;

typedef enum {
	IDLE,
	HUMIDITY,
	PRESSURE,
	ACCELERO,
	MAGNETO,
	SUMMARY
} State;

volatile State currentState = IDLE;
volatile int8_t button_pushed;

float humidity;
float pressure;
int16_t accelero[3];
int16_t magneto[3];



void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin) {
	if (GPIO_Pin == PUSH_BUTTON_Pin) {
		button_pushed = 1;
		blink_red = !blink_red;
//		if (currentState == IDLE || currentState == MAGNETO) {
//			humidity = BSP_HSENSOR_ReadHumidity();
//			sprintf(message, "The humidity is %0.2f.\r\n", humidity);
//			uint16_t length = strlen(message);
//			HAL_UART_Transmit(&huart1, (uint8_t *) message, length, 100);
//			currentState = HUMIDITY;
//		} else if (currentState == HUMIDITY) {
//			pressure = BSP_PSENSOR_ReadPressure();
//			sprintf(message, "The pressure is %0.2f.\r\n", pressure);
//			uint16_t length = strlen(message);
//			HAL_UART_Transmit(&huart1, (uint8_t *) message, length, 100);
//			currentState = PRESSURE;
//		} else if (currentState == PRESSURE) {
//			BSP_ACCELERO_AccGetXYZ(accelero);
//			sprintf(message, "The acceleration in the x direction is %d, in the y direction is %d, in the z direction is %d.\r\n", accelero[0], accelero[1], accelero[2]);
//			uint16_t length = strlen(message);
//			HAL_UART_Transmit(&huart1, (uint8_t *) message, length, 100);
//			currentState = ACCELERO;
//		} else if (currentState == ACCELERO) {
//			BSP_MAGNETO_GetXYZ(magneto);
//			sprintf(message, "The magnetic field in the x direction is %d, in the y direction is %d, in the z direction is %d.\r\n", magneto[0], magneto[1], magneto[2]);
//			uint16_t length = strlen(message);
//			HAL_UART_Transmit(&huart1, (uint8_t *) message, length, 100);
//			currentState = MAGNETO;
//		}
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
	printf("new code!");
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
  MX_I2C2_Init();
  MX_OCTOSPI1_Init();
  MX_BlueNRG_MS_Init();
  /* USER CODE BEGIN 2 */
//  BSP_HSENSOR_Init();
//  BSP_MAGNETO_Init();
//  BSP_ACCELERO_Init();
//  BSP_PSENSOR_Init();
//  BSP_QSPI_Init();
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of SensorWriterTsk */
  osThreadDef(SensorWriterTsk, StartSensorWriterTracker, osPriorityNormal, 0, 1600);
  SensorWriterTskHandle = osThreadCreate(osThread(SensorWriterTsk), NULL);

  /* definition and creation of ledBlinker */
  osThreadDef(ledBlinker, StartLEDblinker, osPriorityIdle, 0, 128);
  ledBlinkerHandle = osThreadCreate(osThread(ledBlinker), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
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

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x30A175AB;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief OCTOSPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_OCTOSPI1_Init(void)
{

  /* USER CODE BEGIN OCTOSPI1_Init 0 */

  /* USER CODE END OCTOSPI1_Init 0 */

  /* USER CODE BEGIN OCTOSPI1_Init 1 */

  /* USER CODE END OCTOSPI1_Init 1 */
  /* OCTOSPI1 parameter configuration*/
  hospi1.Instance = OCTOSPI1;
  hospi1.Init.FifoThreshold = 1;
  hospi1.Init.DualQuad = HAL_OSPI_DUALQUAD_DISABLE;
  hospi1.Init.MemoryType = HAL_OSPI_MEMTYPE_MICRON;
  hospi1.Init.DeviceSize = 32;
  hospi1.Init.ChipSelectHighTime = 1;
  hospi1.Init.FreeRunningClock = HAL_OSPI_FREERUNCLK_DISABLE;
  hospi1.Init.ClockMode = HAL_OSPI_CLOCK_MODE_0;
  hospi1.Init.ClockPrescaler = 1;
  hospi1.Init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_NONE;
  hospi1.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE;
  hospi1.Init.ChipSelectBoundary = 0;
  hospi1.Init.DelayBlockBypass = HAL_OSPI_DELAY_BLOCK_BYPASSED;
  if (HAL_OSPI_Init(&hospi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OCTOSPI1_Init 2 */

  /* USER CODE END OCTOSPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LEDred_GPIO_Port, LEDred_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin : LEDred_Pin */
  GPIO_InitStruct.Pin = LEDred_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LEDred_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PE6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PUSH_BUTTON_Pin */
  GPIO_InitStruct.Pin = PUSH_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PUSH_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PE10 PE11 PE12 PE13
                           PE14 PE15 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OCTOSPIM_P1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PD13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, odd);
    if (blink_red) HAL_GPIO_WritePin(LEDred_GPIO_Port, LEDred_Pin, odd);

//    char msg[100];
//    sprintf(msg, "TEST TES TEST\r\n");
//    uint16_t length = strlen(msg);
//        HAL_UART_Transmit(&huart1, (uint8_t *) msg, length, 100);
//    if (flash_running) HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
//    else HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
  }
  /* USER CODE END StartLEDblinker */
}

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
  HAL_GPIO_WritePin(LEDred_GPIO_Port, LEDred_Pin, GPIO_PIN_RESET);
  printf("ERROR ERROR\r\n")''
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
