
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
    ${OSAL_FREERTOS_SRC_DIR}/portable/GCC/RISC-V/*.c
    ${OSAL_FREERTOS_SRC_DIR}/portable/GCC/RISC-V/*.S
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

# Define drivers used by the application
set(CONFIG_MSS_MMUART   TRUE)
set(CONFIG_MSS_GPIO     TRUE)
set(CONFIG_MSS_MMC      TRUE)
set(CONFIG_MSS_RTC      TRUE)
set(CONFIG_MSS_WATCHDOG TRUE)

add_subdirectory(platform)

# The 'src/platform' subdirectory defines variables in its own scope.
# Pull them here to make them visible to the main project.
get_directory_property(PLATFORM_SRC_FROM_SUBDIR
    DIRECTORY platform
    DEFINITION PLATFORM_SRC
)

get_directory_property(PLATFORM_HEADERS_FROM_SUBDIR
    DIRECTORY platform
    DEFINITION PLATFORM_HEADERS
)

get_directory_property(LIB_MSS_HEADERS_FROM_SUBDIR
    DIRECTORY platform
    DEFINITION LIB_MSS_HEADERS
)

set(PLATFORM_SRC ${PLATFORM_SRC_FROM_SUBDIR})
set(PLATFORM_HEADERS ${PLATFORM_HEADERS_FROM_SUBDIR})
set(LIB_MSS_HEADERS ${LIB_MSS_HEADERS_FROM_SUBDIR})

include_directories(
    ${PLATFORM_HEADERS}
    ${LIB_MSS_HEADERS}

    ${OSAL_FREERTOS_SRC_DIR}/portable/GCC/RISC-V
    ${OSAL_FREERTOS_SRC_DIR}/portable/GCC/RISC-V/chip_specific_extensions/RISCV_MTIME_CLINT_no_extensions
    ${OSAL_FREERTOS_SRC_DIR}/include

    ../shared-freertos/src
    ../shared-freertos/src
    ../shared-freertos/vendor
)


FILE(GLOB VENDOR_CODE
    ../shared-freertos/vendor/*.c
    vendor/*.c
    app_helpers_board.c
    middleware/fatfs/*.c
    middleware/vector_table.S
)

FILE(GLOB SOURCES
    ../shared-freertos/src/bsp_start.c
    ../shared-freertos/src/bsp_console.c
)

add_library(osal_mpfs-discovery-freertos_impl OBJECT
    ${FREERTOS_SOURCES}
    ${FREERTOS_PORTABLE_SOURCES}
    ${FILESYSTEM_SOURCES}
    ${VENDOR_CODE}
    ${SOURCES}
    ${PLATFORM_SRC}
)

set(OSAL_EXPECTED_OSTYPE "freertos" PARENT_SCOPE)
