/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the license, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.  You should have received a copy
 * of the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * LESSER GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        ntipc.h
 *
 * Abstract:
 *
 *        NT lwmsg IPC Implementaion
 *
 * Authors: Danilo Almeida (dalmeida@likewisesoftware.com)
 */

#define LWMSG_SPEC_DEBUG

#include "includes.h"
#include "ntipcmsg.h"

#define _LWMSG_MEMBER_BUFFER(Type, BufferField, LengthField) \
    LWMSG_MEMBER_POINTER_BEGIN(Type, BufferField), \
    LWMSG_UINT8(BYTE), \
    LWMSG_POINTER_END, \
    LWMSG_ATTR_LENGTH_MEMBER(Type, LengthField)

static
LWMsgTypeSpec gNtIpcTypeSpecIoFileName[] =
{
    LWMSG_STRUCT_BEGIN(IO_FILE_NAME),
    LWMSG_MEMBER_HANDLE(IO_FILE_NAME, RootFileHandle, IO_FILE_HANDLE),
    LWMSG_MEMBER_PWSTR(IO_FILE_NAME, FileName),
    LWMSG_MEMBER_UINT32(IO_FILE_NAME, IoNameOptions),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static
LWMsgTypeSpec gNtIpcTypeSpecMessageGenericFile[] =
{
    LWMSG_STRUCT_BEGIN(NT_IPC_MESSAGE_GENERIC_FILE),
    LWMSG_MEMBER_HANDLE(NT_IPC_MESSAGE_GENERIC_FILE, FileHandle, IO_FILE_HANDLE),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static
LWMsgTypeSpec gNtIpcTypeSpecMessageGenericControlFile[] =
{
    LWMSG_STRUCT_BEGIN(NT_IPC_MESSAGE_GENERIC_CONTROL_FILE),
    LWMSG_MEMBER_HANDLE(NT_IPC_MESSAGE_GENERIC_CONTROL_FILE, FileHandle, IO_FILE_HANDLE),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_GENERIC_CONTROL_FILE, ControlCode),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_GENERIC_CONTROL_FILE, InputBufferLength),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_GENERIC_CONTROL_FILE, OutputBufferLength),
    _LWMSG_MEMBER_BUFFER(NT_IPC_MESSAGE_GENERIC_CONTROL_FILE, InputBuffer, InputBufferLength),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static
LWMsgTypeSpec gNtIpcTypeSpecMessageGenericFileIoResult[] =
{
    LWMSG_STRUCT_BEGIN(NT_IPC_MESSAGE_GENERIC_FILE_IO_RESULT),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_GENERIC_FILE_IO_RESULT, Status),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_GENERIC_FILE_IO_RESULT, BytesTransferred),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static
LWMsgTypeSpec gNtIpcTypeSpecMessageGenericFileBufferResult[] =
{
    LWMSG_STRUCT_BEGIN(NT_IPC_MESSAGE_GENERIC_FILE_BUFFER_RESULT),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_GENERIC_FILE_BUFFER_RESULT, Status),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_GENERIC_FILE_BUFFER_RESULT, BytesTransferred),
    _LWMSG_MEMBER_BUFFER(NT_IPC_MESSAGE_GENERIC_FILE_BUFFER_RESULT, Buffer, BytesTransferred),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static
LWMsgTypeSpec gNtIpcTypeSpecMessageCreateFile[] =
{
    LWMSG_STRUCT_BEGIN(NT_IPC_MESSAGE_CREATE_FILE),
    LWMSG_MEMBER_PSECTOKEN(NT_IPC_MESSAGE_CREATE_FILE, pSecurityToken),
    LWMSG_MEMBER_TYPESPEC(NT_IPC_MESSAGE_CREATE_FILE, FileName, gNtIpcTypeSpecIoFileName),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_CREATE_FILE, DesiredAccess),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_CREATE_FILE, AllocationSize),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_CREATE_FILE, FileAttributes),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_CREATE_FILE, ShareAccess),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_CREATE_FILE, CreateDisposition),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_CREATE_FILE, CreateOptions),
    // TODO -- Add stuff for EAs, SDs, etc.
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static
LWMsgTypeSpec gNtIpcTypeSpecMessageCreateFileResult[] =
{
    LWMSG_STRUCT_BEGIN(NT_IPC_MESSAGE_CREATE_FILE_RESULT),
    LWMSG_MEMBER_HANDLE(NT_IPC_MESSAGE_CREATE_FILE_RESULT, FileHandle, IO_FILE_HANDLE),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_CREATE_FILE_RESULT, Status),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_CREATE_FILE_RESULT, CreateResult),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static
