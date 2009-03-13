/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
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

#include "includes.h"


#if !defined(MAXHOSTNAMELEN)
#define MAXHOSTNAMELEN (256)
#endif

NET_API_STATUS NetUnjoinDomainLocal(const wchar16_t *machine, 
                                    const wchar16_t *domain,
                                    const wchar16_t *account,
                                    const wchar16_t *password,
                                    uint32 options)
{
    const uint32 domain_access  = DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                  DOMAIN_ACCESS_OPEN_ACCOUNT |
                                  DOMAIN_ACCESS_LOOKUP_INFO_2 |
                                  DOMAIN_ACCESS_CREATE_USER;
    WINERR err = ERROR_SUCCESS;
    NTSTATUS status = STATUS_SUCCESS;
    PolicyHandle account_h;
    HANDLE hStore = (HANDLE)NULL;
    PLWPS_PASSWORD_INFO pi = NULL;
    NetConn *conn = NULL;
    char localname[MAXHOSTNAMELEN] = {0};
    wchar16_t *domain_controller_name = NULL;
    wchar16_t *machine_name = NULL;
    PIO_ACCESS_TOKEN access_token = NULL;

    goto_if_invalid_param_winerr(machine, cleanup);
    goto_if_invalid_param_winerr(domain, cleanup);
    goto_if_invalid_param_winerr(account, cleanup);
    goto_if_invalid_param_winerr(password, cleanup);

    machine_name = wc16sdup(machine);
    goto_if_no_memory_ntstatus(machine_name, error);

    if (gethostname((char*)localname, sizeof(localname)) < 0) {
       err = ERROR_INTERNAL_ERROR;
       goto error;
    }

    status = NetpGetDcName(domain, FALSE, &domain_controller_name);
    goto_if_ntstatus_not_success(status, error);

    status = LwpsOpenPasswordStore(LWPS_PASSWORD_STORE_DEFAULT, &hStore);
    goto_if_ntstatus_not_success(status, error);

    status = LwpsGetPasswordByHostName(hStore, localname, &pi);
    goto_if_ntstatus_not_success(status, error);

    /* zero the machine password */
    memset((void*)pi->pwszMachinePassword, 0,
           wc16slen(pi->pwszMachinePassword));
    pi->last_change_time = time(NULL);

    status = LwpsWritePasswordToAllStores(pi);
    goto_if_ntstatus_not_success(status, error);

    /* disable the account only if requested */
    if (options & NETSETUP_ACCT_DELETE) {
        if (account && password)
        {
            status = LwIoCreatePlainAccessTokenW(account, password, &access_token);
            goto_if_ntstatus_not_success(status, error);
        }
        else
        {
            status = LwIoGetThreadAccessToken(&access_token);
            goto_if_ntstatus_not_success(status, error);
        }

        status = NetConnectSamr(&conn, domain_controller_name, domain_access, 0, access_token);
        goto_if_ntstatus_not_success(status, error);

        status = DisableWksAccount(conn, machine_name, &account_h);
        goto_if_ntstatus_not_success(status, error);

        status = NetDisconnectSamr(conn);
        goto_if_ntstatus_not_success(status, error);
    }

cleanup:
    SAFE_FREE(machine_name);

    if (pi) {
        LwpsFreePasswordInfo(hStore, pi);
    }

    if (hStore != (HANDLE)NULL) {
       LwpsClosePasswordStore(hStore);
    }

    if (err == ERROR_SUCCESS &&
        status != STATUS_SUCCESS) {
        err = NtStatusToWin32Error(status);
    }

    return err;

error:
    if (access_token)
    {
        LwIoDeleteAccessToken(access_token);
    }

    SAFE_FREE(domain_controller_name);
    SAFE_FREE(machine_name);

    goto cleanup;
}


NET_API_STATUS NetUnjoinDomain(const wchar16_t *hostname,
                               const wchar16_t *account,
                               const wchar16_t *password,
                               uint32 options)
{
    NET_API_STATUS err = ERROR_SUCCESS;
    NTSTATUS status = STATUS_SUCCESS;
    wchar16_t *domain = NULL;
    HANDLE hStore = (HANDLE)NULL;
    PLWPS_PASSWORD_INFO pi = NULL;

    /* at the moment we support only locally triggered unjoin */
    if (hostname) {
        status = STATUS_NOT_IMPLEMENTED;

    } else {
        char hostname[MAXHOSTNAMELEN];
        wchar16_t host[MAXHOSTNAMELEN];

        if (gethostname((char*)hostname, sizeof(hostname)) < 0) {
            err = ERROR_INTERNAL_ERROR;
            goto error;
        }

        mbstowc16s(host, hostname, sizeof(wchar16_t)*MAXHOSTNAMELEN);

        status = LwpsOpenPasswordStore(LWPS_PASSWORD_STORE_DEFAULT, &hStore);
        goto_if_ntstatus_not_success(status, error);

        status = LwpsGetPasswordByHostName(hStore, hostname, &pi);
        goto_if_ntstatus_not_success(status, error);

        domain = pi->pwszDnsDomainName;
        err = NetUnjoinDomainLocal(host, domain, account, password, options);
        goto_if_winerr_not_success(err, error);
    }

    if (hStore != (HANDLE)NULL) {
       status = LwpsClosePasswordStore(hStore);
       goto_if_ntstatus_not_success(status, error);
    }

cleanup:
    if (pi) {
       LwpsFreePasswordInfo(hStore, pi);
    }

    if (err == ERROR_SUCCESS &&
        status != STATUS_SUCCESS) {
        err = NtStatusToWin32Error(status);
    }

    return err;

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
