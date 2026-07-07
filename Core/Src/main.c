/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "app_main.h"
#include "app_state.h"
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
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc3;

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* Definitions for logTask */
osThreadId_t logTaskHandle;
const osThreadAttr_t logTask_attributes = {
  .name = "logTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ledTask */
osThreadId_t ledTaskHandle;
const osThreadAttr_t ledTask_attributes = {
  .name = "ledTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for keyTask */
osThreadId_t keyTaskHandle;
const osThreadAttr_t keyTask_attributes = {
  .name = "keyTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */
osThreadId_t sensorTaskHandle;
const osThreadAttr_t sensorTask_attributes = {
  .name = "sensorTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t displayTaskHandle;
const osThreadAttr_t displayTask_attributes = {
  .name = "displayTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t wifiTaskHandle;
const osThreadAttr_t wifiTask_attributes = {
  .name = "wifiTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t mqttTaskHandle;
const osThreadAttr_t mqttTask_attributes = {
  .name = "mqttTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC3_Init(void);
static void MX_USART3_UART_Init(void);
void StartlogTask(void *argument);
void StartLedTask(void *argument);
void StartKeyTask(void *argument);

/* USER CODE BEGIN PFP */
void StartSensorTask(void *argument);
void StartDisplayTask(void *argument);
void StartWifiTask(void *argument);
void StartMqttTask(void *argument);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
/* 温湿度浮点格式化工具 */
static void App_FormatFloat1(char *buf, size_t len, float value)
{
  if ((buf == NULL) || (len == 0U)) {
    return;
  }

  /* 将浮点数放大 10 倍转成整数，避免 printf 浮点格式化不可用。 */
  int32_t value_x10 = (value >= 0.0f) ?
                      (int32_t)(value * 10.0f + 0.5f) :
                      (int32_t)(value * 10.0f - 0.5f);

  /* 负数需要单独处理符号，方便后面用整数拼成 x.y 字符串。 */
  const char *sign = (value_x10 < 0) ? "-" : "";
  uint32_t abs_x10 = (value_x10 < 0) ? (uint32_t)(-value_x10) : (uint32_t)value_x10;

  snprintf(buf,
           len,
           "%s%lu.%lu",
           sign,
           (unsigned long)(abs_x10 / 10U),
           (unsigned long)(abs_x10 % 10U));
}

/* I2C扫描工具 */
static void App_I2CScan(I2C_HandleTypeDef *hi2c)
{
  uint8_t found_count = 0U;

  if (hi2c == NULL) {
    return;
  }

  printf("[i2c] scan start\r\n");

  /* I2C 7 位地址范围是 0x08~0x77，HAL 接口需要左移 1 位传入。 */
  for (uint8_t addr = 0x08U; addr <= 0x77U; addr++) {
    if (HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(addr << 1), 1U, 10U) == HAL_OK) {
      printf("[i2c] found device: 0x%02X\r\n", addr);
      found_count++;
    }
  }
  printf("[i2c] scan done, found=%u\r\n", found_count);
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
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_ADC3_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  App_Init();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

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
  /* creation of logTask */
  logTaskHandle = osThreadNew(StartlogTask, NULL, &logTask_attributes);

  /* creation of ledTask */
  ledTaskHandle = osThreadNew(StartLedTask, NULL, &ledTask_attributes);

  /* creation of keyTask */
  keyTaskHandle = osThreadNew(StartKeyTask, NULL, &keyTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  sensorTaskHandle = osThreadNew(StartSensorTask, NULL, &sensorTask_attributes);
  displayTaskHandle = osThreadNew(StartDisplayTask, NULL, &displayTask_attributes);
  wifiTaskHandle = osThreadNew(StartWifiTask, NULL, &wifiTask_attributes);
  mqttTaskHandle = osThreadNew(StartMqttTask, NULL, &mqttTask_attributes);

  if (sensorTaskHandle == NULL) {
    printf("[rtos] sensorTask create failed\r\n");
  }

  if (displayTaskHandle == NULL) {
    printf("[rtos] displayTask create failed\r\n");
  }

  if (wifiTaskHandle == NULL) {
    printf("[rtos] wifiTask create failed\r\n");
  }

  if (mqttTaskHandle == NULL) {
    printf("[rtos] mqttTask create failed\r\n");
  }
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // printf("STM32F407 AIoT bringup OK, tick=%lu\r\n", HAL_GetTick());
    // HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9 | GPIO_PIN_10);  // 翻转 LED0 和 LED1
    // HAL_Delay(1000);                                     // 延时 1000ms
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8|LED0_Pin|LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : KEY2_Pin KEY1_Pin KEY0_Pin */
  GPIO_InitStruct.Pin = KEY2_Pin|KEY1_Pin|KEY0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PF8 LED0_Pin LED1_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_8|LED0_Pin|LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : WK_UP_Pin */
  GPIO_InitStruct.Pin = WK_UP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(WK_UP_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void StartSensorTask(void *argument)
{
  for(;;)
  {
    /* 周期采集 AHT30 和光敏 ADC，并更新 AppState。 */
    App_SensorTaskLoop();
    osDelay(500);
  }
}

void StartDisplayTask(void *argument)
{
  for(;;)
  {
    /* 从 AppState 读取最新环境数据，并刷新串口日志和 OLED。 */
    App_DisplayTaskLoop();
    osDelay(1000);
  }
}

void StartWifiTask(void *argument)
{
  for(;;)
  {
    App_WifiTaskLoop();
    osDelay(2000);
  }
}

void StartMqttTask(void *argument)
{
  for(;;)
  {
    /* MQTT 当前仍是占位流程，恢复后便于后续接入上报状态机。 */
    App_MqttTaskLoop();
    osDelay(3000);
  }
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartlogTask */
/**
  * @brief  Function implementing the logTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartlogTask */
void StartlogTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    // printf("[log_task] tick=%lu\r\n", HAL_GetTick());
    osDelay(1000);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartLedTask */
/**
* @brief Function implementing the ledTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLedTask */
void StartLedTask(void *argument)
{
  /* USER CODE BEGIN StartLedTask */
  /* Infinite loop */
  for(;;)
  {
    AppState_t *s = AppState_Get();
    uint32_t delay = 500;
    if(s->mode == APP_MODE_FOCUS)  delay = 200;
    if(s->mode == APP_MODE_ALERT)  delay = 100;
    HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9 | GPIO_PIN_10);
    osDelay(delay);
  }
  /* USER CODE END StartLedTask */
}

/* USER CODE BEGIN Header_StartKeyTask */
/**
* @brief Function implementing the keyTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartKeyTask */
void StartKeyTask(void *argument)
{
  /* USER CODE BEGIN StartKeyTask */
  /*
   * keyTask：按键扫描任务。
   * KEY0：低电平有效，稳定按下后切换工作模式。
   * KEY1：低电平有效，稳定按下后通过串口打印当前系统状态。
   * KEY2：低电平有效，稳定按下后清除报警状态，并在 ALERT 模式下切回 NORMAL。
   * 这里使用“周期采样 +稳定时间确认”的方式做软件防抖，避免机械按键抖动造成重复触发。
   */
  const uint32_t scan_interval_ms = 10U;
  const uint32_t debounce_ms = 30U;

  /* 初始化 KEY0 状态：KEY0 用于切换工作模式。 */
  GPIO_PinState key0_last_sample = HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin);
  GPIO_PinState key0_stable_state = key0_last_sample;
  uint32_t key0_stable_time_ms = 0U;
  /* 标记按键是否已处理，避免按键长按时重复触发。 */
  uint8_t key0_press_handled = (key0_stable_state == GPIO_PIN_RESET) ? 1U : 0U;

  /* 初始化 KEY1 状态：KEY1 用于串口打印当前系统状态。 */
  GPIO_PinState key1_last_sample = HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin);
  GPIO_PinState key1_stable_state = key1_last_sample;
  uint32_t key1_stable_time_ms = 0U;
  uint8_t key1_press_handled = (key1_stable_state == GPIO_PIN_RESET) ? 1U : 0U;

  /* 初始化 KEY2 状态：KEY2 用于清除报警状态。 */
  GPIO_PinState key2_last_sample = HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin);
  GPIO_PinState key2_stable_state = key2_last_sample;
  uint32_t key2_stable_time_ms = 0U;
  uint8_t key2_press_handled = (key2_stable_state == GPIO_PIN_RESET) ? 1U : 0U;

  for(;;)
  {
    GPIO_PinState key0_sample = HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin);
    GPIO_PinState key1_sample = HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin);
    GPIO_PinState key2_sample = HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin);

    /* KEY0 防抖状态机 */
    if (key0_sample == key0_last_sample) {
      if (key0_stable_time_ms < debounce_ms) {
        key0_stable_time_ms += scan_interval_ms;
      }
    } else {
      key0_last_sample = key0_sample;
      key0_stable_time_ms = 0U;
    }

    /* KEY0 按键状态更新 */
    if ((key0_stable_time_ms >= debounce_ms) && (key0_stable_state != key0_last_sample)) {
      key0_stable_state = key0_last_sample;
      if (key0_stable_state == GPIO_PIN_SET) {
        key0_press_handled = 0U;
      }
    }

    /* KEY1 防抖状态机 */
    if (key1_sample == key1_last_sample) {
      if (key1_stable_time_ms < debounce_ms) {
        key1_stable_time_ms += scan_interval_ms;
      }
    } else {
      key1_last_sample = key1_sample;
      key1_stable_time_ms = 0U;
    }

    /* KEY1 按键状态更新 */
    if ((key1_stable_time_ms >= debounce_ms) && (key1_stable_state != key1_last_sample)) {
      key1_stable_state = key1_last_sample;
      if (key1_stable_state == GPIO_PIN_SET) {
        key1_press_handled = 0U;
      }
    }

    /* KEY2 防抖状态机 */
    if (key2_sample == key2_last_sample) {
      if (key2_stable_time_ms < debounce_ms) {
        key2_stable_time_ms += scan_interval_ms;
      }
    } else {
      key2_last_sample = key2_sample;
      key2_stable_time_ms = 0U;
    }

    /* KEY2 按键状态更新 */
    if ((key2_stable_time_ms >= debounce_ms) && (key2_stable_state != key2_last_sample)) {
      key2_stable_state = key2_last_sample;
      if (key2_stable_state == GPIO_PIN_SET) {
        key2_press_handled = 0U;
      }
    }

    /* KEY0 按下稳定且本次按下尚未处理时，只触发一次模式切换。 */
    if ((key0_stable_state == GPIO_PIN_RESET) && (key0_press_handled == 0U)) {
      AppState_t *s = AppState_Get();
      AppMode_t next_mode = APP_MODE_NORMAL;

      /* 模式循环顺序：NORMAL -> FOCUS -> ALERT -> NORMAL。 */
      if (s->mode == APP_MODE_NORMAL) {
        next_mode = APP_MODE_FOCUS;
      } else if (s->mode == APP_MODE_FOCUS) {
        next_mode = APP_MODE_ALERT;
      }

      AppState_SetMode(next_mode);
      key0_press_handled = 1U;

      printf("[key] mode changed: %s\r\n", AppState_ModeToString(next_mode));

      /* 蜂鸣器短响 80ms，作为 KEY0 模式切换已经生效的反馈。 */
      HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_SET);
      osDelay(80);
      HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_RESET);
    }

    /* KEY1 稳定按下且本次按下尚未处理时，打印一次当前系统状态。 */
    if ((key1_stable_state == GPIO_PIN_RESET) && (key1_press_handled == 0U)) {
      AppState_t *s = AppState_Get();
      char temp_text[12];
      char hum_text[12];

      /* 先把温湿度转成字符串，再交给 printf 使用 %s 输出。 */
      App_FormatFloat1(temp_text, sizeof(temp_text), s->temperature);
      App_FormatFloat1(hum_text, sizeof(hum_text), s->humidity);

      printf("[key] status: mode=%s light_raw=%u light=%d aht=%s temp=%s "
             "hum=%s alert=%u wifi=%s mqtt=%s\r\n",
             AppState_ModeToString(s->mode), (unsigned int)s->light_raw,
             s->light_percent, s->aht30_valid ? "OK" : "ERR",
             temp_text, 
             hum_text, 
             (unsigned int)s->alert_active,
             AppState_WifiToString(s->wifi_status),
             AppState_MqttToString(s->mqtt_status));
      
      /* KEY1 作为调试键：打印系统状态后，顺便扫描 I2C1 总线设备。 */
      App_I2CScan(&hi2c1);
      /* 标记本次 KEY1 按下已处理，避免长按时重复打印。 */
      key1_press_handled = 1U;
    }

    /* KEY2 按下稳定且本次按下尚未处理时，清除报警状态。 */
    if ((key2_stable_state == GPIO_PIN_RESET) && (key2_press_handled == 0U)) {
      AppState_t *s = AppState_Get();

      /* 清除报警标志，表示用户已经手动确认当前报警。 */
      s->alert_active = 0U;

      /* 如果当前处于 ALERT 模式，清除报警后自动回到 NORMAL 模式。 */
      if (s->mode == APP_MODE_ALERT) {
        AppState_SetMode(APP_MODE_NORMAL);
      }
      
      printf("[key] alert cleared: mode=%s alert=%u\r\n",
             AppState_ModeToString(s->mode),
             (unsigned int)s->alert_active);

      /* 标记本次 KEY2 按下已处理，避免长按时重复清除和重复打印。 */
      key2_press_handled = 1U;
    }
    /* 任务周期延时 */
    osDelay(scan_interval_ms);
  }
  /* USER CODE END StartKeyTask */
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
