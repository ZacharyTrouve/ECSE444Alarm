/* USER CODE BEGIN Header */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define ARM_MATH_CM4
#include "dac.h"
#include "dfsdm.h"
#include "dma.h"
#include "i2c.h"
#include "octospi.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "stm32l4s5i_iot01_accelero.h"
#include "arm_math.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
  ALARM_EVENT_AUDIO = 0x01U,
  ALARM_EVENT_MOTION = 0x02U
} AlarmEventSource_t;

typedef struct
{
  AlarmEventSource_t source;
  uint32_t magnitude;
} AlarmEvent_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define AUDIO_FRAME_SIZE                    128U
#define AUDIO_DMA_BUFFER_SIZE               (AUDIO_FRAME_SIZE * 2U)
#define AUDIO_BUFFER_HALF_FLAG              (1UL << 0)
#define AUDIO_BUFFER_FULL_FLAG              (1UL << 1)
#define AUDIO_SAMPLE_SHIFT                  8U
#define ALARM_EVENT_QUEUE_LENGTH            4U
#define AUDIO_RETRIGGER_MS                  500U
#define AUDIO_SENSOR_STABILIZATION_MS       1500U
#define AUDIO_CALIBRATION_FRAMES            20U


#define ALARM_WAVEFORM_SAMPLES             10U
#define DAC_MAX                            4095U // 12-bit DAC
#define DAC_CENTER                         (DAC_MAX / 2U)
#define AMP                                (DAC_MAX / 2U) // Amplitude of the alarm waveform
#define MOTION_SAMPLE_PERIOD_MS            50U
#define MOTION_THRESHOLD                   4000U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

static TaskHandle_t audioProcessingTaskNativeHandle = NULL;

static QueueHandle_t alarmEventQueue;
static StaticQueue_t alarmEventQueueStruct;
static uint8_t alarmEventQueueStorage[ALARM_EVENT_QUEUE_LENGTH * sizeof(AlarmEvent_t)];

static volatile uint32_t s_audioReadyMask = 0U;
static int32_t s_audioDmaBuffer[AUDIO_DMA_BUFFER_SIZE];
static uint16_t s_alarmWaveform[ALARM_WAVEFORM_SAMPLES];
static TickType_t s_lastAudioAlarmTick = 0;
static volatile bool s_alarmOutputActive = false;
static uint32_t s_audioThreshold = 20000;
static bool s_audioCalibrationDone = false;
static uint64_t s_audioCalibrationAccum = 0;
static uint32_t s_audioCalibrationCount = 0;
// static TickType_t s_audioStabilizeUntil = 0;

/* USER CODE END Variables */
osThreadId audioProcessingHandle;
osThreadId alarmTaskHandle;
osThreadId motionTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void DispatchAlarmEvent(AlarmEventSource_t source, uint32_t magnitude);
static void AnalyzeAudioFrame(const int32_t *frame, size_t length);
static uint32_t CalculateFrameEnergy(const int32_t *frame, size_t length);
static void InitAlarmWaveform(void);
static HAL_StatusTypeDef StartAlarmOutput(void);
static void StopAlarmOutput(void);
static void DebugPrint(const char *format, ...);
void vApplicationIdleHook(void);


static uint32_t frameEnergy = 0;
static size_t audio_called_n = 0;


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    DebugPrint("Stack OVerflow");// Handle overflow
}
/* USER CODE END FunctionPrototypes */

