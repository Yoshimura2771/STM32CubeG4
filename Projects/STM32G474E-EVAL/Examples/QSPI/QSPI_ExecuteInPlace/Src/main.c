/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    QSPI/QSPI_ExecuteInPlace/Src/main.c
  * @author  MCD Application Team
  * @brief   This sample code shows how to erase part of the QSPI memory, write
  *          data in DMA mode and access to QSPI memory in memory-mapped mode.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
QSPI_HandleTypeDef hqspi1;
DMA_HandleTypeDef hdma_quadspi;

/* USER CODE BEGIN PV */
#if defined(__CC_ARM)
extern uint32_t Load$$QSPI$$Base;
extern uint32_t Load$$QSPI$$Length;
#elif defined(__ICCARM__)
#pragma section =".qspi"
#pragma section =".qspi_init"
#elif defined(__GNUC__)
extern uint32_t _qspi_init_base;
extern uint32_t _qspi_init_length;
#endif

__IO uint8_t CmdCplt, RxCplt, TxCplt, StatusMatch;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_QUADSPI1_Init(void);
/* USER CODE BEGIN PFP */
static void QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi);
static void QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi);
static void QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi);
static void GpioToggle(void);
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
  QSPI_CommandTypeDef      sCommand;
  QSPI_MemoryMappedTypeDef sMemMappedCfg;
  __IO uint32_t qspi_addr = 0;
  uint8_t *flash_addr = 0;
  __IO uint8_t step = 0;
  uint32_t max_size = 0, size = 0;


  /* STM32G4xx HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user
         can eventually implement his proper time base source (a general purpose
         timer for example or other time source), keeping in mind that Time base
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  /* USER CODE END 1 */


  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  BSP_LED_Init(LED1);
  BSP_LED_Init(LED2);
  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_QUADSPI1_Init();
  /* USER CODE BEGIN 2 */
  /* Initialize QSPI commande structure */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

#if defined(__CC_ARM)
  max_size = (uint32_t)(&Load$$QSPI$$Length);
#elif defined(__ICCARM__)
  max_size = __section_size(".qspi_init");
#elif defined(__GNUC__)
  max_size = (uint32_t)((uint8_t *)(&_qspi_init_length));
