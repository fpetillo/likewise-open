/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 */

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

#ifndef __SRV_SHARELST_H__
#define __SRV_SHARELST_H__


NTSTATUS
SrvShareInitContextContents(
    PLWIO_SRV_SHARE_LIST pShareList
    );


NTSTATUS
SrvShareGetServiceStringId(
    SHARE_SERVICE  service,
    PSTR*          ppszService
    );

NTSTATUS
SrvShareGetServiceId(
    PCSTR          pszService,
    SHARE_SERVICE* pService
    );

VOID
SrvShareFreeContextContents(
    PLWIO_SRV_SHARE_LIST pShareList
    );


NTSTATUS
SrvFindShareByName(
    PLWIO_SRV_SHARE_LIST pShareList,
    PWSTR pszShareName,
    PSHARE_DB_INFO *ppShareInfo
    );


NTSTATUS
SrvShareAddShare(
    PLWIO_SRV_SHARE_LIST pShareList,
    PWSTR  pwszShareName,
    PWSTR  pwszPath,
    PWSTR  pwszComment,
    PBYTE  pSecDesc,
    ULONG  ulSecDescLen,
    ULONG  ulShareType
    );


NTSTATUS
SrvShareDeleteShare(
    PLWIO_SRV_SHARE_LIST pShareList,
    PWSTR pwszShareName
    );


NTSTATUS
SrvShareSetInfo(
    PLWIO_SRV_SHARE_LIST pShareList,
    PWSTR pwszShareName,
    PSHARE_DB_INFO pShareInfo
    );


NTSTATUS
SrvShareGetInfo(
    PLWIO_SRV_SHARE_LIST pShareList,
    PWSTR pwszShareName,
    PSHARE_DB_INFO *ppShareInfo
    );


NTSTATUS
SrvShareEnumShares(
    PLWIO_SRV_SHARE_LIST pShareList,
    DWORD dwLevel,
    PSHARE_DB_INFO** pppShareInfo,
    PDWORD pdwNumEntries
    );


#endif /* __SRV_SHARELST_H__ */


/*
local variables:
mode: c
c-basic-offset: 4
indent-tabs-mode: nil
tab-width: 4
end:
*/
