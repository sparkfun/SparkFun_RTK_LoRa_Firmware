/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    sys_conf.h
  * @author  MCD Application Team
  * @brief   Applicative configuration, e.g. : debug, trace, low power, sensors
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYS_CONF_H__
#define __SYS_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/

/**
  * @brief  Verbose level for all trace logs
  */
#define VERBOSE_LEVEL                        VLEVEL_OFF

/**
  * @brief Enable trace logs
  */
#define APP_LOG_ENABLED                      0

/**
  * @brief Activate monitoring (probes) of some internal RF signals for debug purpose
  */
#define DEBUG_SUBGHZSPI_MONITORING_ENABLED   0

#define DEBUG_RF_NRESET_ENABLED              0

#define DEBUG_RF_HSE32RDY_ENABLED            0

#define DEBUG_RF_SMPSRDY_ENABLED             0

#define DEBUG_RF_LDORDY_ENABLED              0

#define DEBUG_RF_DTB1_ENABLED                0

#define DEBUG_RF_BUSY_ENABLED                0

/**
  * @brief Enable/Disable MCU Debugger pins (dbg serial wires)
  * @note  by HW serial wires are ON by default, need to put them OFF to save power
  */
#define DEBUGGER_ENABLED                     0

/**
  * @brief Disable Low Power mode
  * @note  0: LowPowerMode enabled. MCU enters stop2 mode, 1: LowPowerMode disabled. MCU enters sleep mode only
  */
#define LOW_POWER_DISABLE                    0

/* USER CODE BEGIN EC */

/*
  TORCH:
    PORT_NUM     1 : UART2 is used for both configuration and RTCM (after TRANS, until +++)
    COM_PORT_IDX 1 : UART2 is the command port (UART1 is only used for firmware updates)
    DBG_PORT       : is best commented out, but can be set to 0 (UART1) if desired for debug

  FACET FP:
    PORT_NUM     2 : UART2 is used for configuration, UART1 carries the RTCM to/from the GNSS
    COM_PORT_IDX 1 : UART2 is the command port
    DBG_PORT       : must be commented out (unless you want to send debug to the GNSS?)
*/

// PORTNUM: Number of ports: 1:use com_port; 2:use com and data ports
#define PORT_NUM                             1
// COM_PORT_IDX: selects the command port uart: 0:uart1; 1:uart2 
#define COM_PORT_IDX                         1
// DBG_PORT: selects the debug port uart: 0:uart1; 1:uart2; Comment to disable
//#define DBG_PORT                             0

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __SYS_CONF_H__ */
