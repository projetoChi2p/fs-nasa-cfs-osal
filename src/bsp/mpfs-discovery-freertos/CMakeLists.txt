
message("*** Inside cmake list ${CMAKE_CURRENT_LIST_FILE}.")
message("*** Using OSAL_FREERTOS_SRC_DIR '${OSAL_FREERTOS_SRC_DIR}'.")
message("*** Using OSAL_FREERTOS_INC_DIR '${OSAL_FREERTOS_INC_DIR}'.")
message("*** Using OSAL_FREERTOS_PLUS_FAT_SRC_DIR '${OSAL_FREERTOS_PLUS_FAT_SRC_DIR}'.")


include_directories(
    polarfire_hal/platform
    polarfire_hal/boards/mpfs-discovery-kit/
    polarfire_hal/boards/mpfs-discovery-kit/platform_config/lim-release

    ${OSAL_FREERTOS_SRC_DIR}/portable/GCC/RISC-V
    ${OSAL_FREERTOS_SRC_DIR}/portable/GCC/RISC-V/chip_specific_extensions/RISCV_MTIME_CLINT_no_extensions
    ${OSAL_FREERTOS_SRC_DIR}/include

    ../shared-freertos/src
    ../shared-freertos/src
    ../shared-freertos/vendor
)

file(GLOB VENDOR_CODE
    polarfire_hal/platform/mpfs_hal/startup_gcc/*.c
    polarfire_hal/platform/mpfs_hal/startup_gcc/*.S
    polarfire_hal/platform/mpfs_hal/startup_gcc/*.c
    polarfire_hal/platform/mpfs_hal/common/*.c
    polarfire_hal/platform/mpfs_hal/common/nwc/*.c
    polarfire_hal/platform/hal/*.c
    polarfire_hal/platform/hal/*.S
    polarfire_hal/platform/drivers/mss/mss_mmuart/*.c
    polarfire_hal/platform/drivers/mss/mss_gpio/*.c
    vector_table.S
    app_helpers_board.c
)

file(GLOB FREERTOS_SOURCES
    ${OSAL_FREERTOS_SRC_DIR}/*.c
    ${OSAL_FREERTOS_SRC_DIR}/portable/GCC/RISC-V/*.c
    ${OSAL_FREERTOS_SRC_DIR}/portable/GCC/RISC-V/*.S
    ${OSAL_FREERTOS_SRC_DIR}/portable/MemMang/heap_4.c
)

FILE(GLOB SOURCES
    ../shared-freertos/src/bsp_start.c
    ../shared-freertos/src/bsp_console.c
    ../shared-freertos/vendor/*.c
)

FILE(GLOB FATFS_SOURCES
    ${OSAL_FREERTOS_PLUS_FAT_SRC_DIR}/*.c
    ${OSAL_FREERTOS_PLUS_FAT_SRC_DIR}/portable/common/*.c
)

add_library(osal_mpfs-discovery-freertos_impl OBJECT
    ${FREERTOS_SOURCES}
    ${VENDOR_CODE}
    ${FATFS_SOURCES}
    ${SOURCES} 
)

set(OSAL_EXPECTED_OSTYPE "freertos" PARENT_SCOPE)

#[[
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET_NAME}> ${TARGET_NAME}.hex
    COMMAND ${CMAKE_OBJDUMP} --source --all-headers --demangle --line-numbers --wide $<TARGET_FILE:${TARGET_NAME}> > ${TARGET_NAME}.lst
    COMMAND ${CMAKE_SIZE} --format=sysv --radix=16 $<TARGET_FILE:${TARGET_NAME}>
    COMMENT "Generating hex, lst, and size files"
)
]]

#[[
add_custom_target(pre-build
    COMMAND /home/luis/Microchip/SoftConsole-v2022.2-RISC-V-747/python3/bin/python3 ../src/platform/soc_config_generator/mpfs_configuration_generator.py ../src/boards/mpfs-discovery-kit/fpga_design/design_description/ ../src/boards/mpfs-discovery-kit
    COMMENT "Generating configuration header files from the XML hardware configuration"
)
]]

# add_dependencies(${TARGET_NAME} pre-build)

# set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${TARGET_NAME}.hex;${TARGET_NAME}.lst;${TARGET_NAME}.siz;${TARGET_NAME}.map")


