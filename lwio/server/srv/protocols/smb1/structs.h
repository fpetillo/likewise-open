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
 *        structs.h
 *
 * Abstract:
 *
 *        Likewise IO (LWIO) - SRV
 *
 *        Structures
 *
 * Authors: Sriram Nambakam (snambakam@likewise.com)
 *
 */

#ifndef __STRUCTS_H__
#define __STRUCTS_H__

typedef enum
{
    SRV_SMB_UNLOCK = 0,
    SRV_SMB_LOCK
} SRV_SMB_LOCK_OPERATION_TYPE;

typedef struct _SRV_SMB_LOCK_REQUEST* PSRV_SMB_LOCK_REQUEST;

typedef struct _SRV_SMB_LOCK_CONTEXT
{
    SRV_SMB_LOCK_OPERATION_TYPE operation;

    ULONG   ulKey;
    BOOLEAN bExclusiveLock;

    IO_ASYNC_CONTROL_BLOCK  acb;
    PIO_ASYNC_CONTROL_BLOCK pAcb;

    IO_STATUS_BLOCK ioStatusBlock;

    SRV_SMB_LOCK_TYPE lockType;

    union
    {
        LOCKING_ANDX_RANGE_LARGE_FILE largeFileRange;
        LOCKING_ANDX_RANGE            regularRange;

    } lockInfo;

    PSRV_SMB_LOCK_REQUEST pLockRequest;

} SRV_SMB_LOCK_CONTEXT, *PSRV_SMB_LOCK_CONTEXT;

typedef struct _SRV_SMB_LOCK_REQUEST
{
    LONG                  refCount;

    pthread_mutex_t       mutex;
    pthread_mutex_t*      pMutex;

    IO_STATUS_BLOCK       ioStatusBlock;

    ULONG                 ulTimeout;

    USHORT                usNumUnlocks;
    USHORT                usNumLocks;

    PSRV_SMB_LOCK_CONTEXT pLockContexts; /* unlocks and locks */

    LONG                  lPendingContexts;

    BOOLEAN               bExpired;
    BOOLEAN               bCompleted;
    BOOLEAN               bResponseSent;

    PSRV_TIMER_REQUEST    pTimerRequest;

    PSRV_EXEC_CONTEXT     pExecContext_timer;
    PSRV_EXEC_CONTEXT     pExecContext_async;

} SRV_SMB_LOCK_REQUEST;

typedef struct _SRV_CREATE_STATE_SMB_V1
{
    LONG                    refCount;

    pthread_mutex_t         mutex;
    pthread_mutex_t*        pMutex;

    PVOID                   pSecurityDescriptor;
    PVOID                   pSecurityQOS;
    PWSTR                   pwszFilename; // Do not free
    PIO_FILE_NAME           pFilename;
    PIO_ECP_LIST            pEcpList;
    IO_FILE_HANDLE          hFile;

    IO_ASYNC_CONTROL_BLOCK  acb;
    PIO_ASYNC_CONTROL_BLOCK pAcb;

    IO_STATUS_BLOCK         ioStatusBlock;

    PCREATE_REQUEST_HEADER  pRequestHeader; // Do not free

} SRV_CREATE_STATE_SMB_V1, *PSRV_CREATE_STATE_SMB_V1;

typedef struct _SRV_SMB_READ_REQUEST
{
    LONG                    refCount;

    pthread_mutex_t         mutex;
    pthread_mutex_t*        pMutex;

    IO_ASYNC_CONTROL_BLOCK  acb;
    PIO_ASYNC_CONTROL_BLOCK pAcb;

    IO_STATUS_BLOCK         ioStatusBlock;

} SRV_SMB_READ_REQUEST, *PSRV_SMB_READ_REQUEST;

typedef struct _SRV_SMB_WRITE_REQUEST
{
    LONG                    refCount;

    pthread_mutex_t         mutex;
    pthread_mutex_t*        pMutex;

    IO_ASYNC_CONTROL_BLOCK  acb;
    PIO_ASYNC_CONTROL_BLOCK pAcb;

    IO_STATUS_BLOCK         ioStatusBlock;

} SRV_SMB_WRITE_REQUEST, *PSRV_SMB_WRITE_REQUEST;

typedef VOID (*PFN_SRV_MESSAGE_STATE_RELEASE_SMB_V1)(HANDLE hState);

typedef struct __SRV_MESSAGE_SMB_V1
{
    ULONG        ulSerialNum;       // Sequence # in packet; starts from 0

    PBYTE        pBuffer;           // Raw packet buffer

    UCHAR        ucCommand;         // andx or SMB command
    PSMB_HEADER  pHeader;           // header corresponding to serial 0 message
    PBYTE        pWordCount;        // points to word count field in message
                                    // (a) for the serial 0 message, this points
                                    //     to the word count field in the header
                                    // (b) andx messages share the serial 0
                                    //     smb header; but, they have their own
                                    //     word count referenced by this field.
    PANDX_HEADER pAndXHeader;
    USHORT       usHeaderSize;
    ULONG        ulMessageSize;

    ULONG        ulBytesAvailable;

} SRV_MESSAGE_SMB_V1, *PSRV_MESSAGE_SMB_V1;

typedef struct _SRV_EXEC_CONTEXT_SMB_V1
{
    PSRV_MESSAGE_SMB_V1                  pRequests;
    ULONG                                ulNumRequests;
    ULONG                                iMsg;

    PLWIO_SRV_SESSION                    pSession;
    PLWIO_SRV_TREE                       pTree;
    PLWIO_SRV_FILE                       pFile;

    HANDLE                               hState;
    PFN_SRV_MESSAGE_STATE_RELEASE_SMB_V1 pfnStateRelease;

    ULONG                                ulNumResponses;
    PSRV_MESSAGE_SMB_V1                  pResponses;

} SRV_EXEC_CONTEXT_SMB_V1;

typedef struct _SRV_RUNTIME_GLOBALS_SMB_V1
{
    pthread_mutex_t         mutex;

    PSMB_PROD_CONS_QUEUE    pWorkQueue;

} SRV_RUNTIME_GLOBALS_SMB_V1, *PSRV_RUNTIME_GLOBALS_SMB_V1;

#endif /* __STRUCTS_H__ */
