/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
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
 *        lsaldap_p.h
 *
 * Abstract:
 *
 *        Likewise Security and Authentication Subsystem (LSASS)
 *
 *        LDAP API (Private Header)
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *          Sriram Nambakam (snambakam@likewisesoftware.com)
 *          Wei Fu (wfu@likewisesoftware.com)
 *          Kyle Stemen (kstemen@likewisesoftware.com)
 *          Brian Dunstan (bdunstan@likewisesoftware.com)
 */
#ifndef __LWLDAP_P_H__
#define __LWLDAP_P_H__

typedef struct _LSA_LDAP_DIRECTORY_CONTEXT {
    LDAP *ld;
} LSA_LDAP_DIRECTORY_CONTEXT, *PLSA_LDAP_DIRECTORY_CONTEXT;

static
DWORD
LwLdapOpenDirectoryWithReaffinity(
    IN PCSTR pszDnsDomainOrForestName,
    IN DWORD dwFlags,
    IN BOOLEAN bNeedGc,
    OUT PHANDLE phDirectory
    );

static
DWORD
LwLdapBindDirectoryAnonymous(
    HANDLE hDirectory
    );

static
DWORD
LwLdapBindDirectory(
    HANDLE hDirectory,
    PCSTR pszServerName
    );

static
DWORD
LwLdapOpenDirectoryServerSingleAttempt(
    IN PCSTR pszServerAddress,
    IN PCSTR pszServerName,
    IN DWORD dwTimeoutSec,
    IN DWORD dwFlags,
    OUT PAD_DIRECTORY_CONTEXT* ppDirectory
    );

void display_status(char *msg, OM_uint32 maj_stat, OM_uint32 min_stat);

void display_status_1(char *m, OM_uint32 code, int type);

#ifndef WIN32
typedef gss_ctx_id_t CtxtHandle, *PCtxtHandle;
#endif

#endif /* __LWLDAP_P_H__ */
