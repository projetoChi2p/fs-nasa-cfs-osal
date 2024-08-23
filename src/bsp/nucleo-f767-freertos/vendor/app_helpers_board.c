#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "nucleo_stm32f767.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "app_helpers.h"




#define BOARD_SYSCONF1 "\r\nb"
#define BOARD_SYSCONF2 "oot\r\n"

#define BOARD_LE "le.\r\n"
#define BOARD_BE "be.\r\n"

#define HLP_USE_UART3
//#undef HLP_USE_UART3
#define HLP_USE_UART2
#undef HLP_USE_UART2

extern __IO uint32_t uwTick;
extern SemaphoreHandle_t xStdioMutex;

#ifdef HLP_USE_UART2
#define UART2_BAUDRATE 921600
UART_HandleTypeDef g_stm32_uart2;
#endif

#ifdef HLP_USE_UART3
/* NUCLEO F767 onboard UART-USB transceiver supports up to 2000000 bps on Linux. */
#define UART3_BAUDRATE 115200
UART_HandleTypeDef g_stm32_uart3;
#endif

UART_HandleTypeDef* g_phStm32UartConsole;

#ifdef HLP_USE_UART2

/*****************************************************
 */
void MX_USART_UART2_Init(void) {
    
    g_stm32_uart2.Instance = USART2;
    g_stm32_uart2.Init.BaudRate = UART2_BAUDRATE;
    g_stm32_uart2.Init.WordLength = UART_WORDLENGTH_8B;
    g_stm32_uart2.Init.StopBits = UART_STOPBITS_1;
    g_stm32_uart2.Init.Parity = UART_PARITY_NONE;
    g_stm32_uart2.Init.Mode = UART_MODE_TX; //_RX;
    g_stm32_uart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    g_stm32_uart2.Init.OverSampling = UART_OVERSAMPLING_16;
    g_stm32_uart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    g_stm32_uart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&g_stm32_uart2) != HAL_OK)
    {
        Error_Handler();
    }
}



void UART2_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    assert_param(ZIO_RX_GPIO_Port == GPIOD);
    assert_param(ZIO_RX_GPIO_Port == ZIO_TX_GPIO_Port);

    if (g_stm32_uart2.Instance == USART2)
    {
          /* Peripheral clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**USART2 GPIO Configuration
        PD5     ------> USART2_TX
        PD6     ------> USART2_RX
        */
        GPIO_InitStruct.Pin = ZIO_RX_Pin | ZIO_TX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    }
}
#endif /* HLP_USE_UART2 */


#ifdef HLP_USE_UART3
/*****************************************************
 */
void MX_USART_UART3_Init(void) {
    
    memset(&g_stm32_uart3, 0, sizeof(g_stm32_uart3));

    g_stm32_uart3.Instance = USART3;
    g_stm32_uart3.Init.BaudRate = UART3_BAUDRATE;
    g_stm32_uart3.Init.WordLength = UART_WORDLENGTH_8B;
    g_stm32_uart3.Init.StopBits = UART_STOPBITS_1;
    g_stm32_uart3.Init.Parity = UART_PARITY_NONE;
    g_stm32_uart3.Init.Mode = UART_MODE_TX_RX;
    g_stm32_uart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    g_stm32_uart3.Init.OverSampling = UART_OVERSAMPLING_16;
    g_stm32_uart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    g_stm32_uart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&g_stm32_uart3) != HAL_OK) {
        Error_Handler();
    }
}


void UART3_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    assert_param(STLK_RX_Pin == GPIOD);
    assert_param(STLK_RX_Pin == STLK_TX_Pin);

    if (g_stm32_uart3.Instance == USART3)
    {
          /* Peripheral clock enable */
        __HAL_RCC_USART3_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**USART3 GPIO Configuration
        PD8     ------> USART3_TX
        PD9     ------> USART3_RX
        */
        GPIO_InitStruct.Pin = STLK_RX_Pin | STLK_TX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    }
}

#endif /* HLP_USE_UART3 */


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

       /* Infinite loop */

    HAL_GPIO_WritePin(GPIOB, LD1_Pin | LD3_Pin | LD2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_SET);
    while (1)
    {
    }
}
#endif /* USE_FULL_ASSERT */




/*****************************************************
 */
