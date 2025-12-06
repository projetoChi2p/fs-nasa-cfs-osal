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

#ifdef BUILD_STM32F767_TRACE_ENABLE

// See also ST Microeletronics Reference manual RM0410 "Debug support (DBG)" chapter

/* The configure_tracing() and other supporting functions 
   below are based on the code from
   https://github.com/PetteriAimonen/STM32_Trace_Example
   which is declared in the repository as released into 
   the public domain.
*/

void ITM_Print(int port, const char *p)
{
    if ((ITM->TCR & ITM_TCR_ITMENA_Msk) && (ITM->TER & (1UL << port)))
    {
        while (*p)
        {
            while (ITM->PORT[port].u32 == 0);
            ITM->PORT[port].u8 = *p++;
        }
    }
}

extern uint16_t g_u16MessageCount;


#define BOARD_SYSCONF4 "May not enable trace.\r\n"

void configure_tracing()
{

    #ifdef BUILD_STM32F767_TRACE_SWO

    // RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    // AFIO->MAPR |= (2 << 24); // Disable JTAG to release TRACESWO

    if (!(DBGMCU->CR & DBGMCU_CR_TRACE_IOEN))
    {
        // Some (all?) STM32s don't allow writes to DBGMCU register until
        // C_DEBUGEN in CoreDebug->DHCSR is set. This cannot be set by the
        // CPU itself, so in practice you need to connect to the CPU with
        // a debugger once before resetting it.
        HLP_vConsolePrintBytesBaremetal((uint8_t*)BOARD_SYSCONF4, sizeof(BOARD_SYSCONF4));
        //return;
    }

    /* For NUCLEO-F767ZI (MB1137 Rev. B) board the SWO pin is available
       on CN6 pin 6 on the ST-Link side of the board:
       CN6-1 
       CN6-2 JTCK
       CN6-3 GND
       CN6-4 JTMS
       CN6-5 NRST
       CN6-6 SWO
     */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = SWO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP; ///GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_SWJ;
    HAL_GPIO_Init(SWO_GPIO_Port, &GPIO_InitStruct);

    #else /* BUILD_STM32F767_TRACE_SWO */

    __HAL_RCC_GPIOE_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_TRACE;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* TRACE pins on NUCLEO-F767ZI (MB1137 Rev. B) board:

      PE2 (CN11 pin 46) -> TRACECLK
      PE3 (CN11 pin 47) -> TRACED0
      PE4 (CN11 pin 48) -> TRACED1
      PE5 (CN11 pin 50) -> TRACED2
      PE6 (CN11 pin 62) -> TRACED3

      See also STM32 Nucleo-144 MB1137 user manual UM1974, chapter "ST morpho connector"
               STM32-Nucleo-144 MB1137 electric schematics
               STM32F76... Datasheet DS11532, chapter "Pinouts and pin description"
               STM32F7.... Reference manual RM0410
     */

    #endif /* ! BUILD_STM32F767_TRACE_SWO */

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; 

    #ifndef BUILD_STM32F767_TRACE_SWO
    TPI->CSPSR = 8;
    #endif
    TPI->FFCR = 0x102; // TPIU packet framing enabled when bit 2 is set.
                       // Can use 0x100 if you only need DWT/ITM and not ETM.

    #ifdef BUILD_STM32F767_TRACE_SWO
        TPI->ACPR = 3;//(12500000 / 115200) - 1; // See also SystemClock_Config()
        TPI->SPPR = 2; // Pin protocol = NRZ/USART: 
                       // 0x1 = Single Wire Output (Manchester)
                       // 0x2 = Single Wire Output (NRZ)
    #else
        TPI->SPPR = 0;
    #endif

    DBGMCU->CR |= DBGMCU_CR_TRACE_IOEN;

    //DBGMCU->CR |= 0x00000060;
    DBGMCU->CR &= ~DBGMCU_CR_TRACE_MODE_Msk;
    DBGMCU->CR |= (0b00 << DBGMCU_CR_TRACE_MODE_Pos); 


    /* Configure PC sampling and exception trace  */
    DWT->LAR = 0xC5ACCE55;
    DWT->CTRL = (1 << DWT_CTRL_CYCTAP_Pos) // Prescaler for PC sampling
                                           // 0 = x64, 1 = x1024
              | (0 << DWT_CTRL_POSTPRESET_Pos) // Postscaler for PC sampling
                                                // Divider = value + 1
              | (1 << DWT_CTRL_PCSAMPLENA_Pos) // Enable PC sampling
              | (1 << DWT_CTRL_SYNCTAP_Pos)    // Sync packet interval
                                               // 0 = Off, 1 = Every 2^23 cycles,
                                               // 2 = Every 2^25, 3 = Every 2^27
              | (1 << DWT_CTRL_EXCTRCENA_Pos)  // Enable exception trace
              | (1 << DWT_CTRL_CYCCNTENA_Pos); // Enable cycle counter
    
    /* Configure instrumentation trace macroblock */
    ITM->LAR = 0xC5ACCE55;
    ITM->TCR = (1 << ITM_TCR_TraceBusID_Pos) // Trace bus ID for TPIU
             | (1 << ITM_TCR_DWTENA_Pos) // Enable events from DWT
             | (1 << ITM_TCR_SYNCENA_Pos) // Enable sync packets
             | (1 << ITM_TCR_ITMENA_Pos); // Main enable for ITM
    ITM->TER = 0xFFFFFFFF; // Enable all stimulus ports
    
    /* Configure embedded trace macroblock */
   /* ETM->LAR = 0xC5ACCE55;
    ETM_SetupMode();
    ETM->CR = ETM_CR_ETMEN // Enable ETM output port
            | ETM_CR_STALL_PROCESSOR // Stall processor when fifo is full
            | ETM_CR_BRANCH_OUTPUT
            | ETM_CR_TRACE_ADDR
            | ETM_CR_TRACE_DATA
            | ETM_CR_PORTSIZE_4BIT; // Report all branches
         // | ETM_CR_PORTIZE_8BIT;  // Add this code in F103 to set port_size 21, 6, 5, 4 as 0, 0, 0, 1 for 8Bit.
    ETM->TRACEIDR = 2; // Trace bus ID for TPIU
    ETM->TECR1 = 0x00000000; // Trace always enabled
    //ETM->FFRR = ETM_FFRR_EXCLUDE; // Stalling always enabled
    ETM->FFLR = 24; */// Stall when less than N bytes free in FIFO (range 1..24)
                    // Larger values mean less latency in trace, but more stalls.
    // ETM->TRIGGER = 0x0000406F; // Add this code in F103 to define the trigger event
    // ETM->TEEVR = 0x0000006F;   // Add this code in F103 to  define an event to start/stop
    // Note: we do not enable ETM trace yet, only for specific parts of code.
    //ETM_TraceMode(); // Set ETM to trace mode

    // ETM->LAR = 0xC5ACCE55;      

    // ETM_SetupMode();

    // ETM->TRIGGER = 0x0;             // No specific trigger
    // ETM->TEEVR = 0x0;               // No specific event
    // ETM->TECR1 = 0x0;               // Trace always
    // ETM->FFLR = 0x10;               // FIFO threshold
    // ETM->TRACEIDR = 0x2;            // Trace ID
    // ETM->CR = ETM_CR_ETMEN               // Enable ETM output
    //     | ETM_CR_BRANCH_OUTPUT       // Trace branches (key for basic blocks)
    //     | ETM_CR_TRACE_ADDR          // Trace address (not just data)
    //     | ETM_CR_PORTSIZE_4BIT       // 4-bit port, use 8-bit if supported
    //     | ETM_CR_STALL_PROCESSOR;   
    // DWT->COMP0 = (uint32_t)&g_u16MessageCount;
    // DWT->MASK0 = 0;
    // DWT->FUNCTION0 = (3 << DWT_FUNCTION_FUNCTION_Pos); 
    // ETM->CR |= 1;

    // ETM_TraceMode(); // Set ETM to trace mode

    while (0)
    {
      ITM_Print(1, "Sort");
      for (int i = 0; i < 100; i++)
        asm("nop");
    }
}

