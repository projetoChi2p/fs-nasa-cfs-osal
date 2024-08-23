/*
 *  NASA Docket No. GSC-18,370-1, and identified as "Operating System Abstraction Layer"
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
 * \file     os-impl-files.c
 * \ingroup  freertos
 * \author   joseph.p.hickey@nasa.gov
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
****************************************************************************************/

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
#include "os-impl-files.h"
#include "os-shared-file.h"

/* freertos-plus-fat */
#include "portable/common/ff_ramdisk.h"
#include "include/ff_stdio.h"
#include "include/ff_headers.h"


/*
 * The global file handle table.
 *
 * This is shared by all OSAL entities that perform low-level I/O.
 */
OS_impl_file_internal_record_t OS_impl_filehandle_table[OS_MAX_NUM_OPEN_FILES];

/*----------------------------------------------------------------
 *
 * Function: OS_FreeRTOS_StreamAPI_Impl_Init
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
int32 OS_FreeRTOS_StreamAPI_Impl_Init(void)
{
    osal_index_t local_id;

    /*
     * init all filehandles to -1, which is always invalid.
     * this isn't strictly necessary but helps when debugging.
     */
    for (local_id = 0; local_id < OS_MAX_NUM_OPEN_FILES; ++local_id)
    {
        //OS_impl_filehandle_table[local_id].fd         = -1;
        //OS_impl_filehandle_table[local_id].selectable = false;
        OS_impl_filehandle_table[local_id].pxFile     = NULL;
    }

    // OS_DebugPrintf(1, __func__, __LINE__, "return OS_SUCCESS\n");
    return OS_SUCCESS;
} /* end OS_FreeRTOS_StreamAPI_Impl_Init */


/****************************************************************************************
                                 Named File API
 ***************************************************************************************/



/*----------------------------------------------------------------
 *
 * Function: OS_FileOpen_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_FileOpen_Impl(const OS_object_token_t *token, const char *local_path, int32 flags, int32 access)
{
    char os_perm_sz[5];
    OS_impl_file_internal_record_t *impl;

    // OS_DebugPrintf(1, __func__, __LINE__, "path:'%s' flags:%d access:%d\n", 
    // local_path,
    // flags, 
    // access);

    impl = OS_OBJECT_TABLE_GET(OS_impl_filehandle_table, *token);

    //OS_DebugPrintf(1, __func__, __LINE__, "RAM Disk partition:\n");
    //FF_RAMDiskShowPartition(impl->allocated_disk);


    /*
    ** Check for a valid access mode
    ** For creating a file, OS_READ_ONLY does not make sense
    **
    ** See also FreeRTOS FAT FF_GetModeBits()
    */
    switch (access)
    {
        case OS_WRITE_ONLY:
            if (flags & OS_FILE_FLAG_TRUNCATE) 
            {
                strcpy(os_perm_sz,"w");
            }
            else {
                strcpy(os_perm_sz,"a");
            }
            break;
        case OS_READ_ONLY:
            strcpy(os_perm_sz,"r");
            break;
        case OS_READ_WRITE:
            if ( (flags & OS_FILE_FLAG_CREATE) && (flags & OS_FILE_FLAG_TRUNCATE) )
            {
                strcpy(os_perm_sz,"rw");
            }
            else if (flags & OS_FILE_FLAG_CREATE) 
            {
                strcpy(os_perm_sz,"ra");
            }
            else if (flags & OS_FILE_FLAG_TRUNCATE)  
            {
                return OS_ERR_FILE;
            }
            else
            {
                strcpy(os_perm_sz,"r+");
            }
            break;
        default:
            return OS_ERR_FILE;
    }

    impl->pxFile = ff_fopen( local_path, os_perm_sz );
    if ( impl->pxFile == NULL)
    {
        OS_DEBUG("OS_FileOpen_Impl(%s,\"%s\"): ERRNO %d\n", local_path, os_perm_sz, stdioGET_ERRNO());
        return OS_ERROR;
    }

    return OS_SUCCESS;
} /* end OS_FileOpen_Impl */



/*----------------------------------------------------------------
 *
 * Function: OS_GenericRead_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_GenericRead_Impl(const OS_object_token_t *token, void *buffer, size_t nbytes, int32 timeout)
{
    int32                           return_code;
    size_t                          os_result;
    OS_impl_file_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_filehandle_table, *token);

    //OS_DebugPrintf(1, __func__, __LINE__, "bytes:%ld timeout:%d\n", 
    //nbytes, 
    //timeout);

    return_code = OS_SUCCESS;

    if (timeout != OS_PEND) {
        OS_DEBUG("read: timed read not supported.\n");
        return_code = OS_ERR_NOT_IMPLEMENTED;
    }

    if (nbytes > 0)
    {
        if (return_code == OS_SUCCESS)
        {
            os_result = ff_fread(buffer, 1, nbytes, impl->pxFile);
            if ( stdioGET_ERRNO( ) != pdFREERTOS_ERRNO_NONE ) 
            {
                OS_DEBUG("read: %s\n", strerror(stdioGET_ERRNO( )));
                return_code = OS_ERROR;
            }
            else
            {
                /* type conversion from size_t to int32 for return */
                return_code = (int32)os_result;
            }
        }
    }

    //OS_DebugPrintf(1, __func__, __LINE__, "bytes:%ld timeout:%d result:%d\n", 
    //nbytes, 
    //timeout,
    //return_code);

    return (return_code);
} /* end OS_GenericRead_Impl */



/*----------------------------------------------------------------
 *
 * Function: OS_GenericWrite_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_GenericWrite_Impl(const OS_object_token_t *token, const void *buffer, size_t nbytes, int32 timeout)
{
    OS_impl_file_internal_record_t* impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_filehandle_table, *token);

    // TODO: do a better error propagation.
    int result = ff_fwrite(buffer, 1 ,nbytes, impl->pxFile);
    if (result > 0)
    {
        // OS_DebugPrintf(1, __func__, __LINE__, "OS_SUCESS \n");
        return OS_SUCCESS;
    }

    // OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_FILE \n");
    return OS_ERR_FILE;

} /* end OS_GenericWrite_Impl */



/*----------------------------------------------------------------
 *
 * Function: OS_GenericClose_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_GenericClose_Impl(const OS_object_token_t *token)
{
    int                             result;
    OS_impl_file_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_filehandle_table, *token);

    result = ff_fclose(impl->pxFile);
    if (result < 0)
    {
        /*
         * close() can technically fail for various reasons, but
         * there isn't much recourse if this call fails.  Just log
         * the failure for debugging.
         */
        OS_DEBUG("close: %s\n", strerror(stdioGET_ERRNO( )));
    }
    impl->pxFile = NULL;
    return OS_SUCCESS;
} /* end OS_GenericClose_Impl */