LWMsgTypeSpec gNtIpcTypeSpecMessageReadFile[] =
{
    LWMSG_STRUCT_BEGIN(NT_IPC_MESSAGE_READ_FILE),
    LWMSG_MEMBER_HANDLE(NT_IPC_MESSAGE_READ_FILE, FileHandle, IO_FILE_HANDLE),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_READ_FILE, Length),
    LWMSG_MEMBER_POINTER(NT_IPC_MESSAGE_READ_FILE, ByteOffset, LWMSG_INT64(ULONG64)),
    LWMSG_MEMBER_POINTER(NT_IPC_MESSAGE_READ_FILE, Key, LWMSG_UINT32(ULONG)),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static
LWMsgTypeSpec gNtIpcTypeSpecMessageWriteFile[] =
{
    LWMSG_STRUCT_BEGIN(NT_IPC_MESSAGE_WRITE_FILE),
    LWMSG_MEMBER_HANDLE(NT_IPC_MESSAGE_WRITE_FILE, FileHandle, IO_FILE_HANDLE),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_WRITE_FILE, Length),
    _LWMSG_MEMBER_BUFFER(NT_IPC_MESSAGE_WRITE_FILE, Buffer, Length),
    LWMSG_MEMBER_POINTER(NT_IPC_MESSAGE_WRITE_FILE, ByteOffset, LWMSG_INT64(ULONG64)),
    LWMSG_MEMBER_POINTER(NT_IPC_MESSAGE_WRITE_FILE, Key, LWMSG_UINT32(ULONG)),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static
LWMsgTypeSpec gNtIpcTypeSpecMessageQueryInformationFile[] =
{
    LWMSG_STRUCT_BEGIN(NT_IPC_MESSAGE_QUERY_INFORMATION_FILE),
    LWMSG_MEMBER_HANDLE(NT_IPC_MESSAGE_QUERY_INFORMATION_FILE, FileHandle, IO_FILE_HANDLE),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_QUERY_INFORMATION_FILE, Length),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_QUERY_INFORMATION_FILE, FileInformationClass),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static
LWMsgTypeSpec gNtIpcTypeSpecMessageSetInformationFile[] =
{
    LWMSG_STRUCT_BEGIN(NT_IPC_MESSAGE_SET_INFORMATION_FILE),
    LWMSG_MEMBER_HANDLE(NT_IPC_MESSAGE_SET_INFORMATION_FILE, FileHandle, IO_FILE_HANDLE),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_SET_INFORMATION_FILE, Length),
    _LWMSG_MEMBER_BUFFER(NT_IPC_MESSAGE_SET_INFORMATION_FILE, FileInformation, Length),
    LWMSG_MEMBER_UINT32(NT_IPC_MESSAGE_SET_INFORMATION_FILE, FileInformationClass),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static
