/**
  ******************************************************************************
  * @file    interfaces_conf.h
  * @author  MCD Application Team
  * @brief   Contains Interfaces configuration
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Image license SLA0044,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INTERFACES_CONF_H
#define INTERFACES_CONF_H

#define MEMORIES_SUPPORTED                5U

/* ------------------------- Definitions for USART -------------------------- */
#define USARTx                            USART1

#define USARTx_TX_PIN                     GPIO_PIN_9
#define USARTx_TX_GPIO_PORT               GPIOA
#define USARTx_RX_PIN                     GPIO_PIN_10
#define USARTx_RX_GPIO_PORT               GPIOA
#define USARTx_ALTERNATE                  GPIO_AF7_USART1

/* ------------------------- Definitions for FDCAN -------------------------- */
#define FDCANx                            FDCAN1

#define FDCANx_TX_PIN                     GPIO_PIN_9
#define FDCANx_TX_GPIO_PORT               GPIOB
#define FDCANx_TX_AF                      GPIO_AF9_FDCAN1
#define FDCANx_RX_PIN                     GPIO_PIN_8
#define FDCANx_RX_GPIO_PORT               GPIOB
#define FDCANx_RX_AF                      GPIO_AF9_FDCAN1

#define FDCANx_IT0_IRQn                   FDCAN1_IT0_IRQn
#define FDCANx_IT1_IRQn                   FDCAN1_IT1_IRQn
#define FDCANx_IRQHandler                 FDCAN1_IRQHandler

#define FDCANx_FORCE_RESET()              __HAL_RCC_FDCAN_FORCE_RESET()
#define FDCANx_RELEASE_RESET()            __HAL_RCC_FDCAN_RELEASE_RESET()

#endif /* INTERFACES_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
