#include_directories(${OSAL_FREERTOS_INC_DIR})
#include_directories(${OSAL_FREERTOS_SRC_DIR}/portable/GCC/ARM_CM3)

message("*** Inside cmake list ${CMAKE_CURRENT_LIST_FILE}.")
message("*** Using OSAL_FREERTOS_SRC_DIR '${OSAL_FREERTOS_SRC_DIR}'.")
message("*** Using OSAL_FREERTOS_INC_DIR '${OSAL_FREERTOS_INC_DIR}'.")
message("*** Using OSAL_FREERTOS_PLUS_FAT_SRC_DIR '${OSAL_FREERTOS_PLUS_FAT_SRC_DIR}'.")

# FreeRTOS kernel
include_directories(${OSAL_FREERTOS_INC_DIR})
include_directories(${OSAL_FREERTOS_SRC_DIR}/include)
#include_directories(${OSAL_FREERTOS_SRC_DIR}/portable/GCC/ARM_CM4F)
FILE(GLOB FREERTOS_SOURCES
    ${OSAL_FREERTOS_SRC_DIR}/*.c
)

FILE(GLOB FREERTOS_PORTABLE_SOURCES
    ${OSAL_FREERTOS_SRC_DIR}/portable/GCC/ARM_CM4F/*.c
    ${OSAL_FREERTOS_SRC_DIR}/portable/MemMang/heap_4.c
)

FILE(GLOB FATFS_SOURCES
    ${OSAL_FREERTOS_PLUS_FAT_SRC_DIR}/*.c
    ${OSAL_FREERTOS_PLUS_FAT_SRC_DIR}/portable/common/*.c
)

include_directories(
    src
)

# FreeRTOS BSP vendored code
include_directories(
    vendor
)

#FILE(GLOB BOOT_CODE
set(BOOT_CODE
   vendor/init/startup_stm32f767xx.s
)

FILE(GLOB VENDOR_CODE
    ../shared-freertos/vendor/*.c
    vendor/STM32F7xx_HAL_Driver/Src/*.c
    vendor/*.c
)

set(SOURCES
    ../shared-freertos/src/bsp_start.c
    ../shared-freertos/src/bsp_console.c
)

add_library(osal_nucleo-f767-freertos_impl OBJECT
    ${BOOT_CODE}
    ${VENDOR_CODE}
    ${SOURCES}
    ${FREERTOS_SOURCES}
    ${FREERTOS_PORTABLE_SOURCES}
    ${FATFS_SOURCES}
)

set(OSAL_EXPECTED_OSTYPE "freertos" PARENT_SCOPE)
