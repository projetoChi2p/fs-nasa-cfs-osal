include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

set(FREERTOS_BASE_SOURCES
    src/os-impl-common.c
    src/os-impl-queue.c
    src/os-impl-binsem.c
    src/os-impl-countsem.c
    src/os-impl-mutex.c
    src/os-impl-console.c
    src/os-impl-idmap.c
    src/os-impl-task.c
    src/os-impl-timebase.c
    src/os-impl-errors.c
)

#if(NOT DEFINED CUSTOM_FREERTOS_FILESYSTEM)
#    message(FATAL_ERROR "You must set CUSTOM_FREERTOS_FILESYSTEM for your BSP in arch_build_custom.cmake and provide your own OS_FreeRTOS_FileSysAPI_Impl_Init implementation.")
#endif()

set(FREERTOS_TODO_SOURCES
    todo/os-impl-dirs.c
    todo/os-impl-file.c
    todo/os-impl-filesys.c
    todo/os-impl-module.c
    todo/os-impl-network.c
    todo/os-impl-heap.c
    todo/os-impl-sockets.c
    todo/os-impl-clock.c
)

# From portable/README.txt:
#   Files in this directory contain an implmentation that adheres to a defined API
#   that is applicable to more than one of the supported OS's, but not all.  For
#   example, the BSD-style sockets API is implemented in VxWorks, RTEMS, and Linux.
#   Therefore it is beneficial to put the code in here and let each implementation
#   selectively include this rather than reinventing the wheel.

# Add portable code
set(FREERTOS_IMPL_SOURCES
    ../portable/os-impl-console-bsp.c
)

# @FIXME Disable shell (see VxWorks implementation)
list(APPEND FREERTOS_IMPL_SOURCES
    ../portable/os-impl-no-shell.c
)
# @FIXME Disable module loading (see VxWorks implementation)
list(APPEND FREERTOS_IMPL_SOURCES
    ../portable/os-impl-no-loader.c
    ../portable/os-impl-no-symtab.c
)
# @FIXME Disable network and sockets (see VxWorks implementation)
list(APPEND FREERTOS_IMPL_SOURCES
    ../portable/os-impl-no-network.c    # non-implemented versions of all network APIs
    ../portable/os-impl-no-sockets.c    # non-implemented versions of all socket APIs
    ../portable/os-impl-no-condvar.c    # non-implemented versions of all condition variables APIs
    ../portable/os-impl-no-loader.c     # non-implemented versions of all module subsystem APIs
    ../portable/os-impl-no-select.c     # non-implemented versions of all module subsystem APIs
)

add_library(osal_freertos_impl OBJECT
    ${FREERTOS_BASE_SOURCES}
    ${FREERTOS_TODO_SOURCES}
    ${FREERTOS_IMPL_SOURCES}
)
