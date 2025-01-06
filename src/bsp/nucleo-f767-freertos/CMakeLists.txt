
message("*** Inside cmake list ${CMAKE_CURRENT_LIST_FILE}.")
message("*** Using OSAL_FREERTOS_SRC_DIR '${OSAL_FREERTOS_SRC_DIR}'.")
message("*** Using OSAL_FREERTOS_INC_DIR '${OSAL_FREERTOS_INC_DIR}'.")


# FreeRTOS kernel
include_directories(${OSAL_FREERTOS_INC_DIR})
include_directories(${OSAL_FREERTOS_SRC_DIR}/include)
FILE(GLOB FREERTOS_SOURCES
    ${OSAL_FREERTOS_SRC_DIR}/*.c
)

FILE(GLOB FREERTOS_PORTABLE_SOURCES
    ${OSAL_FREERTOS_SRC_DIR}/portable/GCC/ARM_CM4F/*.c
    ${OSAL_FREERTOS_SRC_DIR}/portable/MemMang/heap_4.c
)


if(OSAL_RAMDISK_FILESYSTEM_IS_MFS)
    message("*** Using OSAL_XILINX_MFS_SRC_DIR '${OSAL_XILINX_MFS_SRC_DIR}'.")
    # Xilinx Memory Filesystem
    FILE(GLOB FILESYSTEM_SOURCES
        ${OSAL_XILINX_MFS_SRC_DIR}/src/mfs_filesys.c
    )
else()
    message("*** Using OSAL_FREERTOS_PLUS_FAT_SRC_DIR '${OSAL_FREERTOS_PLUS_FAT_SRC_DIR}'.")
    # FreeRTOS + FAT Filesystem
    FILE(GLOB FILESYSTEM_SOURCES
        ${OSAL_FREERTOS_PLUS_FAT_SRC_DIR}/*.c
        ${OSAL_FREERTOS_PLUS_FAT_SRC_DIR}/portable/common/*.c
    )
endif()


include_directories(
    src
)

# FreeRTOS BSP vendored code
include_directories(
    vendor
)



FILE(GLOB VENDOR_CODE
    ../shared-freertos/vendor/*.c
    vendor/*.c
    vendor/init/startup_stm32f767xx.s
    vendor/STM32F7xx_HAL_Driver/Src/*.c
)

FILE(GLOB SOURCES
    ../shared-freertos/src/bsp_start.c
    ../shared-freertos/src/bsp_console.c
)


add_library(osal_nucleo-f767-freertos_impl OBJECT
    ${FREERTOS_SOURCES}
    ${FREERTOS_PORTABLE_SOURCES}
    ${FILESYSTEM_SOURCES}
    ${VENDOR_CODE}
    ${SOURCES}
)

set(OSAL_EXPECTED_OSTYPE "freertos" PARENT_SCOPE)