void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, LD1_Pin | LD3_Pin | LD2_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : USER_Btn_Pin */
    GPIO_InitStruct.Pin = USER_Btn_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
    GPIO_InitStruct.Pin = LD1_Pin | LD3_Pin | LD2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
    GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : USB_OverCurrent_Pin */
    GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
    /* User may add here some code to deal with this error */
    HAL_GPIO_WritePin(GPIOB, LD1_Pin | LD3_Pin | LD2_Pin, GPIO_PIN_SET);
    while (1)
    {
    }
}


int __io_putchar(int ch)
{
    ITM_SendChar(ch);
    return ch;
}

int _write(int file, char *ptr, int len)
{
    int DataIdx;
    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        __io_putchar(*ptr++);
    }
    return len;
  }




/**
  * @brief  Configure the MPU attributes as Write Through for SRAM1/2.
  * @note   The Base Address is 0x20010000 since this memory interface is the AXI.
  *         The Region Size is 256KB, it is related to SRAM1 and SRAM2  memory size.
  * @param  None
  * @retval None
  */
static void MPU_Config(void)
{
#if HLP_MPU_SET_WRITE_THROUGH
    MPU_Region_InitTypeDef MPU_InitStruct;

    /* Disable the MPU */
    HAL_MPU_Disable();

    /* Configure the MPU attributes as WT for SRAM */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    //MPU_InitStruct.BaseAddress = 0x20010000; // skip 64 kB DTCM
    MPU_InitStruct.BaseAddress = 0x20020000; // skip 128 kB DTCM 
    MPU_InitStruct.Size = MPU_REGION_SIZE_256KB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Enable the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
#endif /* HLP_MPU_SET_WRITE_THROUGH */
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
#if HLP_INSTRUCTION_CACHE_ENABLE
    /* Enable I-Cache */
    SCB_EnableICache();
#endif

#if HLP_DATA_CACHE_ENABLE
    /* Enable D-Cache */
    SCB_EnableDCache();
#endif
}


/*****************************************************
 */
void SystemClock_Config(void) {

    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Configure LSE Drive Capability
    */
    HAL_PWR_EnableBkUpAccess();
    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 216;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Activate the Over-Drive mode
    */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
    {
        Error_Handler();
    }

#ifdef HLP_USE_UART2
    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_USART2;
    PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
#endif

#ifdef HLP_USE_UART3
    PeriphClkInitStruct.PeriphClockSelection |= RCC_PERIPHCLK_USART3;
    PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
#endif

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

}

#define SETTLE_OR_FLUSH_SMALL_DELAY_REPEATS 1000000000


// called from .../osal/src/bsp/generic-freertos/src/bsp_start.c
void HLP_vSystemConfig(void) 
{
    __disable_irq();

    uwTick = 0;

    assert(__SIZEOF_POINTER__ == 4);

    g_phStm32UartConsole = &g_stm32_uart3;

    MPU_Config();
    CPU_CACHE_Enable();

    SystemClock_Config();

    HAL_Init();
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();


    MX_GPIO_Init();
    HAL_GPIO_WritePin(GPIOB, LD1_Pin | LD3_Pin | LD2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_SET);

    // @TODO FBV 2024-01-15 Investigate why UART is initialized twice
    #ifdef HLP_USE_UART2
    MX_USART_UART2_Init();
    UART2_GPIO_Init();
    MX_USART_UART2_Init();
    UART2_GPIO_Init();
    #endif

    #ifdef HLP_USE_UART3
    /* @TODO investigate why we are initializing twice */
    MX_USART_UART3_Init();
    UART3_GPIO_Init();
    MX_USART_UART3_Init();
    //UART3_GPIO_Init();
    #endif

    HLP_vConsolePrintBytesBaremetal((uint8_t*)BOARD_SYSCONF1, sizeof(BOARD_SYSCONF1));

    HLP_vConsoleInit();

    HLP_vRtosBringUp();

    HAL_GPIO_WritePin(GPIOB, LD1_Pin | LD3_Pin | LD2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_SET);

    HLP_vConsolePrintBytesBaremetal((uint8_t*)BOARD_SYSCONF2, sizeof(BOARD_SYSCONF2));

    if (HLP_bIsBigEndian()) 
    {
        HLP_vConsolePrintBytesBaremetal((uint8_t*)BOARD_BE, sizeof(BOARD_BE));
    }
    else 
    {
        HLP_vConsolePrintBytesBaremetal((uint8_t*)BOARD_LE, sizeof(BOARD_LE));
    }

}


/*****************************************************
 */
void HLP_vConsolePrintBytesBaremetal( const uint8_t *data, int size )
{
    // HAL_UART_Transmit() data arg is not const
    HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t *) data, size, HAL_MAX_DELAY);
}
