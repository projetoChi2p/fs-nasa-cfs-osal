#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "common_types.h"
#include "osapi.h"
//#include "osapi-os-core.h"
#include "os-shared-globaldefs.h"
#include "os-freertos.h"


int32 OS_FreeRTOS_ModuleAPI_Impl_Init(void){
    return OS_SUCCESS;  // @FIXME
}

#if 0
int32 OS_ModuleAPI_Init(void){
    return OS_SUCCESS;  // @FIXME
}

int32 OS_ModuleLoad(osal_id_t *module_id, const char *module_name, const char *filename, uint32 flags){
    return OS_ERROR;  // @FIXME
}

int32 OS_ModuleUnload(osal_id_t module_id){
    return OS_ERROR;  // @FIXME
}

int32 OS_ModuleGetInfo(void){
    return OS_ERROR;  // @FIXME
}

int32 OS_ModuleInfo(osal_id_t module_id, OS_module_prop_t *module_info){
    return OS_ERROR;  // @FIXME
}

int32 OS_SymbolLookup(cpuaddr *symbol_address, const char *symbol_name){
    return -1;
}
#endif