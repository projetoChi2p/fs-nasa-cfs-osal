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
 * \file     os-impl-time.c
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
                                Clock API
 ***************************************************************************************/

int32 OS_GetLocalTime_Impl(OS_time_t *time_struct)
{
    /*
    ** Not implemented yet
    */
    return OS_ERR_NOT_IMPLEMENTED;    
}


int32 OS_SetLocalTime_Impl(const OS_time_t *time_struct)
{
    /*
    ** Not implemented yet
    */
    return OS_ERR_NOT_IMPLEMENTED;    
}
