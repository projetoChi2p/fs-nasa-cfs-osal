/*
 *  NASA Docket No. GSC-18,370-1, and identified as "Operating System Abstraction Layer"
 *
 *
 *  Copyright (c) 2024 Universidade Federal do Rio Grande do Sul
 *
 *
 *  Copyright (c) 2021 Patrick Paul
 *  SPDX-License-Identifier: MIT-0
 *
 *
 *  Copyright (c) 2019 United States Government as represented by
 *  the Administrator of the National Aeronautics and Space Administration.
 *  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * \file     os-impl-dirs.c
 * \ingroup  freertos
 * \author   joseph.p.hickey@nasa.gov
 * \author   Patrick Paul
 * \author   Fabio Benevenuti
 */


#include <string.h>
#include <unistd.h>
#include <sys/types.h>


#include "common_types.h"
#include "osapi.h"
//#include "osapi-os-core.h"
//#include "osapi-os-filesys.h"
#include "os-shared-filesys.h"
#include "os-shared-idmap.h"
#include "os-shared-globaldefs.h"

#include "os-freertos.h"

/* freertos-plus-fat */
#include "portable/common/ff_ramdisk.h"
#include "include/ff_stdio.h"
#include "include/ff_headers.h"


/****************************************************************************************
                                   Data Types
****************************************************************************************/


/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/


/****************************************************************************************
                         IMPLEMENTATION-SPECIFIC ROUTINES
             These are specific to this particular operating system
 ****************************************************************************************/

/****************************************************************************************
                                Filesys API
 ***************************************************************************************/

/* --------------------------------------------------------------------------------------
    Name: OS_FreeRTOS_DirAPI_Impl_Init

    Purpose: Directory subsystem global initialization

    Returns: OS_SUCCESS if success
 ---------------------------------------------------------------------------------------*/
int32 OS_FreeRTOS_DirAPI_Impl_Init(void)
{
    // OS_DebugPrintf(1, __func__, __LINE__, "OS_FreeRTOS_DirAPI_Impl_Init()\n");
    return OS_SUCCESS;
}



/*----------------------------------------------------------------
 *
 * Function: OS_DirCreate_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_DirCreate_Impl(const char *local_path, uint32 access)
{
    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");

    return OS_ERR_NOT_IMPLEMENTED;
} /* end OS_DirCreate_Impl */


/*----------------------------------------------------------------
 *
 * Function: OS_DirOpen_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_DirOpen_Impl(const OS_object_token_t *token, const char *local_path)
{
    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");

    return OS_ERR_NOT_IMPLEMENTED;
} /* end OS_DirOpen_Impl */



/*----------------------------------------------------------------
 *
 * Function: OS_DirClose_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_DirClose_Impl(const OS_object_token_t *token)
{
    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");

    return OS_ERR_NOT_IMPLEMENTED;
} /* end OS_DirClose_Impl */



/*----------------------------------------------------------------
 *
 * Function: OS_DirRead_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_DirRead_Impl(const OS_object_token_t *token, os_dirent_t *dirent)
{
    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");

    return OS_ERR_NOT_IMPLEMENTED;
} /* end OS_DirRead_Impl */



/*----------------------------------------------------------------
 *
 * Function: OS_DirRewind_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_DirRewind_Impl(const OS_object_token_t *token)
{
    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");

    return OS_ERR_NOT_IMPLEMENTED;
} /* end OS_DirRewind_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_DirRemove_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_DirRemove_Impl(const char *local_path)
{
    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");

    return OS_ERR_NOT_IMPLEMENTED;
} /* end OS_DirRemove_Impl */