LWMsgProtocolSpec gNtIpcProtocolSpec[] =
{
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_CREATE_FILE,        gNtIpcTypeSpecMessageCreateFile),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_CREATE_FILE_RESULT, gNtIpcTypeSpecMessageCreateFileResult),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_CLOSE_FILE,         gNtIpcTypeSpecMessageGenericFile),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_CLOSE_FILE_RESULT,  gNtIpcTypeSpecMessageGenericFileIoResult),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_READ_FILE,          gNtIpcTypeSpecMessageReadFile),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_READ_FILE_RESULT,   gNtIpcTypeSpecMessageGenericFileBufferResult),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_WRITE_FILE,         gNtIpcTypeSpecMessageWriteFile),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_WRITE_FILE_RESULT,  gNtIpcTypeSpecMessageGenericFileIoResult),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_DEVICE_IO_CONTROL_FILE,        gNtIpcTypeSpecMessageGenericControlFile),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_DEVICE_IO_CONTROL_FILE_RESULT, gNtIpcTypeSpecMessageGenericFileBufferResult),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_FS_CONTROL_FILE,               gNtIpcTypeSpecMessageGenericControlFile),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_FS_CONTROL_FILE_RESULT,        gNtIpcTypeSpecMessageGenericFileBufferResult),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_FLUSH_BUFFERS_FILE,            gNtIpcTypeSpecMessageGenericFile),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_FLUSH_BUFFERS_FILE_RESULT,     gNtIpcTypeSpecMessageGenericFileIoResult),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_QUERY_INFORMATION_FILE,        gNtIpcTypeSpecMessageQueryInformationFile),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_QUERY_INFORMATION_FILE_RESULT, gNtIpcTypeSpecMessageGenericFileBufferResult),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_SET_INFORMATION_FILE,          gNtIpcTypeSpecMessageSetInformationFile),
    LWMSG_MESSAGE(NT_IPC_MESSAGE_TYPE_SET_INFORMATION_FILE_RESULT,   gNtIpcTypeSpecMessageGenericFileIoResult),
    LWMSG_PROTOCOL_END
};

LWMsgProtocolSpec*
NtIpcGetProtocolSpec(
    VOID
    )
{
    return gNtIpcProtocolSpec;
}

NTSTATUS
NtIpcLWMsgStatusToNtStatus(
    IN LWMsgStatus LwMsgStatus
    )
{
    NTSTATUS status = 0;

    switch (LwMsgStatus)
    {
        case LWMSG_STATUS_SUCCESS:
            status = STATUS_SUCCESS;
            break;
        case LWMSG_STATUS_ERROR:
            status = STATUS_UNSUCCESSFUL;
            break;
        case LWMSG_STATUS_MEMORY:
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        case LWMSG_STATUS_EOF:
            status = STATUS_END_OF_FILE;
            break;
        case LWMSG_STATUS_NOT_FOUND:
            status = STATUS_NOT_FOUND;
            break;
        case LWMSG_STATUS_UNIMPLEMENTED:
            status = STATUS_NOT_IMPLEMENTED;
            break;
        case LWMSG_STATUS_INVALID_PARAMETER:
            status = STATUS_INVALID_PARAMETER;
            break;
        case LWMSG_STATUS_OVERFLOW:
            status = STATUS_INTEGER_OVERFLOW;
            break;
        case LWMSG_STATUS_UNDERFLOW:
            // TODO -- check this:
            status = STATUS_INTEGER_OVERFLOW;
            break;
        case LWMSG_STATUS_TIMEOUT:
            status = STATUS_IO_TIMEOUT;
            break;
        case LWMSG_STATUS_SECURITY:
            status = STATUS_ACCESS_DENIED;
            break;
        case LWMSG_STATUS_FILE_NOT_FOUND:
            // Cannot use either of these because we do not know
            // whether the last component or some intermediate component
            // was not found.
            // - STATUS_OBJECT_NAME_NOT_FOUND
            // - STATUS_OBJECT_PATH_NOT_FOUND
            // So we use this:
            // - STATUS_NOT_FOUND
            // TODO  -- it may be that we should do STATUS_OBJECT_NAME_NOT_FOUND
            // (need to check Win32 for that).
            status = STATUS_NOT_FOUND;
            break;

        // TODO -- map these lwmsg status codes:
        case LWMSG_STATUS_AGAIN:
        case LWMSG_STATUS_MALFORMED:
        case LWMSG_STATUS_SYSTEM:
        case LWMSG_STATUS_INTERRUPT:
        case LWMSG_STATUS_CONNECTION_REFUSED:
        case LWMSG_STATUS_INVALID_STATE: // STATUS_INTERNAL?
            status = STATUS_UNSUCCESSFUL;
            break;

        default:
            SMB_LOG_ERROR("Failed to map lwmsg error %", LwMsgStatus);
            status = STATUS_NONE_MAPPED;
            break;
    }

    return status;
}

