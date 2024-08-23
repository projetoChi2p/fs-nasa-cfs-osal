# Remove osapi-debug.c
# User application provides custom OS_DebugPrintf()
# list(REMOVE_ITEM OSAL_SRCLIST src/os/shared/src/osapi-debug.c)
#list(REMOVE_ITEM OSAL_SRCLIST src/os/shared/src/osapi-module.c)


set(OSAL_SRCLIST ${OSAL_SRCLIST} PARENT_SCOPE)
message("${OSAL_SRCLIST}")
set_property(TARGET osal PROPERTY SOURCES "${OSAL_SRCLIST};$<TARGET_OBJECTS:osal_${OSAL_SYSTEM_OSTYPE}_impl>")
