#ifndef __NUCLEO_STM32F767_H
#define __NUCLEO_STM32F767_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);


extern UART_HandleTypeDef* g_phStm32UartConsole;
extern UART_HandleTypeDef* g_phStm32UartInstruments;

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

//#define LF_FUNC_LINE_LF ("\r\n " STRINGIZE_VALUE_OF(__func__) ":" STRINGIZE_VALUE_OF(__LINE__) "\r\n")
#define LF_LINE_LF ("\r\n" STRINGIZE_VALUE_OF(__LINE__) "\r\n")



/* Private defines -----------------------------------------------------------*/
#define USER_Btn_Pin GPIO_PIN_13
#define USER_Btn_GPIO_Port GPIOC
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB

#define ZIO_RX_Pin GPIO_PIN_6
#define ZIO_RX_GPIO_Port GPIOD
#define ZIO_TX_Pin GPIO_PIN_5
#define ZIO_TX_GPIO_Port GPIOD

#define STLK_RX_Pin GPIO_PIN_8
#define STLK_RX_GPIO_Port GPIOD
#define STLK_TX_Pin GPIO_PIN_9
#define STLK_TX_GPIO_Port GPIOD
#define USB_PowerSwitchOn_Pin GPIO_PIN_6
#define USB_PowerSwitchOn_GPIO_Port GPIOG
#define USB_OverCurrent_Pin GPIO_PIN_7
#define USB_OverCurrent_GPIO_Port GPIOG
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB

#ifdef __cplusplus
}
#endif


#endif /* __NUCLEO_STM32F767_H */