void AudioProcessingTask(void const * argument);
void AlarmTask(void const * argument);
void MotionTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	DebugPrint("OS Init\r\n");
  alarmEventQueue = xQueueCreateStatic(ALARM_EVENT_QUEUE_LENGTH,
                                       sizeof(AlarmEvent_t),
                                       alarmEventQueueStorage,
                                       &alarmEventQueueStruct);
  configASSERT(alarmEventQueue != NULL);
  InitAlarmWaveform();
  HAL_GPIO_WritePin(green_LED_GPIO_Port, green_LED_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(Error_LED_GPIO_Port, Error_LED_Pin, GPIO_PIN_SET);
  /* USER CODE END Init */

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
  /* definition and creation of audioProcessing */
  osThreadDef(audioProcessing, AudioProcessingTask, osPriorityNormal, 0, 512);
  audioProcessingHandle = osThreadCreate(osThread(audioProcessing), NULL);

  /* definition and creation of alarmTask */
  osThreadDef(alarmTask, AlarmTask, osPriorityNormal, 0, 512);
  alarmTaskHandle = osThreadCreate(osThread(alarmTask), NULL);

  /* definition and creation of motionTask */
  osThreadDef(motionTask, MotionTask, osPriorityNormal, 0, 256);
  motionTaskHandle = osThreadCreate(osThread(motionTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_AudioProcessingTask */
/**
  * @brief  Function implementing the audioProcessing thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_AudioProcessingTask */
void AudioProcessingTask(void const * argument)
{
  /* USER CODE BEGIN AudioProcessingTask */
  DebugPrint("AudioProcessingTask begins\r\n");
  audio_called_n ++;
  (void)(argument);

  audioProcessingTaskNativeHandle = xTaskGetCurrentTaskHandle();
  s_audioCalibrationDone = false;
  s_audioCalibrationAccum = 0;
  s_audioCalibrationCount = 0;
  memset(s_audioDmaBuffer, 0, sizeof(s_audioDmaBuffer));

  if (HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter0,
                                       s_audioDmaBuffer,
                                       AUDIO_DMA_BUFFER_SIZE) != HAL_OK)
  {
    Error_Handler();
  }
  // s_audioStabilizeUntil = xTaskGetTickCount() + pdMS_TO_TICKS(AUDIO_SENSOR_STABILIZATION_MS);
//  frameEnergy = CalculateFrameEnergy(&s_audioDmaBuffer[0], AUDIO_FRAME_SIZE);
//  frameEnergy = CalculateFrameEnergy(&s_audioDmaBuffer[AUDIO_FRAME_SIZE], AUDIO_FRAME_SIZE);
//  memset(s_audioDmaBuffer, 0, sizeof(s_audioDmaBuffer));
  osDelay(AUDIO_SENSOR_STABILIZATION_MS);

  
  DebugPrint("AudioProcessingTask ready\r\n");

  for (;;)
  {
	  audio_called_n++;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    uint32_t readyMask;
    taskENTER_CRITICAL();
    readyMask = s_audioReadyMask;
    s_audioReadyMask = 0U;
    taskEXIT_CRITICAL();

    if ((readyMask & AUDIO_BUFFER_HALF_FLAG) != 0U)
    {
      AnalyzeAudioFrame(&s_audioDmaBuffer[0], AUDIO_FRAME_SIZE);
    }
    if ((readyMask & AUDIO_BUFFER_FULL_FLAG) != 0U)
    {
      AnalyzeAudioFrame(&s_audioDmaBuffer[AUDIO_FRAME_SIZE], AUDIO_FRAME_SIZE);
    }
  }
  /* USER CODE END AudioProcessingTask */
}

/* USER CODE BEGIN Header_AlarmTask */
/**
* @brief Function implementing the alarmTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_AlarmTask */
void AlarmTask(void const * argument)
{
  /* USER CODE BEGIN AlarmTask */
  (void)(argument);

  AlarmEvent_t evt;
  bool alarmRunning = false;
  DebugPrint("=========AlarmTask started========\r\n");

  for (;;)
  {
    DebugPrint("AlarmTask received evt src=%u mag=%lu\r\n", evt.source, evt.magnitude);
    if (xQueueReceive(alarmEventQueue, &evt, portMAX_DELAY) == pdPASS)
    {

      if (!alarmRunning)
      {
        HAL_StatusTypeDef startStatus = StartAlarmOutput();
        if (startStatus == HAL_OK)
        {
          alarmRunning = true;
          DebugPrint("Alarm started\r\n");
        }
        else
        {
          DebugPrint("Alarm start failed (%ld)\r\n", (long)startStatus);
        }
      }
    }
  }
  /* USER CODE END AlarmTask */
}

/* USER CODE BEGIN Header_MotionTask */
/**
* @brief Function implementing the motionTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_MotionTask */
void MotionTask(void const * argument)
{
  /* USER CODE BEGIN MotionTask */
  (void)argument;
  DebugPrint("MotionTask started\r\n");

  if (BSP_ACCELERO_Init() != ACCELERO_OK)
  {
    DebugPrint("Accel init failed\r\n");
    vTaskDelete(NULL);
  }

  BSP_ACCELERO_LowPower(0);

  int16_t prev[3] = {0};
  int16_t current[3] = {0};
  BSP_ACCELERO_AccGetXYZ(prev);


  for (;;)
  {
    vTaskDelay(pdMS_TO_TICKS(MOTION_SAMPLE_PERIOD_MS));

    BSP_ACCELERO_AccGetXYZ(current);

    int32_t diffX = (int32_t)current[0] - (int32_t)prev[0];
    int32_t diffY = (int32_t)current[1] - (int32_t)prev[1];
    int32_t diffZ = (int32_t)current[2] - (int32_t)prev[2];

    uint32_t magnitude = (uint32_t)(diffX * diffX + diffY * diffY + diffZ * diffZ);
    if (magnitude > MOTION_THRESHOLD)
    {
      DebugPrint("Motion alarm: %lu\r\n", magnitude);
      DispatchAlarmEvent(ALARM_EVENT_MOTION, magnitude);
    }

    prev[0] = current[0];
    prev[1] = current[1];
    prev[2] = current[2];
  }
  /* USER CODE END MotionTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

static void DispatchAlarmEvent(AlarmEventSource_t source, uint32_t magnitude)
{
  if (alarmEventQueue == NULL)
  {
    return;
  }

  AlarmEvent_t evt =
  {
    .source = source,
    .magnitude = magnitude
  };

  if (xQueueSend(alarmEventQueue, &evt, pdMS_TO_TICKS(50)) != pdPASS)
  {
    DebugPrint("Alarm queue full (src=%u mag=%lu)\r\n", source, magnitude);
  }
//  DebugPrint("Dispatch Alarm!!!\r\n");
}

static void AnalyzeAudioFrame(const int32_t *frame, size_t length)
{
  if ((frame == NULL) || (length == 0U))
  {
    return;
  }

  // if ((int32_t)(xTaskGetTickCount() - s_audioStabilizeUntil) < 0)
  // {
  //   /* Discard frames during DFSDM stabilization window */
  //   return;
  // }

  frameEnergy = CalculateFrameEnergy(frame, length);

  if (!s_audioCalibrationDone)
  {
    s_audioCalibrationAccum += frameEnergy;
    s_audioCalibrationCount++;
    if (s_audioCalibrationCount >= AUDIO_CALIBRATION_FRAMES)
    {
      uint32_t base = (uint32_t)(s_audioCalibrationAccum / s_audioCalibrationCount);
      s_audioThreshold = base + (base >> 1); /* add 50% headroom */
      s_audioCalibrationDone = true;
      DebugPrint("Audio calibration done (base: %lu thr: %lu)\r\n", base, s_audioThreshold);
    }
    return;
  }

  if (frameEnergy > s_audioThreshold)
  {
    DebugPrint("{\r\n");
    // DebugPrint("	audio_called_n: %ld\r\n", audio_called_n);
    DebugPrint("	frameEnergy: %ld\r\n", frameEnergy);
    DebugPrint("	audioThreshold: %ld\r\n", s_audioThreshold);
    DebugPrint("}\r\n");
    TickType_t now = xTaskGetTickCount();
    if ((int32_t)(now - s_lastAudioAlarmTick) >= (int32_t)pdMS_TO_TICKS(AUDIO_RETRIGGER_MS))
    {
      s_lastAudioAlarmTick = now;
      DispatchAlarmEvent(ALARM_EVENT_AUDIO, (uint32_t)frameEnergy);
    }
  }
  else
  {
//    DebugPrint("Quiet...frame energy is: %u\r\n", (unsigned int)frameEnergy);
  }
}

static void InitAlarmWaveform(void)
{
  for (uint32_t i = 0; i < ALARM_WAVEFORM_SAMPLES; ++i)
  {
    float angle = (2.0f * (float)PI * (float)i) / (float)ALARM_WAVEFORM_SAMPLES;
    float s = sinf(angle);
    int32_t y = (int32_t)DAC_CENTER + (int32_t)((float)AMP * s);
    if (y < 0) y = 0;
    if (y > (int32_t)DAC_MAX) y = DAC_MAX;
    s_alarmWaveform[i] = (uint16_t)y;
  }
}

static uint32_t CalculateFrameEnergy(const int32_t *frame, size_t length)
{
  if (length == 0U)
  {
    return 0;
  }

//  if (audio_called_n == 64) {
//	  ;
//  }

  uint32_t acc = 0;
  for (size_t i = 0; i < length; ++i)
  {
      int32_t raw = frame[i] >> AUDIO_SAMPLE_SHIFT; // still signed
      if (raw > 10000000) {
    	  DebugPrint("abnormal sound, raw = %ld\r\n", raw);

      }
      if (raw < 0) raw = -raw;   // take magnitude

      acc += (uint32_t)raw;      // safe: now always positive
  }

  acc /= (uint32_t)length;
  return acc;
}

static HAL_StatusTypeDef StartAlarmOutput(void)
{
  HAL_StatusTypeDef status = HAL_DAC_Start_DMA(&hdac1,
                                               DAC_CHANNEL_1,
                                               (uint32_t *)s_alarmWaveform,
                                               ALARM_WAVEFORM_SAMPLES,
                                               DAC_ALIGN_12B_R);
  if (status != HAL_OK)
  {
    return status;
  }

  status = HAL_TIM_Base_Start(&htim2);
  if (status != HAL_OK)
  {
    HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
    return status;
  }

  HAL_GPIO_WritePin(Error_LED_GPIO_Port, Error_LED_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(green_LED_GPIO_Port, green_LED_Pin, GPIO_PIN_RESET);
  s_alarmOutputActive = true;
  return HAL_OK;
}

static void StopAlarmOutput(void)
{
  HAL_TIM_Base_Stop(&htim2);
  HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
  HAL_GPIO_WritePin(Error_LED_GPIO_Port, Error_LED_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(green_LED_GPIO_Port, green_LED_Pin, GPIO_PIN_SET);
  s_alarmOutputActive = false;
}

static void DebugPrint(const char *format, ...)
{
  if (huart1.gState == HAL_UART_STATE_RESET)
  {
    return;
  }

  char buffer[128];
  va_list args;
  va_start(args, format);
  int len = vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  if (len <= 0)
  {
    return;
  }

  if (len > (int)sizeof(buffer))
  {
    len = sizeof(buffer);
  }

  HAL_UART_Transmit(&huart1, (uint8_t *)buffer, (uint16_t)len, HAL_MAX_DELAY);
}

void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  if (hdfsdm_filter != &hdfsdm1_filter0)
  {
    return;
  }

  BaseType_t higherPriorityTaskWoken = pdFALSE;
  s_audioReadyMask |= AUDIO_BUFFER_HALF_FLAG;

  TaskHandle_t targetHandle = audioProcessingTaskNativeHandle;
  if ((targetHandle == NULL) && (audioProcessingHandle != NULL))
  {
    targetHandle = (TaskHandle_t)audioProcessingHandle;
  }

  if (targetHandle != NULL)
  {
    vTaskNotifyGiveFromISR(targetHandle, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
  }
  else
  {
    DebugPrint("DFSDM half callback w/o task handle\r\n");
  }
}

void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  if (hdfsdm_filter != &hdfsdm1_filter0)
  {
    return;
  }

  BaseType_t higherPriorityTaskWoken = pdFALSE;
  s_audioReadyMask |= AUDIO_BUFFER_FULL_FLAG;
  
  TaskHandle_t targetHandle = audioProcessingTaskNativeHandle;
  if ((targetHandle == NULL) && (audioProcessingHandle != NULL))
  {
    targetHandle = (TaskHandle_t)audioProcessingHandle;
  }

  if (targetHandle != NULL)
  {
    vTaskNotifyGiveFromISR(targetHandle, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
  }
  else
  {
    DebugPrint("DFSDM full callback w/o task handle\r\n");
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  (void)GPIO_Pin;
}

void vApplicationIdleHook(void)
{
  /* Enter SLEEP when alarm output is idle; DFSDM/DMA and accel IRQs will wake us */
  if (s_alarmOutputActive)
  {
    return;
  }

  HAL_SuspendTick();
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  HAL_ResumeTick();
}
/* USER CODE END Application */
