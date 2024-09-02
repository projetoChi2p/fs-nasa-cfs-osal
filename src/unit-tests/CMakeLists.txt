# CMake snippet for OSAL unit tests
#
# This is a set of functional tests for certain OSAL subsystems,
# focused on API responses and implemented as black-box tests (i.e.
# linked to normal OSAL, only using public API).
#

enable_testing()

include_directories(inc)

# nucleo f767 does not support osal_core_UT, it overflows RAM
if (NOT (OSAL_SYSTEM_BSPTYPE STREQUAL "nucleo-f767-freertos"))
    add_subdirectory(oscore-test)
endif()
add_subdirectory(osloader-test)
add_subdirectory(osfilesys-test)
add_subdirectory(osfile-test)
add_subdirectory(osnetwork-test)
add_subdirectory(ostimer-test)
