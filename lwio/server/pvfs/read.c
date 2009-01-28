/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.  You should have received a copy of the GNU General
 * Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */



/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        read.c
 *
 * Abstract:
 *
 *        Likewise Posix File System Driver (PVFS)
 *
 *       Read Dispatch Routine
 *
 * Authors: Gerald Carter <gcarter@likewise.com>
 */

#include "pvfs.h"

NTSTATUS
PvfsRead(
    IO_DEVICE_HANDLE IoDeviceHandle,
    PPVFS_IRP_CONTEXT pIrpContext
    )
{
    NTSTATUS ntError = STATUS_UNSUCCESSFUL;
    PIRP pIrp = pIrpContext->pIrp;
    PVOID pBuffer = pIrp->Args.ReadWrite.Buffer;
    ULONG bufLen = pIrp->Args.ReadWrite.Length;
    PPVFS_CCB pCcb = NULL;
    size_t totalBytesRead = 0;

    BAIL_ON_INVALID_PTR(pBuffer, ntError);

    pCcb = (PPVFS_CCB)IoFileGetContext(pIrp->FileHandle);
    PVFS_BAIL_ON_INVALID_CCB(pCcb, ntError);

#if 0xFFFFFFFF > SSIZE_MAX
    if ((size_t) bufLen > (size_t) SSIZE_MAX) {
        ntError = STATUS_INVALID_PARAMETER;
        BAIL_ON_NT_STATUS(ntError);
    }
#endif

    while (totalBytesRead < bufLen)
    {
        size_t bytesRead = 0;

        bytesRead = read(pCcb->fd,
                         pBuffer + totalBytesRead,
                         bufLen - totalBytesRead);
        if (bytesRead == -1) {
            int err = errno;

            /* try again? */
            if (err == EAGAIN) {
                continue;
            }

            ntError = PvfsMapUnixErrnoToNtStatus(err);
            BAIL_ON_NT_STATUS(ntError);
        }

        /* Check for EOF */
        if (bytesRead == 0) {
            break;
        }

        totalBytesRead += bytesRead;
    }

    /* Can only get here is the loop was completed
       successfully */

    pIrp->IoStatusBlock.BytesTransferred = totalBytesRead;

    ntError = STATUS_SUCCESS;


cleanup:
    return ntError;

error:
    goto cleanup;
}


/*
local variables:
mode: c
c-basic-offset: 4
indent-tabs-mode: nil
tab-width: 4
end:
*/
