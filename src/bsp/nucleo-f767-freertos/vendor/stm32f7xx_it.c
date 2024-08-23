/**
  ******************************************************************************
  * @file    Templates/stm32f7xx_it.c
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    18-November-2015
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_it.h"
#include "nucleo_stm32f767.h"

#include "app_helpers.h"

/** @addtogroup STM32F7xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M7 Processor Exceptions Handlers                         */
/******************************************************************************/

#if 1
void HardFault_Handler(void) { 
  HAL_GPIO_WritePin(GPIOB, LD1_Pin | LD3_Pin | LD2_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);
  while (1) 
  {
  }
}

void NMI_Handler(void) { while (1) {} }
void MemManage_Handler(void) { while (1) {} }
void BusFault_Handler(void) { while (1) {} }
void UsageFault_Handler(void) { while (1) {} }
void DebugMon_Handler(void) { while (1) {} }


void WWDG_IRQHandler(void) { while (1) {} }
void PVD_IRQHandler(void) { while (1) {} }
void TAMP_STAMP_IRQHandler(void) { while (1) {} }
void RTC_WKUP_IRQHandler(void) { while (1) {} }
void FLASH_IRQHandler(void) { while (1) {} }
void RCC_IRQHandler(void) { while (1) {} }
void EXTI0_IRQHandler(void) { while (1) {} }
void EXTI1_IRQHandler(void) { while (1) {} }
void EXTI2_IRQHandler(void) { while (1) {} }
void EXTI3_IRQHandler(void) { while (1) {} }
void EXTI4_IRQHandler(void) { while (1) {} }
void DMA1_Stream0_IRQHandler(void) { while (1) {} }
void DMA1_Stream1_IRQHandler(void) { while (1) {} }
void DMA1_Stream2_IRQHandler(void) { while (1) {} }
void DMA1_Stream3_IRQHandler(void) { while (1) {} }
void DMA1_Stream4_IRQHandler(void) { while (1) {} }
void DMA1_Stream5_IRQHandler(void) { while (1) {} }
void DMA1_Stream6_IRQHandler(void) { while (1) {} }
void ADC_IRQHandler(void) { while (1) {} }
void CAN1_TX_IRQHandler(void) { while (1) {} }
void CAN1_RX0_IRQHandler(void) { while (1) {} }
void CAN1_RX1_IRQHandler(void) { while (1) {} }
void CAN1_SCE_IRQHandler(void) { while (1) {} }
void EXTI9_5_IRQHandler(void) { while (1) {} }
void TIM1_BRK_TIM9_IRQHandler(void) { while (1) {} }
void TIM1_UP_TIM10_IRQHandler(void) { while (1) {} }
void TIM1_TRG_COM_TIM11_IRQHandler(void) { while (1) {} }
void TIM1_CC_IRQHandler(void) { while (1) {} }
void TIM2_IRQHandler(void) { while (1) {} }
void TIM3_IRQHandler(void) { while (1) {} }
void TIM4_IRQHandler(void) { while (1) {} }
void I2C1_EV_IRQHandler(void) { while (1) {} }
void I2C1_ER_IRQHandler(void) { while (1) {} }
void I2C2_EV_IRQHandler(void) { while (1) {} }
void I2C2_ER_IRQHandler(void) { while (1) {} }
void SPI1_IRQHandler(void) { while (1) {} }
void SPI2_IRQHandler(void) { while (1) {} }
void USART1_IRQHandler(void) { while (1) {} }
void USART2_IRQHandler(void) { while (1) {} }
void USART3_IRQHandler(void) { while (1) {} }
void EXTI15_10_IRQHandler(void) { while (1) {} }
void RTC_Alarm_IRQHandler(void) { while (1) {} }
void OTG_FS_WKUP_IRQHandler(void) { while (1) {} }
void TIM8_BRK_TIM12_IRQHandler(void) { while (1) {} }
void TIM8_UP_TIM13_IRQHandler(void) { while (1) {} }
void TIM8_TRG_COM_TIM14_IRQHandler(void) { while (1) {} }
void TIM8_CC_IRQHandler(void) { while (1) {} }
void DMA1_Stream7_IRQHandler(void) { while (1) {} }
void FMC_IRQHandler(void) { while (1) {} }
void SDMMC1_IRQHandler(void) { while (1) {} }
void TIM5_IRQHandler(void) { while (1) {} }
void SPI3_IRQHandler(void) { while (1) {} }
void UART4_IRQHandler(void) { while (1) {} }
void UART5_IRQHandler(void) { while (1) {} }
void TIM6_DAC_IRQHandler(void) { while (1) {} }
void TIM7_IRQHandler(void) { while (1) {} }
void DMA2_Stream0_IRQHandler(void) { while (1) {} }
void DMA2_Stream1_IRQHandler(void) { while (1) {} }
void DMA2_Stream2_IRQHandler(void) { while (1) {} }
void DMA2_Stream3_IRQHandler(void) { while (1) {} }
void DMA2_Stream4_IRQHandler(void) { while (1) {} }
void ETH_IRQHandler(void) { while (1) {} }
void ETH_WKUP_IRQHandler(void) { while (1) {} }
void CAN2_TX_IRQHandler(void) { while (1) {} }
void CAN2_RX0_IRQHandler(void) { while (1) {} }
void CAN2_RX1_IRQHandler(void) { while (1) {} }
void CAN2_SCE_IRQHandler(void) { while (1) {} }
void OTG_FS_IRQHandler(void) { while (1) {} }
void DMA2_Stream5_IRQHandler(void) { while (1) {} }
void DMA2_Stream6_IRQHandler(void) { while (1) {} }
void DMA2_Stream7_IRQHandler(void) { while (1) {} }
void USART6_IRQHandler(void) { while (1) {} }
void I2C3_EV_IRQHandler(void) { while (1) {} }
void I2C3_ER_IRQHandler(void) { while (1) {} }
void OTG_HS_EP1_OUT_IRQHandler(void) { while (1) {} }
void OTG_HS_EP1_IN_IRQHandler(void) { while (1) {} }
void OTG_HS_WKUP_IRQHandler(void) { while (1) {} }
void OTG_HS_IRQHandler(void) { while (1) {} }
void DCMI_IRQHandler(void) { while (1) {} }
void RNG_IRQHandler(void) { while (1) {} }
void FPU_IRQHandler(void) { while (1) {} }
void UART7_IRQHandler(void) { while (1) {} }
void UART8_IRQHandler(void) { while (1) {} }
void SPI4_IRQHandler(void) { while (1) {} }
void SPI5_IRQHandler(void) { while (1) {} }
void SPI6_IRQHandler(void) { while (1) {} }
void SAI1_IRQHandler(void) { while (1) {} }
void LTDC_IRQHandler(void) { while (1) {} }
void LTDC_ER_IRQHandler(void) { while (1) {} }
void DMA2D_IRQHandler(void) { while (1) {} }
void SAI2_IRQHandler(void) { while (1) {} }
void QUADSPI_IRQHandler(void) { while (1) {} }
void LPTIM1_IRQHandler(void) { while (1) {} }
void CEC_IRQHandler(void) { while (1) {} }
void I2C4_EV_IRQHandler(void) { while (1) {} }
void I2C4_ER_IRQHandler(void) { while (1) {} }
void SPDIF_RX_IRQHandler(void) { while (1) {} }
void DFSDM1_FLT0_IRQHandler(void) { while (1) {} }
void DFSDM1_FLT1_IRQHandler(void) { while (1) {} }
void DFSDM1_FLT2_IRQHandler(void) { while (1) {} }
void DFSDM1_FLT3_IRQHandler(void) { while (1) {} }
void SDMMC2_IRQHandler(void) { while (1) {} }
void CAN3_TX_IRQHandler(void) { while (1) {} }
void CAN3_RX0_IRQHandler(void) { while (1) {} }
void CAN3_RX1_IRQHandler(void) { while (1) {} }
void CAN3_SCE_IRQHandler(void) { while (1) {} }
void JPEG_IRQHandler(void) { while (1) {} }
void MDIOS_IRQHandler(void) { while (1) {} }

