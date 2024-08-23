#ifndef INCLUDE_OS_IMPL_CONSOLE_H
#define INCLUDE_OS_IMPL_CONSOLE_H

#include <osconfig.h>
#include "os-freertos.h"
#include "os-shared-idmap.h"
#include "os-shared-binsem.h"

// included in portable/os-impl-console-bsp.c

// @FIXME
// FBV 2023-12-20 Move PSP prototype to a proper header
int32 PSP_Console_Init(void); 

#endif /* INCLUDE_OS_IMPL_CONSOLE_H */