#endif /* BUILD_STM32F767_TRACE_ENABLE */



#define BOARD_SYSCONF1 "\r\nb"
#define BOARD_SYSCONF2 "oot\r\n"
#define BOARD_SYSCONF3 "Arm Cortex-M ETM/TPIU trace enabled.\r\n"

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
    uint8_t u8 = ch;
    HAL_UART_Transmit(g_phStm32UartConsole, &u8, 1, HAL_MAX_DELAY);

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
    #ifdef BUILD_STM32F767_TRACE_ENABLE
    // Clock frequency will be lower when trace is enabled to reduce loss of trace data.
    // See also collaterial in MX_USART3_UART_Init()
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 100;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    #else
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 216;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    #endif
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
    #ifdef BUILD_STM32F767_TRACE_ENABLE
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    #else
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    #endif
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
    #ifdef BUILD_STM32F767_TRACE_ENABLE
    PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_SYSCLK;
    #else
    PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
    #endif
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

    #ifdef BUILD_STM32F767_TRACE_ENABLE
    configure_tracing();
    HLP_vConsolePrintBytesBaremetal((uint8_t*)BOARD_SYSCONF3, sizeof(BOARD_SYSCONF3));
    #endif

}

/*****************************************************
 */
void HLP_vSystemRestart(void) {
    /* Request a reset from software for ARM Cortex M7*/
    /*  See https://community.freescale.com/thread/99740
        To write to this register, you must write 0x5FA to the VECTKEY field, otherwise the processor ignores the write.
        SYSRESETREQ will cause a system reset asynchronously, so need to wait afterwards.
    */
    /* Ensure all memory operations are complete before reset */
    __DSB();

    /* Write the reset value to the AIRCR register with the required key */
    SCB->AIRCR = (0x5FA << SCB_AIRCR_VECTKEY_Pos) |         /*!< SCB AIRCR: VECTKEY Position */
                 (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |    /*!< Preserve priority group */
                 SCB_AIRCR_SYSRESETREQ_Msk;                 /*!< SCB AIRCR: SYSRESETREQ Mask */

    __DSB();
    for (;;) {
        /* wait until reset*/
    }
}

uint32_t HLP_uGetResetType(void) {
    uint32_t reset_type;

    reset_type = HLP_RESET_TYPE_POWERON;

    return reset_type;
}



/*****************************************************
 */
void HLP_vConsolePrintBytesBaremetal( const uint8_t *data, int size )
{
    // HAL_UART_Transmit() data arg is not const
    HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t *) data, size, HAL_MAX_DELAY);
}

void HLP_vPrintChar(char c, int8_t out){
    if (c == 0) return;
    static volatile char trace_task_tag[3];
    trace_task_tag[0] = '~';
    trace_task_tag[1] = c + out;
    trace_task_tag[2] = '\n';
    HLP_vConsolePrintBytesBaremetal((uint8_t*)trace_task_tag, 3);
}

/* FBV 2024-11-27 This is the FreeRTOS heap for head_4.c policy we
 * are allocating explicitly to enforce alignment or to put it inside
 * arbitraty memory region.
 */
#if (configAPPLICATION_ALLOCATED_HEAP == 1)
//__attribute__ ((section(".l2_scratchpad")))
//__attribute__ ((aligned (8)))
//__attribute__ ((section(".noinit.freertos_heap")))
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif



#if configGENERATE_RUN_TIME_STATS == 1

static uint32_t g_tick_start;

void HLP_vSystemConfigPerfCounter(void)
{
    g_tick_start = uwTick;
}

uint32_t HLP_ulSystemGetPerfCounter(void)
{
    return uwTick-g_tick_start;
}

#endif /* configGENERATE_RUN_TIME_STATS == 1 */