#else
/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  //HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
  //while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
  HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
}

#if 0
/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
__weak void SVC_Handler(void)
{
  HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
__weak void PendSV_Handler(void)
{
  HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
__weak void SysTick_Handler(void)
{
  HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)("LF_LINE_LF"), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
	HAL_IncTick();
}
#endif

// uint8_t outbuf[9];
// const uint8_t digits[] = "0123456789ABCDEF";
// uint32_t num;
// int32_t i;

void Default_Handler(void)
{
  // //__disable_irq();
  // //NVIC_DisableIRQ(IRQn_Type IRQn);
  // num = __get_xPSR();
  // // i = 0;
  // // for (i = 0; i < sizeof(outbuf); i++) {
  // //     outbuf[i] = '0';
  // // }
  // i = 0;
  // do {
  //   outbuf[i] = digits[(num % 16)];
  //   i++;
  //   num /= 16;
  // } while (num > 0);
  // outbuf[i] = '0';
  // while (i >= 0) {
  //   HAL_UART_Transmit(g_phStm32UartConsole, outbuf[i], 1, HAL_MAX_DELAY);
  //   i--;
  // }

  //HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
  //__enable_irq();
  //NVIC_EnableIRQ(IRQn_Type IRQn);
  HAL_UART_Transmit(g_phStm32UartConsole, "!", 1, HAL_MAX_DELAY);
  //while (1)
  {
  }
}

void USART3_IRQHandler(void)
{
  HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
  //while (1) {};
}

void USART2_IRQHandler(void)
{
  HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
  //while (1) {};
}


void WWDG_IRQHandler(void)
{
  HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
  while (1)
  {
  }
}

void PVD_IRQHandler(void)
{
  HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
  while (1)
  {
  }
}

void TAMP_STAMP_IRQHandler(void)
{
  HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
  while (1)
  {
  }
}

void RTC_WKUP_IRQHandler(void)
{
  HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
  while (1)
  {
  }
}

void FLASH_IRQHandler(void)
{
  HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY);
  while (1)
  {
  }
}

