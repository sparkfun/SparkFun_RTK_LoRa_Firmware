/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart_if.c
  * @author  MCD Application Team
  * @brief   Configuration of UART driver interface for hyperterminal communication
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "usart_if.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/**
  * @brief DMA handle
  */
extern DMA_HandleTypeDef hdma_spi1_tx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_tx;

/**
  * @brief UART handle
  */
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

extern SPI_HandleTypeDef hspi1;

/**
  * @brief buffer to receive 1 character
  */
uint8_t charRx;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/**
  * @brief Trace driver callbacks handler
  */
const UTIL_ADV_TRACE_Driver_s UTIL_TraceDriver =
{
  vcom_Init,
  vcom_DeInit,
  vcom_ReceiveInit,
  vcom_Trace_DMA,
};

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
#if defined(DBG_PORT)
/**
  * @brief  TX complete callback
  * @return none
  */
static void (*TxCpltCallback)(void *);
#endif
#if defined(DBG_PORT) && (DBG_PORT < 2)
/**
  * @brief  RX complete callback
  * @param  rxChar ptr of chars buffer sent by user
  * @param  size buffer size
  * @param  error errorcode
  * @return none
  */
static void (*RxCpltCallback)(uint8_t *rxChar, uint16_t size, uint8_t error);
#endif

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/

UTIL_ADV_TRACE_Status_t vcom_Init(void (*cb)(void *))
{
  /* USER CODE BEGIN vcom_Init_1 */
#if !defined(DBG_PORT)
#warning DBG_PORT not defined. vcom / trace not available
#elif(DBG_PORT == 0)
  TxCpltCallback = cb;
  MX_DMA_Init();
  MX_USART1_UART_Init();
  LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_26);
  /* USER CODE END vcom_Init_1 */
  /* USER CODE BEGIN vcom_Init_2 */
#elif(DBG_PORT == 1)
  TxCpltCallback = cb;
  MX_DMA_Init();
  MX_USART2_UART_Init();
  LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_27);
  /* USER CODE END vcom_Init_2 */
#elif(DBG_PORT == 2)
  TxCpltCallback = cb;
  MX_DMA_Init();
  MX_SPI1_Init();
#else
#error Invalid DBG_PORT
#endif
  return UTIL_ADV_TRACE_OK;
}

UTIL_ADV_TRACE_Status_t vcom_DeInit(void)
{
  /* USER CODE BEGIN vcom_DeInit_1 */
#if defined(DBG_PORT) && (DBG_PORT == 0)
  /* ##-1- Reset peripherals ################################################## */
  __HAL_RCC_USART1_FORCE_RESET();
  __HAL_RCC_USART1_RELEASE_RESET();

  /* ##-2- MspDeInit ################################################## */
  HAL_UART_MspDeInit(&huart1);

  /* ##-3- Disable the NVIC for DMA ########################################### */
  /* USER CODE BEGIN 1 */
  HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);
  /* USER CODE END vcom_DeInit_1 */
  /* USER CODE BEGIN vcom_DeInit_2 */
#elif defined(DBG_PORT) && (DBG_PORT == 1)
  /* ##-1- Reset peripherals ################################################## */
  __HAL_RCC_USART2_FORCE_RESET();
  __HAL_RCC_USART2_RELEASE_RESET();

  /* ##-2- MspDeInit ################################################## */
  HAL_UART_MspDeInit(&huart2);

  /* ##-3- Disable the NVIC for DMA ########################################### */
  /* USER CODE BEGIN 1 */
  HAL_NVIC_DisableIRQ(DMA1_Channel6_IRQn);
  /* USER CODE END vcom_DeInit_2 */
#elif defined(DBG_PORT) && (DBG_PORT == 2)
  __HAL_RCC_SPI1_FORCE_RESET();
  __HAL_RCC_SPI1_RELEASE_RESET();
  HAL_SPI_MspDeInit(&hspi1);
  HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
#endif
  return UTIL_ADV_TRACE_OK;
}

void vcom_Trace(uint8_t *p_data, uint16_t size)
{
  /* USER CODE BEGIN vcom_Trace_1 */
#if defined(DBG_PORT) && (DBG_PORT == 0)
  HAL_UART_Transmit(&huart1, p_data, size, 1000);
  /* USER CODE END vcom_Trace_1 */
  /* USER CODE BEGIN vcom_Trace_2 */
#elif defined(DBG_PORT) && (DBG_PORT == 1)
  HAL_UART_Transmit(&huart2, p_data, size, 1000);
  /* USER CODE END vcom_Trace_2 */
#elif defined(DBG_PORT) && (DBG_PORT == 2)
  HAL_SPI_Transmit(&hspi1, p_data, size, 100);
#endif
}

UTIL_ADV_TRACE_Status_t vcom_Trace_DMA(uint8_t *p_data, uint16_t size)
{
  /* USER CODE BEGIN vcom_Trace_DMA_1 */
#if defined(DBG_PORT) && (DBG_PORT == 0)
  HAL_UART_Transmit_DMA(&huart1, p_data, size);
  /* USER CODE END vcom_Trace_DMA_1 */
  /* USER CODE BEGIN vcom_Trace_DMA_2 */
#elif defined(DBG_PORT) && (DBG_PORT == 1)
  HAL_UART_Transmit_DMA(&huart2, p_data, size);
  /* USER CODE END vcom_Trace_DMA_2 */
#elif defined(DBG_PORT) && (DBG_PORT == 2)
  HAL_SPI_Transmit_DMA(&hspi1, p_data, size);
#endif
  return UTIL_ADV_TRACE_OK;
}

