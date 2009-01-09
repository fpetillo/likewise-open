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

#ifndef __EXTERNS_H__
#define __EXTERNS_H__

extern LWMsgProtocol* gpSMBProtocol;

DWORD
SMBGetClientContext(
    PSMB_CLIENT_CONTEXT* ppContext
    );

DWORD
SMBAcquireConnection(
    OUT PSMB_SERVER_CONNECTION pConnection
    );

VOID
SMBReleaseConnection(
    IN OUT PSMB_SERVER_CONNECTION pConnection
    );

DWORD
SMBAcquireState(
    OUT PSMB_SERVER_CONNECTION pConnection,
    OUT PSMB_CLIENT_CONTEXT* ppContext
    );

VOID
SMBReleaseState(
    IN OUT PSMB_SERVER_CONNECTION pConnection,
    IN OUT PSMB_CLIENT_CONTEXT pContext
    );

DWORD
SMBAPIHandleGetSecurityToken(
    HANDLE hHandle,
    PSMB_SECURITY_TOKEN_REP* ppSecurityToken
    );

DWORD
SMBAPIHandleFreeSecurityToken(
    HANDLE hHandle
    );

#endif /* __EXTERNS_H__ */
