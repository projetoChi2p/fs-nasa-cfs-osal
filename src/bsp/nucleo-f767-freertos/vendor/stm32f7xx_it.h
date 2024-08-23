/**
  ******************************************************************************
  * @file    Templates/stm32f7xx_it.h 
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    18-November-2015 
  * @brief   This file contains the headers of the interrupt handlers.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F7xx_IT_H
#define __STM32F7xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


// void NMI_Handler(void) __attribute__ (( naked ));
// void HardFault_Handler(void) __attribute__ (( naked ));
// void MemManage_Handler(void) __attribute__ (( naked ));
// void BusFault_Handler(void) __attribute__ (( naked ));
// void UsageFault_Handler(void) __attribute__ (( naked ));
// void SVC_Handler(void) __attribute__ (( naked ));
// void DebugMon_Handler(void) __attribute__ (( naked ));
// void PendSV_Handler(void) __attribute__ (( naked ));
// void SysTick_Handler(void) __attribute__ (( naked ));
// void Default_Handler(void) __attribute__ (( naked ));
// void WWDG_IRQHandler(void) __attribute__ (( naked ));
// void PVD_IRQHandler(void) __attribute__ (( naked ));
// void TAMP_STAMP_IRQHandler(void) __attribute__ (( naked ));
// void RTC_WKUP_IRQHandler(void) __attribute__ (( naked ));
// void FLASH_IRQHandler(void) __attribute__ (( naked ));
// void RCC_IRQHandler(void) __attribute__ (( naked ));
// void EXTI0_IRQHandler(void) __attribute__ (( naked ));
// void EXTI1_IRQHandler(void) __attribute__ (( naked ));
// void EXTI2_IRQHandler(void) __attribute__ (( naked ));
// void EXTI3_IRQHandler(void) __attribute__ (( naked ));
// void EXTI4_IRQHandler(void) __attribute__ (( naked ));
// void DMA1_Stream0_IRQHandler(void) __attribute__ (( naked ));
// void DMA1_Stream1_IRQHandler(void) __attribute__ (( naked ));
// void DMA1_Stream2_IRQHandler(void) __attribute__ (( naked ));
// void DMA1_Stream3_IRQHandler(void) __attribute__ (( naked ));
// void DMA1_Stream4_IRQHandler(void) __attribute__ (( naked ));
// void DMA1_Stream5_IRQHandler(void) __attribute__ (( naked ));
// void DMA1_Stream6_IRQHandler(void) __attribute__ (( naked ));
// void ADC_IRQHandler(void) __attribute__ (( naked ));
// void CAN1_TX_IRQHandler(void) __attribute__ (( naked ));
// void CAN1_RX0_IRQHandler(void) __attribute__ (( naked ));
// void CAN1_RX1_IRQHandler(void) __attribute__ (( naked ));
// void CAN1_SCE_IRQHandler(void) __attribute__ (( naked ));
// void EXTI9_5_IRQHandler(void) __attribute__ (( naked ));
// void TIM1_BRK_TIM9_IRQHandler(void) __attribute__ (( naked ));
// void TIM1_UP_TIM10_IRQHandler(void) __attribute__ (( naked ));
// void TIM1_TRG_COM_TIM11_IRQHandler(void) __attribute__ (( naked ));
// void TIM1_CC_IRQHandler(void) __attribute__ (( naked ));
// void TIM2_IRQHandler(void) __attribute__ (( naked ));
// void TIM3_IRQHandler(void) __attribute__ (( naked ));
// void TIM4_IRQHandler(void) __attribute__ (( naked ));
// void I2C1_EV_IRQHandler(void) __attribute__ (( naked ));
// void I2C1_ER_IRQHandler(void) __attribute__ (( naked ));
// void I2C2_EV_IRQHandler(void) __attribute__ (( naked ));
// void I2C2_ER_IRQHandler(void) __attribute__ (( naked ));
// void SPI1_IRQHandler(void) __attribute__ (( naked ));
// void SPI2_IRQHandler(void) __attribute__ (( naked ));
// void USART1_IRQHandler(void) __attribute__ (( naked ));
// void USART2_IRQHandler(void) __attribute__ (( naked ));
// void USART3_IRQHandler(void) __attribute__ (( naked ));
// void EXTI15_10_IRQHandler(void) __attribute__ (( naked ));
// void RTC_Alarm_IRQHandler(void) __attribute__ (( naked ));
// void OTG_FS_WKUP_IRQHandler(void) __attribute__ (( naked ));
// void TIM8_BRK_TIM12_IRQHandler(void) __attribute__ (( naked ));
// void TIM8_UP_TIM13_IRQHandler(void) __attribute__ (( naked ));
// void TIM8_TRG_COM_TIM14_IRQHandler(void) __attribute__ (( naked ));
// void TIM8_CC_IRQHandler(void) __attribute__ (( naked ));
// void DMA1_Stream7_IRQHandler(void) __attribute__ (( naked ));
// void FMC_IRQHandler(void) __attribute__ (( naked ));
// void SDMMC1_IRQHandler(void) __attribute__ (( naked ));
// void TIM5_IRQHandler(void) __attribute__ (( naked ));
// void SPI3_IRQHandler(void) __attribute__ (( naked ));
// void UART4_IRQHandler(void) __attribute__ (( naked ));
// void UART5_IRQHandler(void) __attribute__ (( naked ));
// void TIM6_DAC_IRQHandler(void) __attribute__ (( naked ));
// void TIM7_IRQHandler(void) __attribute__ (( naked ));
// void DMA2_Stream0_IRQHandler(void) __attribute__ (( naked ));
// void DMA2_Stream1_IRQHandler(void) __attribute__ (( naked ));
// void DMA2_Stream2_IRQHandler(void) __attribute__ (( naked ));
// void DMA2_Stream3_IRQHandler(void) __attribute__ (( naked ));
// void DMA2_Stream4_IRQHandler(void) __attribute__ (( naked ));
// void ETH_IRQHandler(void) __attribute__ (( naked ));
// void ETH_WKUP_IRQHandler(void) __attribute__ (( naked ));
// void CAN2_TX_IRQHandler(void) __attribute__ (( naked ));
// void CAN2_RX0_IRQHandler(void) __attribute__ (( naked ));
// void CAN2_RX1_IRQHandler(void) __attribute__ (( naked ));
// void CAN2_SCE_IRQHandler(void) __attribute__ (( naked ));
// void OTG_FS_IRQHandler(void) __attribute__ (( naked ));
// void DMA2_Stream5_IRQHandler(void) __attribute__ (( naked ));
// void DMA2_Stream6_IRQHandler(void) __attribute__ (( naked ));
// void DMA2_Stream7_IRQHandler(void) __attribute__ (( naked ));
// void USART6_IRQHandler(void) __attribute__ (( naked ));
// void I2C3_EV_IRQHandler(void) __attribute__ (( naked ));
// void I2C3_ER_IRQHandler(void) __attribute__ (( naked ));
// void OTG_HS_EP1_OUT_IRQHandler(void) __attribute__ (( naked ));
// void OTG_HS_EP1_IN_IRQHandler(void) __attribute__ (( naked ));
// void OTG_HS_WKUP_IRQHandler(void) __attribute__ (( naked ));
// void OTG_HS_IRQHandler(void) __attribute__ (( naked ));
// void DCMI_IRQHandler(void) __attribute__ (( naked ));
// void RNG_IRQHandler(void) __attribute__ (( naked ));
// void FPU_IRQHandler(void) __attribute__ (( naked ));
// void UART7_IRQHandler(void) __attribute__ (( naked ));
// void UART8_IRQHandler(void) __attribute__ (( naked ));
// void SPI4_IRQHandler(void) __attribute__ (( naked ));
// void SPI5_IRQHandler(void) __attribute__ (( naked ));
// void SPI6_IRQHandler(void) __attribute__ (( naked ));
// void SAI1_IRQHandler(void) __attribute__ (( naked ));
// void LTDC_IRQHandler(void) __attribute__ (( naked ));
// void LTDC_ER_IRQHandler(void) __attribute__ (( naked ));
// void DMA2D_IRQHandler(void) __attribute__ (( naked ));
// void SAI2_IRQHandler(void) __attribute__ (( naked ));
// void QUADSPI_IRQHandler(void) __attribute__ (( naked ));
// void LPTIM1_IRQHandler(void) __attribute__ (( naked ));
// void CEC_IRQHandler(void) __attribute__ (( naked ));
// void I2C4_EV_IRQHandler(void) __attribute__ (( naked ));
// void I2C4_ER_IRQHandler(void) __attribute__ (( naked ));
// void SPDIF_RX_IRQHandler(void) __attribute__ (( naked ));
// void DFSDM1_FLT0_IRQHandler(void) __attribute__ (( naked ));
// void DFSDM1_FLT1_IRQHandler(void) __attribute__ (( naked ));
// void DFSDM1_FLT2_IRQHandler(void) __attribute__ (( naked ));
// void DFSDM1_FLT3_IRQHandler(void) __attribute__ (( naked ));
// void SDMMC2_IRQHandler(void) __attribute__ (( naked ));
// void CAN3_TX_IRQHandler(void) __attribute__ (( naked ));
// void CAN3_RX0_IRQHandler(void) __attribute__ (( naked ));
// void CAN3_RX1_IRQHandler(void) __attribute__ (( naked ));
// void CAN3_SCE_IRQHandler(void) __attribute__ (( naked ));
// void JPEG_IRQHandler(void) __attribute__ (( naked ));
// void MDIOS_IRQHandler(void) __attribute__ (( naked ));

void WWDG_IRQHandler(void);
void PVD_IRQHandler(void);
void TAMP_STAMP_IRQHandler(void);
void RTC_WKUP_IRQHandler(void);
void FLASH_IRQHandler(void);
void RCC_IRQHandler(void);
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);
void DMA1_Stream0_IRQHandler(void);
void DMA1_Stream1_IRQHandler(void);
void DMA1_Stream2_IRQHandler(void);
void DMA1_Stream3_IRQHandler(void);
void DMA1_Stream4_IRQHandler(void);
void DMA1_Stream5_IRQHandler(void);
void DMA1_Stream6_IRQHandler(void);
void ADC_IRQHandler(void);
void CAN1_TX_IRQHandler(void);
void CAN1_RX0_IRQHandler(void);
void CAN1_RX1_IRQHandler(void);
void CAN1_SCE_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void TIM1_BRK_TIM9_IRQHandler(void);
void TIM1_UP_TIM10_IRQHandler(void);
void TIM1_TRG_COM_TIM11_IRQHandler(void);
void TIM1_CC_IRQHandler(void);
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void TIM4_IRQHandler(void);
void I2C1_EV_IRQHandler(void);
void I2C1_ER_IRQHandler(void);
void I2C2_EV_IRQHandler(void);
void I2C2_ER_IRQHandler(void);
void SPI1_IRQHandler(void);
void SPI2_IRQHandler(void);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
void RTC_Alarm_IRQHandler(void);
void OTG_FS_WKUP_IRQHandler(void);
void TIM8_BRK_TIM12_IRQHandler(void);
void TIM8_UP_TIM13_IRQHandler(void);
void TIM8_TRG_COM_TIM14_IRQHandler(void);
void TIM8_CC_IRQHandler(void);
void DMA1_Stream7_IRQHandler(void);
void FMC_IRQHandler(void);
void SDMMC1_IRQHandler(void);
void TIM5_IRQHandler(void);
void SPI3_IRQHandler(void);
void UART4_IRQHandler(void);
void UART5_IRQHandler(void);
void TIM6_DAC_IRQHandler(void);
void TIM7_IRQHandler(void);
void DMA2_Stream0_IRQHandler(void);
void DMA2_Stream1_IRQHandler(void);
void DMA2_Stream2_IRQHandler(void);
void DMA2_Stream3_IRQHandler(void);
void DMA2_Stream4_IRQHandler(void);
void ETH_IRQHandler(void);
void ETH_WKUP_IRQHandler(void);
void CAN2_TX_IRQHandler(void);
void CAN2_RX0_IRQHandler(void);
void CAN2_RX1_IRQHandler(void);
void CAN2_SCE_IRQHandler(void);
void OTG_FS_IRQHandler(void);
void DMA2_Stream5_IRQHandler(void);
void DMA2_Stream6_IRQHandler(void);
void DMA2_Stream7_IRQHandler(void);
void USART6_IRQHandler(void);
void I2C3_EV_IRQHandler(void);
void I2C3_ER_IRQHandler(void);
void OTG_HS_EP1_OUT_IRQHandler(void);
void OTG_HS_EP1_IN_IRQHandler(void);
void OTG_HS_WKUP_IRQHandler(void);
void OTG_HS_IRQHandler(void);
void DCMI_IRQHandler(void);
void RNG_IRQHandler(void);
void FPU_IRQHandler(void);
void UART7_IRQHandler(void);
void UART8_IRQHandler(void);
void SPI4_IRQHandler(void);
void SPI5_IRQHandler(void);
void SPI6_IRQHandler(void);
void SAI1_IRQHandler(void);
void LTDC_IRQHandler(void);
void LTDC_ER_IRQHandler(void);
void DMA2D_IRQHandler(void);
void SAI2_IRQHandler(void);
void QUADSPI_IRQHandler(void);
void LPTIM1_IRQHandler(void);
void CEC_IRQHandler(void);
void I2C4_EV_IRQHandler(void);
void I2C4_ER_IRQHandler(void);
void SPDIF_RX_IRQHandler(void);
void DFSDM1_FLT0_IRQHandler(void);
void DFSDM1_FLT1_IRQHandler(void);
void DFSDM1_FLT2_IRQHandler(void);
void DFSDM1_FLT3_IRQHandler(void);
void SDMMC2_IRQHandler(void);
void CAN3_TX_IRQHandler(void);
void CAN3_RX0_IRQHandler(void);
void CAN3_RX1_IRQHandler(void);
void CAN3_SCE_IRQHandler(void);
void JPEG_IRQHandler(void);
void MDIOS_IRQHandler(void);

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void DebugMon_Handler(void);


#ifdef __cplusplus
}
#endif

#endif /* __STM32F7xx_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