UTIL_ADV_TRACE_Status_t vcom_ReceiveInit(void (*RxCb)(uint8_t *rxChar, uint16_t size, uint8_t error))
{
  /* USER CODE BEGIN vcom_ReceiveInit_1 */
#if defined(DBG_PORT) && (DBG_PORT == 0)
  UART_WakeUpTypeDef WakeUpSelection;

  /*record call back*/
  RxCpltCallback = RxCb;

  /*Set wakeUp event on start bit*/
  WakeUpSelection.WakeUpEvent = UART_WAKEUP_ON_STARTBIT;

  HAL_UARTEx_StopModeWakeUpSourceConfig(&huart1, WakeUpSelection);

  /* Make sure that no UART transfer is on-going */
  while (__HAL_UART_GET_FLAG(&huart1, USART_ISR_BUSY) == SET);

  /* Make sure that UART is ready to receive)   */
  while (__HAL_UART_GET_FLAG(&huart1, USART_ISR_REACK) == RESET);

  /* Enable USART interrupt */
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_WUF);

  /*Enable wakeup from stop mode*/
  HAL_UARTEx_EnableStopMode(&huart1);

  /*Start LPUART receive on IT*/
  HAL_UART_Receive_IT(&huart1, &charRx, 1);
  /* USER CODE END vcom_ReceiveInit_1 */
  /* USER CODE BEGIN vcom_ReceiveInit_2 */
#elif defined(DBG_PORT) && (DBG_PORT == 1)
  UART_WakeUpTypeDef WakeUpSelection;

  /*record call back*/
  RxCpltCallback = RxCb;

  /*Set wakeUp event on start bit*/
  WakeUpSelection.WakeUpEvent = UART_WAKEUP_ON_STARTBIT;

  HAL_UARTEx_StopModeWakeUpSourceConfig(&huart2, WakeUpSelection);

  /* Make sure that no UART transfer is on-going */
  while (__HAL_UART_GET_FLAG(&huart2, USART_ISR_BUSY) == SET);

  /* Make sure that UART is ready to receive)   */
  while (__HAL_UART_GET_FLAG(&huart2, USART_ISR_REACK) == RESET);

  /* Enable USART interrupt */
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_WUF);

  /*Enable wakeup from stop mode*/
  HAL_UARTEx_EnableStopMode(&huart2);

  /*Start LPUART receive on IT*/
  HAL_UART_Receive_IT(&huart2, &charRx, 1);
#endif
  return UTIL_ADV_TRACE_OK;
  /* USER CODE END vcom_ReceiveInit_2 */
}

void vcom_Resume(void)
{
  /* USER CODE BEGIN vcom_Resume_1 */
#if defined(DBG_PORT) && (DBG_PORT == 0)
  /*to re-enable lost UART settings*/
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

  /*to re-enable lost DMA settings*/
  if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END vcom_Resume_1 */
  /* USER CODE BEGIN vcom_Resume_2 */
#elif defined(DBG_PORT) && (DBG_PORT == 1)
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

  /*to re-enable lost DMA settings*/
  if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END vcom_Resume_2 */
#elif defined(DBG_PORT) && (DBG_PORT == 2)
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK)
  {
    Error_Handler();
  }
#endif
}

#if defined(DBG_PORT) && (DBG_PORT < 2)
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  /* USER CODE BEGIN HAL_UART_TxCpltCallback_1 */
  /* buffer transmission complete*/
  if (huart->Instance == USART1)
  {
    TxCpltCallback(NULL);
  }
  /* USER CODE END HAL_UART_TxCpltCallback_1 */
  /* USER CODE BEGIN HAL_UART_TxCpltCallback_2 */
  if (huart->Instance == USART2)
  {
    TxCpltCallback(NULL);
  }
  /* USER CODE END HAL_UART_TxCpltCallback_2 */
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* USER CODE BEGIN HAL_UART_RxCpltCallback_1 */
  if (huart->Instance == USART1)
  {
    if ((NULL != RxCpltCallback) && (HAL_UART_ERROR_NONE == huart->ErrorCode))
    {
      RxCpltCallback(&charRx, 1, 0);
    }
    HAL_UART_Receive_IT(huart, &charRx, 1);
  }
  /* USER CODE END HAL_UART_RxCpltCallback_1 */
  /* USER CODE BEGIN HAL_UART_RxCpltCallback_2 */
  if (huart->Instance == USART2)
  {
    if ((NULL != RxCpltCallback) && (HAL_UART_ERROR_NONE == huart->ErrorCode))
    {
      RxCpltCallback(&charRx, 1, 0);
    }
    HAL_UART_Receive_IT(huart, &charRx, 1);
  }
  /* USER CODE END HAL_UART_RxCpltCallback_2 */
}
#endif

#if defined(DBG_PORT) && (DBG_PORT == 2)
// This should be in spi.c. But TxCpltCallback is not in scope there...
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  TxCpltCallback(NULL);
}
#endif

/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/

/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */
