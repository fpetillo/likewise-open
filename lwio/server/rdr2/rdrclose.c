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
 *        close.c
 *
 * Abstract:
 *
 *        Likewise SMB Redirector File System Driver (RDR)
 *
 *        Close Dispatch Function
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *          Sriram Nambakam (snambakam@likewisesoftware.com)
 */

#include "rdr.h"

static
PRDR_CONTINUATION
RdrFinishClose(
    PRDR_CONTINUATION pContinuation,
    NTSTATUS status,
    PVOID pParam
    );

static
VOID
RdrCancelClose(
    PIRP pIrp,
    PVOID pContext
    )
{
    return;
}

NTSTATUS
RdrClose(
    IO_DEVICE_HANDLE DeviceHandle,
    PIRP pIrp
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PSMB_CLIENT_FILE_HANDLE pFile = IoFileGetContext(pIrp->FileHandle);
    PCLOSE_REQUEST_HEADER pHeader = NULL;
    PRDR_IRP_CONTEXT pContext = NULL;

    ntStatus = RdrCreateContext(pIrp, &pContext);
    BAIL_ON_NT_STATUS(ntStatus);

    if (pFile->fid)
    {
        IoIrpMarkPending(pIrp, RdrCancelClose, NULL);

        ntStatus = SMBPacketBufferAllocate(
            pFile->pTree->pSession->pSocket->hPacketAllocator,
            1024*64,
            &pContext->Packet.pRawBuffer,
            &pContext->Packet.bufferLen);
        BAIL_ON_NT_STATUS(ntStatus);

        ntStatus = SMBPacketMarshallHeader(
            pContext->Packet.pRawBuffer,
            pContext->Packet.bufferLen,
            COM_CLOSE,
            0,
            0,
            pFile->pTree->tid,
            gRdrRuntime.SysPid,
            pFile->pTree->pSession->uid,
            0,
            TRUE,
            &pContext->Packet);
        BAIL_ON_NT_STATUS(ntStatus);

        pContext->Packet.pData = pContext->Packet.pParams + sizeof(CLOSE_REQUEST_HEADER);
        pContext->Packet.bufferUsed += sizeof(CLOSE_REQUEST_HEADER);

        pContext->Packet.pSMBHeader->wordCount = 3;

        pHeader = (PCLOSE_REQUEST_HEADER) pContext->Packet.pParams;

        pHeader->fid = SMB_HTOL16(pFile->fid);
        pHeader->ulLastWriteTime = SMB_HTOL32(0);
        pHeader->byteCount = SMB_HTOL16(0);

        ntStatus = SMBPacketMarshallFooter(&pContext->Packet);
        BAIL_ON_NT_STATUS(ntStatus);

        pContext->Continuation.Function = RdrFinishClose;
        pContext->Continuation.pContext = pContext;

        ntStatus = RdrSocketTransceive(
            pFile->pTree->pSession->pSocket,
            pContext);
        BAIL_ON_NT_STATUS(ntStatus);
    }
    else
    {
        RdrReleaseFile(pFile);
    }


cleanup:

    if (ntStatus != STATUS_PENDING)
    {
        RdrFreeContext(pContext);
    }

    return ntStatus;

error:

    goto cleanup;
}

static
PRDR_CONTINUATION
RdrFinishClose(
    PRDR_CONTINUATION pContinuation,
    NTSTATUS status,
    PVOID pParam
    )
{
    PRDR_IRP_CONTEXT pContext = pContinuation->pContext;
    PSMB_PACKET pPacket = pParam;
    PIRP pIrp = pContext->pIrp;
    PSMB_CLIENT_FILE_HANDLE pFile = IoFileGetContext(pIrp->FileHandle);

    if (status == STATUS_SUCCESS)
    {
        status = pPacket->pSMBHeader->error;
    }

    pIrp->IoStatusBlock.Status = status;
    IoIrpComplete(pIrp);
    RdrReleaseFile(pFile);
    RdrFreeContext(pContext);

    /* FIXME: free packet */
    /* FIXME: handle continuation chaining */
    return NULL;
}

void
RdrReleaseFile(
    PSMB_CLIENT_FILE_HANDLE pFile
    )
{
    if (pFile->pTree)
    {
        SMBTreeRelease(pFile->pTree);
    }

    if (pFile->pMutex)
    {
        pthread_mutex_destroy(pFile->pMutex);
    }

    RTL_FREE(&pFile->pwszPath);
    RTL_FREE(&pFile->find.pBuffer);

    SMBFreeMemory(pFile);
}