#endif
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    switch(step)
    {
      case 0:
        CmdCplt = 0;

        /* Enable write operations ------------------------------------------- */
        QSPI_WriteEnable(&hqspi1);

        /* Erasing Sequence -------------------------------------------------- */
        sCommand.Instruction = MT25QL512ABB_SECTOR_ERASE_64K_CMD;
        sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
        sCommand.Address     = qspi_addr;
        sCommand.DataMode    = QSPI_DATA_NONE;
        sCommand.DummyCycles = 0;

        if (HAL_QSPI_Command_IT(&hqspi1, &sCommand) != HAL_OK)
        {
          Error_Handler();
        }

        step++;
        break;

      case 1:
        if(CmdCplt != 0)
        {
          CmdCplt = 0;
          StatusMatch = 0;

          /* Configure automatic polling mode to wait for end of erase ------- */
          QSPI_AutoPollingMemReady(&hqspi1);

#if defined(__CC_ARM)
          flash_addr = (uint8_t *)(&Load$$QSPI$$Base);
#elif defined(__ICCARM__)
          flash_addr = (uint8_t *)(__section_begin(".qspi_init"));
#elif defined(__GNUC__)
          flash_addr = (uint8_t *)(&_qspi_init_base);
#endif

          /* Copy only one page if the section is bigger */
          if (max_size > QSPI_PAGE_SIZE)
          {
            size = QSPI_PAGE_SIZE;
          }
          else
          {
            size = max_size;
          }

          step++;
        }
        break;

      case 2:
        if(StatusMatch != 0)
        {
          StatusMatch = 0;
          TxCplt = 0;

          /* Enable write operations ----------------------------------------- */
          QSPI_WriteEnable(&hqspi1);

          /* Writing Sequence ------------------------------------------------ */
          sCommand.Instruction = MT25QL512ABB_EXTENDED_QUAD_INPUT_FAST_PROG_CMD;
          sCommand.AddressMode = QSPI_ADDRESS_4_LINES;
          sCommand.Address     = qspi_addr;
          sCommand.DataMode    = QSPI_DATA_4_LINES;
          sCommand.NbData      = size;

          if (HAL_QSPI_Command(&hqspi1, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
          {
            Error_Handler();
          }

          if (HAL_QSPI_Transmit_DMA(&hqspi1, flash_addr) != HAL_OK)
          {
            Error_Handler();
          }

          step++;

        }
        break;

      case 3:
        if(TxCplt != 0)
        {
          TxCplt = 0;
          StatusMatch = 0;

          /* Configure automatic polling mode to wait for end of program ----- */
          QSPI_AutoPollingMemReady(&hqspi1);

          step++;
        }
        break;

      case 4:
        if(StatusMatch != 0)
        {
          qspi_addr += size;
          flash_addr += size;

          /* Check if a new page writing is needed */
          if (qspi_addr < max_size)
          {
            /* Update the remaining size if it is less than the page size */
            if ((qspi_addr + size) > max_size)
            {
              size = (max_size - qspi_addr);
            }
            step = 2;
          }
          else
          {
            StatusMatch = 0;
            RxCplt = 0;

            /* Configure Volatile Configuration register (with new dummy cycles) */
            QSPI_DummyCyclesCfg(&hqspi1);

            /* Reading Sequence ------------------------------------------------ */
            sCommand.Instruction = MT25QL512ABB_4IO_FAST_READ_CMD;
            sCommand.DummyCycles = 4;

            sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

            if (HAL_QSPI_MemoryMapped(&hqspi1, &sCommand, &sMemMappedCfg) != HAL_OK)
            {
              Error_Handler();
            }

            step++;
          }
        }
        break;

      case 5:
        /* Execute the code from QSPI memory ------------------------------- */
        GpioToggle();
        break;

      default :
        Error_Handler();
    }
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_8) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks 
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_QSPI;
  PeriphClkInit.QspiClockSelection = RCC_QSPICLKSOURCE_SYSCLK;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief QUADSPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_QUADSPI1_Init(void)
{

  /* USER CODE BEGIN QUADSPI1_Init 0 */

  /* USER CODE END QUADSPI1_Init 0 */

  /* USER CODE BEGIN QUADSPI1_Init 1 */

  /* USER CODE END QUADSPI1_Init 1 */
  /* QUADSPI1 parameter configuration*/
  hqspi1.Instance = QUADSPI;
  hqspi1.Init.ClockPrescaler = 1;
  hqspi1.Init.FifoThreshold = 4;
  hqspi1.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
  hqspi1.Init.FlashSize = QSPI_FLASH_SIZE;
  hqspi1.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_4_CYCLE;
  hqspi1.Init.ClockMode = QSPI_CLOCK_MODE_0;
  hqspi1.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi1.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI1_Init 2 */

  /* USER CODE END QUADSPI1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */
/**
  * @brief  Command completed callbacks.
  * @param  hqspi QSPI handle
  * @retval None
  */
void HAL_QSPI_CmdCpltCallback(QSPI_HandleTypeDef *hqspi)
{
  CmdCplt++;
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  hqspi QSPI handle
  * @retval None
  */
void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
  RxCplt++;
}

/**
  * @brief  Tx Transfer completed callbacks.
  * @param  hqspi QSPI handle
  * @retval None
  */
void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
  TxCplt++;
}

/**
  * @brief  Status Match callbacks
  * @param  hqspi QSPI handle
  * @retval None
  */
void HAL_QSPI_StatusMatchCallback(QSPI_HandleTypeDef *hqspi)
{
  StatusMatch++;
}

/**
  * @brief  This function send a Write Enable and wait it is effective.
  * @param  hqspi QSPI handle
  * @retval None
  */
static void QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef     sCommand;
  QSPI_AutoPollingTypeDef sConfig;

  /* Enable write operations ------------------------------------------ */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = MT25QL512ABB_WRITE_ENABLE_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }

  /* Configure automatic polling mode to wait for write enabling ---- */
  sConfig.Match           = 0x02;
  sConfig.Mask            = 0x02;
  sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
  sConfig.StatusBytesSize = 1;
  sConfig.Interval        = 0x10;
  sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  sCommand.Instruction    = MT25QL512ABB_READ_STATUS_REG_CMD;
  sCommand.DataMode       = QSPI_DATA_1_LINE;

  if (HAL_QSPI_AutoPolling(hqspi, &sCommand, &sConfig, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function read the SR of the memory and wait the EOP.
  * @param  hqspi QSPI handle
  * @retval None
  */
static void QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef     sCommand;
  QSPI_AutoPollingTypeDef sConfig;

  /* Configure automatic polling mode to wait for memory ready ------ */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = MT25QL512ABB_READ_STATUS_REG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  sConfig.Match           = 0x00;
  sConfig.Mask            = 0x01;
  sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
  sConfig.StatusBytesSize = 1;
  sConfig.Interval        = 0x10;
  sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_QSPI_AutoPolling_IT(hqspi, &sCommand, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function configure the dummy cycles on memory side.
  * @param  hqspi QSPI handle
  * @retval None
  */
static void QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef sCommand;
  uint8_t reg;

  /* Read Volatile Configuration register --------------------------- */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = MT25QL512ABB_READ_VOL_CFG_REG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  sCommand.NbData            = 1;

  if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_QSPI_Receive(hqspi, &reg, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }

  /* Enable write operations ---------------------------------------- */
  QSPI_WriteEnable(hqspi);

  /* Write Volatile Configuration register (with new dummy cycles) -- */
  sCommand.Instruction = MT25QL512ABB_WRITE_VOL_CFG_REG_CMD;
  MODIFY_REG(reg, 0xF0, (4 << POSITION_VAL(0xF0)));

  if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_QSPI_Transmit(hqspi, &reg, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
  * @brief  Toggle the GPIOs
  * @param  None
  * @retval None
  */
#if defined(__CC_ARM)
#pragma arm section code = ".qspi"
#pragma no_inline
static void GpioToggle(void)
#elif defined(__ICCARM__)
static void GpioToggle(void) @ ".qspi"
#elif defined(__GNUC__)
static void __attribute__((section(".qspi"), noinline)) GpioToggle(void)
#endif
{
  BSP_LED_Toggle(LED1);
  /* Insert delay 100 ms */
  HAL_Delay(100);
  BSP_LED_Toggle(LED2);
  /* Insert delay 100 ms */
  HAL_Delay(100);
  BSP_LED_Toggle(LED3);
  /* Insert delay 100 ms */
  HAL_Delay(100);
  BSP_LED_Toggle(LED4);
  /* Insert delay 100 ms */
  HAL_Delay(100);
}
#if defined(__CC_ARM)
#pragma arm section code
#endif

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  BSP_LED_On(LED3);

  while(1)
  {
  }
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
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* Infinite loop */
  while (1)
  {
  }
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