void RCC_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void EXTI0_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void EXTI1_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void EXTI2_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void EXTI3_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void EXTI4_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA1_Stream0_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA1_Stream1_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA1_Stream2_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA1_Stream3_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA1_Stream4_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA1_Stream5_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA1_Stream6_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void ADC_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void CAN1_TX_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void CAN1_RX0_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void CAN1_RX1_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void CAN1_SCE_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void EXTI9_5_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void TIM1_BRK_TIM9_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void TIM1_UP_TIM10_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void TIM1_TRG_COM_TIM11_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void TIM1_CC_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void TIM2_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void TIM3_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void TIM4_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void I2C1_EV_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void I2C1_ER_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void I2C2_EV_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void I2C2_ER_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void SPI1_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void SPI2_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void USART1_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void EXTI15_10_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void RTC_Alarm_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void OTG_FS_WKUP_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void TIM8_BRK_TIM12_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void TIM8_UP_TIM13_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void TIM8_TRG_COM_TIM14_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void TIM8_CC_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA1_Stream7_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void FMC_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void SDMMC1_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void TIM5_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void SPI3_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void UART4_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void UART5_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void TIM6_DAC_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void TIM7_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA2_Stream0_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA2_Stream1_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA2_Stream2_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA2_Stream3_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA2_Stream4_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void ETH_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void ETH_WKUP_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void CAN2_TX_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void CAN2_RX0_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void CAN2_RX1_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void CAN2_SCE_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void OTG_FS_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA2_Stream5_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA2_Stream6_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA2_Stream7_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void USART6_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void I2C3_EV_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void I2C3_ER_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void OTG_HS_EP1_OUT_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void OTG_HS_EP1_IN_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void OTG_HS_WKUP_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void OTG_HS_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DCMI_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void RNG_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void FPU_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void UART7_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void UART8_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void SPI4_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void SPI5_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void SPI6_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void SAI1_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void LTDC_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void LTDC_ER_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DMA2D_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void SAI2_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void QUADSPI_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void LPTIM1_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void CEC_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void I2C4_EV_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void I2C4_ER_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void SPDIF_RX_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DFSDM1_FLT0_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DFSDM1_FLT1_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DFSDM1_FLT2_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void DFSDM1_FLT3_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void SDMMC2_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void CAN3_TX_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void CAN3_RX0_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void CAN3_RX1_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void CAN3_SCE_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void JPEG_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }
void MDIOS_IRQHandler(void)  {   HAL_UART_Transmit(g_phStm32UartConsole, (uint8_t*)(LF_LINE_LF), sizeof(LF_LINE_LF), HAL_MAX_DELAY); }


/******************************************************************************/
/*                 STM32F7xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f7xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/
#endif


/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
