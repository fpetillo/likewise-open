/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 */

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

//#include <lwrpc/unicodestring.h>
#include "includes.h"

extern int verbose_mode;


#define DISPLAY_DOMAINS(names, num)                             \
    {                                                           \
        uint32 i;                                               \
        for (i = 0; i < num; i++) {                             \
            wchar16_t *name = names[i];                         \
            w16printfw(L"Domain name: [%ws]\n", name);          \
        }                                                       \
    }


static
BOOL
CallSamrConnect(
    handle_t hBinding,
    const wchar16_t *system_name,
    CONNECT_HANDLE *phConn
    );


static
BOOL
CallSamrClose(
    handle_t hBinding,
    CONNECT_HANDLE *phConn
    );


static
BOOL
CallSamrEnumDomains(
    handle_t        hBinding,
    CONNECT_HANDLE  hConn,
    PSID           *ppDomainSid,
    PSID           *ppBuiltinSid
    );


static
BOOL
CallSamrOpenDomains(
    handle_t        hBinding,
    CONNECT_HANDLE  hConn,
    PSID            pDomainSid,
    PSID            pBuiltinSid,
    DOMAIN_HANDLE  *phDomain,
    DOMAIN_HANDLE  *phBuiltin
    );


static
BOOL
CallSamrEnumDomainUsers(
    handle_t       hBinding,
    DOMAIN_HANDLE  hDomain
    );


static
BOOL
CallSamrEnumDomainAliases(
    handle_t       hBinding,
    DOMAIN_HANDLE  hDomain
    );



handle_t CreateSamrBinding(handle_t *binding, const wchar16_t *host)
{
    RPCSTATUS status = RPC_S_OK;
    size_t hostname_size = 0;
    char *hostname = NULL;
    PIO_CREDS creds = NULL;

    if (binding == NULL) return NULL;

    if (host)
    {
        hostname_size = wc16slen(host) + 1;
        hostname = (char*) malloc(hostname_size * sizeof(char));
        if (hostname == NULL) return NULL;

        wc16stombs(hostname, host, hostname_size);
    }

    if (LwIoGetActiveCreds(NULL, &creds) != STATUS_SUCCESS)
    {
        return NULL;
    }

    status = InitSamrBindingDefault(binding, hostname, creds);
    if (status != RPC_S_OK) {
        int result;
        unsigned char errmsg[dce_c_error_string_len];

        dce_error_inq_text(status, errmsg, &result);
        if (result == 0) {
            printf("Error: %s\n", errmsg);
        } else {
            printf("Unknown error: %08lx\n", (unsigned long int)status);
        }

        SAFE_FREE(hostname);
        return NULL;
    }

    if (creds) {
        LwIoDeleteCreds(creds);
    }

    SAFE_FREE(hostname);
    return *binding;
}

static
void
GetSessionKey(handle_t binding, unsigned char** sess_key,
              unsigned16* sess_key_len, unsigned32* st)
{
    rpc_transport_info_handle_t info = NULL;

    rpc_binding_inq_transport_info(binding, &info, st);
    if (*st)
    {
        goto error;
    }

    rpc_smb_transport_info_inq_session_key(info, sess_key,
                                           sess_key_len);

cleanup:
    return;

error:
    *sess_key     = NULL;
    *sess_key_len = 0;
    goto cleanup;
}


/*
  Utility function for getting SAM domain name given a hostname
*/
NTSTATUS GetSamDomainName(wchar16_t **domname, const wchar16_t *hostname)
{
    const uint32 conn_access = SAMR_ACCESS_OPEN_DOMAIN |
                               SAMR_ACCESS_ENUM_DOMAINS;
    const uint32 enum_size = 32;
    const char *builtin = "Builtin";

    NTSTATUS status = STATUS_SUCCESS;
    NTSTATUS ret = STATUS_SUCCESS;
    handle_t samr_binding = NULL;
    CONNECT_HANDLE hConn = NULL;
    uint32 resume = 0;
    uint32 count = 0;
    uint32 i = 0;
    wchar16_t **dom_names = NULL;

    if (domname == NULL)
    {
        status = STATUS_INVALID_PARAMETER;
        goto done;
    }

    samr_binding = CreateSamrBinding(&samr_binding, hostname);
    if (samr_binding == NULL) return STATUS_UNSUCCESSFUL;

    status = SamrConnect2(samr_binding, hostname, conn_access, &hConn);
    if (status != 0) rpc_fail(status);

    dom_names = NULL;

    do {
        status = SamrEnumDomains(samr_binding, hConn, &resume,
                                 enum_size, &dom_names, &count);
        if (status != STATUS_SUCCESS &&
            status != STATUS_MORE_ENTRIES) rpc_fail(status);

        for (i = 0; i < count; i++) {
            char n[32] = {0};

            wc16stombs(n, dom_names[i], sizeof(n));
            if (strcasecmp(n, builtin)) {
                *domname = (wchar16_t*) wc16sdup(dom_names[i]);
                ret = STATUS_SUCCESS;

                SamrFreeMemory((void*)dom_names);
                goto found;
            }
        }

        SamrFreeMemory((void*)dom_names);

    } while (status == STATUS_MORE_ENTRIES);

    *domname = NULL;
    ret = STATUS_NOT_FOUND;

found:
    status = SamrClose(samr_binding, hConn);
    if (status != 0) return status;

done:
    FreeSamrBinding(&samr_binding);

    return status;
}


/*
  Utility function for getting SAM domain SID given a hostname
*/
NTSTATUS GetSamDomainSid(PSID* sid, const wchar16_t *hostname)
{
    const uint32 conn_access = SAMR_ACCESS_OPEN_DOMAIN |
                               SAMR_ACCESS_ENUM_DOMAINS;

    NTSTATUS status = STATUS_SUCCESS;
    handle_t samr_b = NULL;
    CONNECT_HANDLE hConn = NULL;
    wchar16_t *domname = NULL;
    PSID out_sid = NULL;

    if (sid == NULL)
    {
        status = STATUS_INVALID_PARAMETER;
        goto done;
    }

    status = GetSamDomainName(&domname, hostname);
    if (status != 0) goto done;

    samr_b = CreateSamrBinding(&samr_b, hostname);
    if (samr_b == NULL) rpc_fail(STATUS_UNSUCCESSFUL);

    status = SamrConnect2(samr_b, hostname, conn_access, &hConn);
    if (status != 0) rpc_fail(status);

    status = SamrLookupDomain(samr_b, hConn, domname, &out_sid);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_b, hConn);
    if (status != 0) rpc_fail(status);

    /* Allocate a copy of sid so it can be freed clean by the caller */
    MsRpcDuplicateSid(sid, out_sid);

done:
    FreeSamrBinding(&samr_b);

    if (out_sid) {
        SamrFreeMemory((void*)out_sid);
    }

    SAFE_FREE(domname);

    return status;
}


NTSTATUS EnsureUserAccount(const wchar16_t *hostname, wchar16_t *username,
                           int *created)
{

    const uint32 account_flags = ACB_NORMAL;
    const uint32 conn_access = SAMR_ACCESS_OPEN_DOMAIN |
                               SAMR_ACCESS_ENUM_DOMAINS;
    const uint32 domain_access = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                 DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                 DOMAIN_ACCESS_CREATE_USER |
                                 DOMAIN_ACCESS_LOOKUP_INFO_2;

    NTSTATUS status = STATUS_SUCCESS;
    PSID sid = NULL;
    handle_t samr_b = NULL;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    ACCOUNT_HANDLE hUser = NULL;
    uint32 user_rid = 0;

    if (created) *created = false;

    status = GetSamDomainSid(&sid, hostname);
    if (status != 0) rpc_fail(status);

    CreateSamrBinding(&samr_b, hostname);
    if (samr_b == NULL) goto done;

    status = SamrConnect2(samr_b, hostname, conn_access, &hConn);
    if (status != 0) rpc_fail(status)

    status = SamrOpenDomain(samr_b, hConn, domain_access, sid, &hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrCreateUser(samr_b, hDomain, username, account_flags,
                            &hUser, &user_rid);
    if (status == STATUS_SUCCESS) {
        if (created) *created = true;

        status = SamrClose(samr_b, hUser);
        if (status != 0) rpc_fail(status);

    } else if (status != 0 &&
               status != STATUS_USER_EXISTS) rpc_fail(status);

    status = SamrClose(samr_b, hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_b, hConn);
    if (status != 0) rpc_fail(status);

done:
    FreeSamrBinding(&samr_b);
    if (sid) MsRpcFreeSid(sid);

    return status;
}


NTSTATUS CleanupAccount(const wchar16_t *hostname, wchar16_t *username)
{
    const uint32 conn_access = SAMR_ACCESS_OPEN_DOMAIN |
                               SAMR_ACCESS_ENUM_DOMAINS;
    const uint32 domain_access = DOMAIN_ACCESS_OPEN_ACCOUNT;
    const uint32 user_access = DELETE;

    handle_t samr_b = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    ACCOUNT_HANDLE hAccount = NULL;
    PSID sid = NULL;
    wchar16_t *names[1] = {0};
    uint32 *rids = NULL;
    uint32 *types = NULL;
    uint32 rids_count = 0;

    status = GetSamDomainSid(&sid, hostname);
    if (status != 0) rpc_fail(status);

    CreateSamrBinding(&samr_b, hostname);
    if (samr_b == NULL) rpc_fail(status);

    status = SamrConnect2(samr_b, hostname, conn_access, &hConn);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_b, hConn, domain_access, sid, &hDomain);
    if (status != 0) rpc_fail(status);

    names[0] = username;
    status = SamrLookupNames(samr_b, hDomain, 1, names, &rids, &types,
                             &rids_count);

    /* if no account has been found return success */
    if (status == STATUS_NONE_MAPPED) {
        status = STATUS_SUCCESS;
        goto done;

    } else if (status != 0) {
        rpc_fail(status);
    }

    status = SamrOpenUser(samr_b, hDomain, user_access, rids[0], &hAccount);
    if (status != 0) rpc_fail(status);

    status = SamrDeleteUser(samr_b, hAccount);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_b, hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_b, hConn);
    if (status != 0) rpc_fail(status);

    FreeSamrBinding(&samr_b);

done:
    if (rids) SamrFreeMemory((void*)rids);
    if (types) SamrFreeMemory((void*)types);

    SAFE_FREE(sid);

    return status;
}


NTSTATUS EnsureAlias(const wchar16_t *hostname, wchar16_t *aliasname,
                     int *created)
{

    const uint32 conn_access = SAMR_ACCESS_OPEN_DOMAIN |
                               SAMR_ACCESS_ENUM_DOMAINS;
    const uint32 domain_access = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                 DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                 DOMAIN_ACCESS_CREATE_ALIAS |
                                 DOMAIN_ACCESS_LOOKUP_INFO_2;
    const uint32 alias_access = ALIAS_ACCESS_LOOKUP_INFO |
                                ALIAS_ACCESS_SET_INFO;

    NTSTATUS status = STATUS_SUCCESS;
    PSID sid = NULL;
    handle_t samr_b = NULL;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    ACCOUNT_HANDLE hAlias = NULL;
    uint32 alias_rid = 0;

    if (created) *created = false;

    status = GetSamDomainSid(&sid, hostname);
    if (status != 0) rpc_fail(status);

    CreateSamrBinding(&samr_b, hostname);
    if (samr_b == NULL) goto done;

    status = SamrConnect2(samr_b, hostname, conn_access, &hConn);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_b, hConn, domain_access, sid, &hDomain);
    if (status != 0) return status;

    status = SamrCreateDomAlias(samr_b, hDomain, aliasname, alias_access,
                                &hAlias, &alias_rid);
    if (status == STATUS_SUCCESS) {
        /* Let caller know that new alias have been created */
        if (created) *created = true;

        status = SamrClose(samr_b, hAlias);
        if (status != 0) rpc_fail(status);

    } else if (status != 0 &&
               status != STATUS_ALIAS_EXISTS) rpc_fail(status);

    status = SamrClose(samr_b, hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_b, hConn);
    if (status != 0) rpc_fail(status);

done:
    FreeSamrBinding(&samr_b);
    if (sid) MsRpcFreeSid(sid);

    return status;
}


NTSTATUS CleanupAlias(const wchar16_t *hostname, wchar16_t *username)
{
    const uint32 conn_access = SAMR_ACCESS_OPEN_DOMAIN |
                               SAMR_ACCESS_ENUM_DOMAINS;
    const uint32 domain_access = DOMAIN_ACCESS_OPEN_ACCOUNT;
    const uint32 alias_access = DELETE;

    handle_t samr_b = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    ACCOUNT_HANDLE hAlias = NULL;
    PSID sid = NULL;
    wchar16_t *names[1] = {0};
    uint32 *rids = NULL;
    uint32 *types = NULL;
    uint32 rids_count = 0;

    status = GetSamDomainSid(&sid, hostname);
    if (status != 0) rpc_fail(status);

    CreateSamrBinding(&samr_b, hostname);
    if (samr_b == NULL) goto done;

    status = SamrConnect2(samr_b, hostname, conn_access, &hConn);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_b, hConn, domain_access, sid, &hDomain);
    if (status != 0) rpc_fail(status);

    names[0] = username;
    status = SamrLookupNames(samr_b, hDomain, 1, names, &rids, &types,
                             &rids_count);

    /* if no account has been found return success */
    if (status == STATUS_NONE_MAPPED) {
        status = STATUS_SUCCESS;
        goto done;

    } else if (status != 0) {
        rpc_fail(status)
    }

    status = SamrOpenAlias(samr_b, hDomain, alias_access, rids[0], &hAlias);
    if (status != 0) rpc_fail(status);

    status = SamrDeleteDomAlias(samr_b, hAlias);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_b, hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_b, hConn);
    if (status != 0) rpc_fail(status);

    FreeSamrBinding(&samr_b);

done:
    if (rids) SamrFreeMemory((void*)rids);
    if (types) SamrFreeMemory((void*)types);

    SAFE_FREE(sid);

    return status;
}


static
BOOL
CallSamrConnect(
    handle_t hBinding,
    const wchar16_t *system_name,
    CONNECT_HANDLE *phConn
    )
{
    BOOL ret = TRUE;
    BOOL connected = FALSE;
    NTSTATUS status = STATUS_SUCCESS;
    uint32 access_mask = SAMR_ACCESS_OPEN_DOMAIN |
                         SAMR_ACCESS_ENUM_DOMAINS |
                         SAMR_ACCESS_CONNECT_TO_SERVER;
    uint32 level_in = 0;
    uint32 level_out = 0;
    SamrConnectInfo info_in;
    SamrConnectInfo info_out;
    CONNECT_HANDLE hConn = NULL;
    CONNECT_HANDLE h = NULL;

    memset(&info_in, 0, sizeof(info_in));
    memset(&info_out, 0, sizeof(info_out));

    DISPLAY_COMMENT(("Testing SamrConnect2\n"));

    status = SamrConnect2(hBinding, system_name, access_mask, &h);
    if (status) {
        DISPLAY_ERROR(("SamrConnect2 error %s\n", NtStatusToName(status)));
        ret = FALSE;
    } else {
        connected = TRUE;
        hConn     = h;
    }

    DISPLAY_COMMENT(("Testing SamrConnect3\n"));

    status = SamrConnect3(hBinding, system_name, access_mask, &h);
    if (status) {
        DISPLAY_ERROR(("SamrConnect3 error %s\n", NtStatusToName(status)));
        ret = FALSE;
    } else {
        if (connected) {
            CallSamrClose(hBinding, &hConn);
        }

        connected = TRUE;
        hConn     = h;
    }

    DISPLAY_COMMENT(("Testing SamrConnect4\n"));

    status = SamrConnect4(hBinding, system_name, 0, access_mask, &h);
    if (status) {
        DISPLAY_ERROR(("SamrConnect4 error %s\n", NtStatusToName(status)));
        ret = FALSE;
    } else {
        if (connected) {
            CallSamrClose(hBinding, &hConn);
        }

        connected = TRUE;
        hConn     = h;
    }

    DISPLAY_COMMENT(("Testing SamrConnect5\n"));

    level_in = 1;
    info_in.info1.client_version = SAMR_CONNECT_POST_WIN2K;

    status = SamrConnect5(hBinding, system_name, access_mask, level_in, &info_in,
                          &level_out, &info_out, &h);
    if (status) {
        DISPLAY_ERROR(("SamrConnect5 error %s\n", NtStatusToName(status)));
        ret = FALSE;
    } else {
        if (connected) {
            CallSamrClose(hBinding, &hConn);
        }

        connected = TRUE;
        hConn     = h;
    }

    *phConn = hConn;

    return ret;
}


static
BOOL
CallSamrClose(
    handle_t        hBinding,
    CONNECT_HANDLE *phConn
    )
{
    BOOL ret = TRUE;
    NTSTATUS status = STATUS_SUCCESS;
    CONNECT_HANDLE hConn = NULL;

    DISPLAY_COMMENT(("Testing SamrClose\n"));

    hConn = *phConn;

    status = SamrClose(hBinding, hConn);
    if (status) {
        DISPLAY_ERROR(("SamrClose error %s\n", NtStatusToName(status)));
    }

    return ret;
}


static
BOOL
CallSamrEnumDomains(
    handle_t        hBinding,
    CONNECT_HANDLE  hConn,
    PSID           *ppDomainSid,
    PSID           *ppBuiltinSid
    )
{
    BOOL ret = TRUE;
    NTSTATUS status = STATUS_SUCCESS;
    uint32 resume = 0;
    uint32 max_size = 16;
    wchar16_t **domains = NULL;
    uint32 num_entries = 0;
    uint32 i = 0;
    PSID pSid = NULL;
    PSID pDomainSid = NULL;
    PSID pBuiltinSid = NULL;

    DISPLAY_COMMENT(("Testing SamrEnumDomains with max_size = %d\n", max_size));

    do {
        status = SamrEnumDomains(hBinding, hConn, &resume, max_size,
                                 &domains, &num_entries);

        if (status != STATUS_SUCCESS &&
            status != STATUS_MORE_ENTRIES) {
            DISPLAY_ERROR(("SamrEnumDomains error %s\n", NtStatusToName(status)));

        } else {
            DISPLAY_DOMAINS(domains, num_entries);

            if (domains) {
                SamrFreeMemory((void*)domains);
            }
        }
    } while (status == STATUS_MORE_ENTRIES);

    resume    = 0;
    max_size *= 2;

    DISPLAY_COMMENT(("Testing SamrEnumDomains with max_size = %d\n", max_size));

    do {
        status = SamrEnumDomains(hBinding, hConn, &resume, max_size,
                                 &domains, &num_entries);

        if (status != STATUS_SUCCESS &&
            status != STATUS_MORE_ENTRIES) {
            DISPLAY_ERROR(("SamrEnumDomains error %s\n", NtStatusToName(status)));

        } else {
            DISPLAY_DOMAINS(domains, num_entries);

            if (domains) {
                SamrFreeMemory((void*)domains);
            }
        }
    } while (status == STATUS_MORE_ENTRIES);

    resume    = 0;
    max_size *= 2;

    DISPLAY_COMMENT(("Testing SamrEnumDomains with max_size = %d\n", max_size));

    do {
        status = SamrEnumDomains(hBinding, hConn, &resume, max_size,
                                 &domains, &num_entries);

        if (status != STATUS_SUCCESS &&
            status != STATUS_MORE_ENTRIES) {
            DISPLAY_ERROR(("SamrEnumDomains error %s\n", NtStatusToName(status)));

        } else {
            DISPLAY_DOMAINS(domains, num_entries);

            if (domains) {
                SamrFreeMemory((void*)domains);
            }
        }
    } while (status == STATUS_MORE_ENTRIES);

    resume   = 0;
    max_size = (uint32)(-1);

    DISPLAY_COMMENT(("Testing SamrEnumDomains with max_size = %d\n", max_size));

    do {
        status = SamrEnumDomains(hBinding, hConn, &resume, max_size,
                                 &domains, &num_entries);

        if (status != STATUS_SUCCESS &&
            status != STATUS_MORE_ENTRIES) {
            DISPLAY_ERROR(("SamrEnumDomains error %s\n", NtStatusToName(status)));
            ret = FALSE;

        } else {
            DISPLAY_DOMAINS(domains, num_entries);

            for (i = 0; i < num_entries; i++) {
                wchar16_t *domain = domains[i];

                DISPLAY_COMMENT(("Testing SamrLookupDomain\n"));

                status = SamrLookupDomain(hBinding, hConn, domain, &pSid);
                if (status) {
                    DISPLAY_ERROR(("SamrLookupDomain error %s\n",
                                   NtStatusToName(status)));
                    ret = FALSE;

                } else {
                    if (pSid->SubAuthorityCount > 1) {
                        DISPLAY_COMMENT(("Found domain SID\n"));
                        pDomainSid = pSid;

                    } else {
                        DISPLAY_COMMENT(("Found builtin domain SID\n"));
                        pBuiltinSid = pSid;
                    }
                }
            }

            if (domains) {
                SamrFreeMemory((void*)domains);
            }
        }
    } while (status == STATUS_MORE_ENTRIES);

    *ppDomainSid  = pDomainSid;
    *ppBuiltinSid = pBuiltinSid;

    return ret;
}


static
BOOL
CallSamrOpenDomains(
    handle_t        hBinding,
    CONNECT_HANDLE  hConn,
    PSID            pDomainSid,
    PSID            pBuiltinSid,
    DOMAIN_HANDLE  *phDomain,
    DOMAIN_HANDLE  *phBuiltin
    )
{
    BOOL ret = TRUE;
    NTSTATUS status = STATUS_SUCCESS;
    uint32 access_mask = DOMAIN_ACCESS_OPEN_ACCOUNT |
                         DOMAIN_ACCESS_ENUM_ACCOUNTS |
                         DOMAIN_ACCESS_CREATE_USER |
                         DOMAIN_ACCESS_CREATE_ALIAS |
                         DOMAIN_ACCESS_LOOKUP_INFO_2 |
                         DOMAIN_ACCESS_LOOKUP_INFO_1;
    DOMAIN_HANDLE hDomain = NULL;
    DOMAIN_HANDLE hBuiltin = NULL;
    DomainInfo *pInfo = NULL;
    uint16 i = 0;

    if (pDomainSid) {
        DISPLAY_COMMENT(("Testing SamrOpenDomain\n"));

        status = SamrOpenDomain(hBinding, hConn, access_mask, pDomainSid,
                                &hDomain);
        if (status) {
            DISPLAY_ERROR(("SamrOpenDomain error %s\n", NtStatusToName(status)));
            ret = FALSE;

        } else {
            for (i = 1; i <= 13; i++) {
                if (i == 10)
                {
                    /* domain info level 10 doesn't exist */
                    continue;
                }

                DISPLAY_COMMENT(("Testing SamrQueryDomainInfo (level=%d)\n", i));

                status = SamrQueryDomainInfo(hBinding, hDomain, i, &pInfo);
                if (status) {
                    DISPLAY_ERROR(("SamrQueryDomainInfo error %s\n", NtStatusToName(status)));
                    ret = FALSE;
                }

                if (pInfo)
                {
                    SamrFreeMemory(pInfo);
                }
            }
        }
    } else {
        DISPLAY_COMMENT(("Domain skipped\n"));
    }

    if (pBuiltinSid) {
        DISPLAY_COMMENT(("Testing SamrOpenDomain\n"));

        status = SamrOpenDomain(hBinding, hConn, access_mask, pBuiltinSid,
                                &hBuiltin);
        if (status) {
            DISPLAY_ERROR(("SamrOpenDomain error %s\n", NtStatusToName(status)));
            ret = FALSE;

        } else {
            for (i = 1; i <= 13; i++) {
                if (i == 10)
                {
                    /* domain info level 10 doesn't exist */
                    continue;
                }

                DISPLAY_COMMENT(("Testing SamrQueryDomainInfo (level=%d)\n", i));

                status = SamrQueryDomainInfo(hBinding, hBuiltin, i, &pInfo);
                if (status) {
                    DISPLAY_ERROR(("SamrQueryDomainInfo error %s\n", NtStatusToName(status)));
                    ret = FALSE;
                }

                if (pInfo)
                {
                    SamrFreeMemory(pInfo);
                }
            }
        }
    } else {
        DISPLAY_COMMENT(("Builtin domain skipped\n"));
    }

    *phDomain  = hDomain;
    *phBuiltin = hBuiltin;

    return ret;
}


#define DISPLAY_USERS(names, num)                               \
    {                                                           \
        uint32 i;                                               \
        for (i = 0; i < num; i++) {                             \
            wchar16_t *name = names[i];                         \
            w16printfw(L"User name: [%ws]\n", name);            \
        }                                                       \
    }


static
BOOL
CallSamrEnumDomainUsers(
    handle_t       hBinding,
    DOMAIN_HANDLE  hDomain
    )
{
    BOOL ret = TRUE;
    NTSTATUS status = STATUS_SUCCESS;
    uint32 resume = 0;
    uint32 max_size = 16;
    uint32 account_flags = 0;
    wchar16_t **users = NULL;
    uint32 *rids = NULL;
    uint32 *types = NULL;
    uint32 num_entries = 0;
    uint32 rids_count = 0;
    uint32 i = 0;
    wchar16_t *names[2];

    DISPLAY_COMMENT(("Testing SamrEnumDomainUsers with max_size = %d\n", max_size));

    resume = 0;
    account_flags = ACB_NORMAL;

    do {
        status = SamrEnumDomainUsers(hBinding, hDomain, &resume, account_flags,
                                     max_size, &users, &rids, &num_entries);

        if (status != STATUS_SUCCESS &&
            status != STATUS_MORE_ENTRIES) {
            DISPLAY_ERROR(("SamrEnumDomainUsers error %s\n", NtStatusToName(status)));

        } else {
            DISPLAY_USERS(users, num_entries);

            if (users) {
                SamrFreeMemory((void*)users);
            }
        }
    } while (status == STATUS_MORE_ENTRIES);

    resume    = 0;
    max_size *= 2;

    DISPLAY_COMMENT(("Testing SamrEnumDomainUsers with max_size = %d\n", max_size));

    do {
        status = SamrEnumDomainUsers(hBinding, hDomain, &resume, account_flags,
                                     max_size, &users, &rids, &num_entries);

        if (status != STATUS_SUCCESS &&
            status != STATUS_MORE_ENTRIES) {
            DISPLAY_ERROR(("SamrEnumDomainUsers error %s\n", NtStatusToName(status)));

        } else {
            DISPLAY_USERS(users, num_entries);

            if (users) {
                SamrFreeMemory((void*)users);
            }
        }
    } while (status == STATUS_MORE_ENTRIES);

    resume    = 0;
    max_size *= 2;

    DISPLAY_COMMENT(("Testing SamrEnumDomainUsers with max_size = %d\n", max_size));

    do {
        status = SamrEnumDomainUsers(hBinding, hDomain, &resume, account_flags,
                                     max_size, &users, &rids, &num_entries);

        if (status != STATUS_SUCCESS &&
            status != STATUS_MORE_ENTRIES) {
            DISPLAY_ERROR(("SamrEnumDomainUsers error %s\n", NtStatusToName(status)));

        } else {
            DISPLAY_USERS(users, num_entries);

            if (users) {
                SamrFreeMemory((void*)users);
            }
        }
    } while (status == STATUS_MORE_ENTRIES);

    resume   = 0;
    max_size = (uint32)(-1);

    DISPLAY_COMMENT(("Testing SamrEnumDomainUsers with max_size = %d\n", max_size));

    do {
        status = SamrEnumDomainUsers(hBinding, hDomain, &resume, account_flags,
                                     max_size, &users, &rids, &num_entries);

        if (status != STATUS_SUCCESS &&
            status != STATUS_MORE_ENTRIES) {
            DISPLAY_ERROR(("SamrEnumDomainUsers error %s\n", NtStatusToName(status)));
            ret = FALSE;

        } else {
            DISPLAY_USERS(users, num_entries);

            for (i = 0; i < num_entries; i++) {
                wchar16_t *user = users[i];

                names[0] = user;
                names[1] = NULL;

                DISPLAY_COMMENT(("Testing SamrLookupNames\n"));

                status = SamrLookupNames(hBinding, hDomain, 1, names,
                                         &rids, &types, &rids_count);
                if (status) {
                    DISPLAY_ERROR(("SamrLookupNames error %s\n",
                                   NtStatusToName(status)));
                }

                if (rids) {
                    SamrFreeMemory(rids);
                }

                if (types) {
                    SamrFreeMemory(types);
                }
            }

            if (users) {
                SamrFreeMemory((void*)users);
            }
        }
    } while (status == STATUS_MORE_ENTRIES);

    return ret;
}


static
BOOL
CallSamrEnumDomainAliases(
    handle_t       hBinding,
    DOMAIN_HANDLE  hDomain
    )
{
    BOOL ret = TRUE;
    NTSTATUS status = STATUS_SUCCESS;
    uint32 resume = 0;
    uint32 account_flags = 0;
    wchar16_t **aliases = NULL;
    uint32 *rids = NULL;
    uint32 *types = NULL;
    uint32 num_entries = 0;
    uint32 rids_count = 0;
    uint32 i = 0;
    wchar16_t *names[2];

    resume = 0;
    account_flags = ACB_NORMAL;

    DISPLAY_COMMENT(("Testing SamrEnumDomainAliases\n"));

    do {
        status = SamrEnumDomainAliases(hBinding, hDomain, &resume, account_flags,
                                       &aliases, &rids, &num_entries);

        if (status != STATUS_SUCCESS &&
            status != STATUS_MORE_ENTRIES) {
            DISPLAY_ERROR(("SamrEnumDomainAliases error %s\n", NtStatusToName(status)));
            ret = FALSE;

        } else {
            DISPLAY_USERS(aliases, num_entries);

            for (i = 0; i < num_entries; i++) {
                wchar16_t *alias = aliases[i];

                names[0] = alias;
                names[1] = NULL;

                DISPLAY_COMMENT(("Testing SamrLookupNames\n"));

                status = SamrLookupNames(hBinding, hDomain, 1, names,
                                         &rids, &types, &rids_count);
                if (status) {
                    DISPLAY_ERROR(("SamrLookupNames error %s\n",
                                   NtStatusToName(status)));
                }

                if (rids) {
                    SamrFreeMemory(rids);
                }

                if (types) {
                    SamrFreeMemory(types);
                }
            }

            if (aliases) {
                SamrFreeMemory((void*)aliases);
            }
        }
    } while (status == STATUS_MORE_ENTRIES);

    return ret;
}


int
TestSamrConnect(struct test *t, const wchar16_t *hostname,
                const wchar16_t *user, const wchar16_t *pass,
                struct parameter *options, int optcount)
{
    PCSTR pszDefSysName = "";

    BOOL ret = TRUE;
    enum param_err perr = perr_success;
    handle_t hBinding = NULL;
    CONNECT_HANDLE hConn = NULL;
    PWSTR pwszSysName = NULL;

    perr = fetch_value(options, optcount, "systemname", pt_w16string,
                       &pwszSysName, &pszDefSysName);
    if (!perr_is_ok(perr)) perr_fail(perr);

    TESTINFO(t, hostname, user, pass);

    SET_SESSION_CREDS(hCreds);

    CreateSamrBinding(&hBinding, hostname);

    ret &= CallSamrConnect(hBinding,
                           pwszSysName,
                           &hConn);

    ret &= CallSamrClose(hBinding,
                         &hConn);

done:
    FreeSamrBinding(&hBinding);
    RELEASE_SESSION_CREDS;

    SAFE_FREE(pwszSysName);

    return (int)ret;
}



int
TestSamrDomains(struct test *t, const wchar16_t *hostname,
                const wchar16_t *user, const wchar16_t *pass,
                struct parameter *options, int optcount)
{
    PCSTR pszDefSysName = "";

    BOOL ret = TRUE;
    enum param_err perr = perr_success;
    handle_t hBinding = NULL;
    CONNECT_HANDLE hConn = NULL;
    PSID pDomainSid = NULL;
    PSID pBuiltinSid = NULL;
    PWSTR pwszSysName = NULL;

    perr = fetch_value(options, optcount, "systemname", pt_w16string,
                       &pwszSysName, &pszDefSysName);
    if (!perr_is_ok(perr)) perr_fail(perr);

    TESTINFO(t, hostname, user, pass);

    SET_SESSION_CREDS(hCreds);

    CreateSamrBinding(&hBinding, hostname);

    ret &= CallSamrConnect(hBinding,
                           pwszSysName,
                           &hConn);

    ret &= CallSamrEnumDomains(hBinding,
                               hConn,
                               &pDomainSid,
                               &pBuiltinSid);

    ret &= CallSamrClose(hBinding,
                         &hConn);

done:
    FreeSamrBinding(&hBinding);

    SAFE_FREE(pwszSysName);

    return (int)ret;
}


int
TestSamrDomainsQuery(struct test *t, const wchar16_t *hostname,
                     const wchar16_t *user, const wchar16_t *pass,
                     struct parameter *options, int optcount)
{
    PCSTR pszDefSysName = "";

    BOOL ret = TRUE;
    enum param_err perr = perr_success;
    handle_t hBinding = NULL;
    CONNECT_HANDLE hConn = NULL;
    PSID pDomainSid = NULL;
    PSID pBuiltinSid = NULL;
    PWSTR pwszSysName = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    DOMAIN_HANDLE hBuiltin = NULL;

    perr = fetch_value(options, optcount, "systemname", pt_w16string,
                       &pwszSysName, &pszDefSysName);
    if (!perr_is_ok(perr)) perr_fail(perr);

    TESTINFO(t, hostname, user, pass);

    SET_SESSION_CREDS(hCreds);

    CreateSamrBinding(&hBinding, hostname);

    ret &= CallSamrConnect(hBinding,
                           pwszSysName,
                           &hConn);

    ret &= CallSamrEnumDomains(hBinding,
                               hConn,
                               &pDomainSid,
                               &pBuiltinSid);

    ret &= CallSamrOpenDomains(hBinding,
                               hConn,
                               pDomainSid,
                               pBuiltinSid,
                               &hDomain,
                               &hBuiltin);

    ret &= CallSamrClose(hBinding,
                         &hDomain);

    ret &= CallSamrClose(hBinding,
                         &hBuiltin);

    ret &= CallSamrClose(hBinding,
                         &hConn);

done:
    FreeSamrBinding(&hBinding);

    SAFE_FREE(pwszSysName);

    return (int)ret;
}


int
TestSamrUsers(struct test *t, const wchar16_t *hostname,
                const wchar16_t *user, const wchar16_t *pass,
                struct parameter *options, int optcount)
{
    PCSTR pszDefSysName = "";

    BOOL ret = TRUE;
    enum param_err perr = perr_success;
    handle_t hBinding = NULL;
    CONNECT_HANDLE hConn = NULL;
    PSID pDomainSid = NULL;
    PSID pBuiltinSid = NULL;
    PWSTR pwszSysName = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    DOMAIN_HANDLE hBuiltin = NULL;

    perr = fetch_value(options, optcount, "systemname", pt_w16string,
                       &pwszSysName, &pszDefSysName);
    if (!perr_is_ok(perr)) perr_fail(perr);

    TESTINFO(t, hostname, user, pass);

    SET_SESSION_CREDS(hCreds);

    CreateSamrBinding(&hBinding, hostname);

    ret &= CallSamrConnect(hBinding,
                           pwszSysName,
                           &hConn);

    ret &= CallSamrEnumDomains(hBinding,
                               hConn,
                               &pDomainSid,
                               &pBuiltinSid);

    ret &= CallSamrOpenDomains(hBinding,
                               hConn,
                               pDomainSid,
                               pBuiltinSid,
                               &hDomain,
                               &hBuiltin);

    ret &= CallSamrEnumDomainUsers(hBinding,
                                   hDomain);

    ret &= CallSamrEnumDomainUsers(hBinding,
                                   hBuiltin);

    ret &= CallSamrClose(hBinding,
                         &hConn);

done:
    FreeSamrBinding(&hBinding);

    SAFE_FREE(pwszSysName);

    return (int)ret;
}


int
TestSamrAliases(struct test *t, const wchar16_t *hostname,
                const wchar16_t *user, const wchar16_t *pass,
                struct parameter *options, int optcount)
{
    PCSTR pszDefSysName = "";

    BOOL ret = TRUE;
    enum param_err perr = perr_success;
    handle_t hBinding = NULL;
    CONNECT_HANDLE hConn = NULL;
    PSID pDomainSid = NULL;
    PSID pBuiltinSid = NULL;
    PWSTR pwszSysName = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    DOMAIN_HANDLE hBuiltin = NULL;

    perr = fetch_value(options, optcount, "systemname", pt_w16string,
                       &pwszSysName, &pszDefSysName);
    if (!perr_is_ok(perr)) perr_fail(perr);

    TESTINFO(t, hostname, user, pass);

    SET_SESSION_CREDS(hCreds);

    CreateSamrBinding(&hBinding, hostname);

    ret &= CallSamrConnect(hBinding,
                           pwszSysName,
                           &hConn);

    ret &= CallSamrEnumDomains(hBinding,
                               hConn,
                               &pDomainSid,
                               &pBuiltinSid);

    ret &= CallSamrOpenDomains(hBinding,
                               hConn,
                               pDomainSid,
                               pBuiltinSid,
                               &hDomain,
                               &hBuiltin);

    ret &= CallSamrEnumDomainAliases(hBinding,
                                     &hDomain);

    ret &= CallSamrEnumDomainAliases(hBinding,
                                     &hBuiltin);

    ret &= CallSamrClose(hBinding,
                         &hConn);

done:
    return (int)ret;
}


int TestSamrQueryUser(struct test *t, const wchar16_t *hostname,
                      const wchar16_t *user, const wchar16_t *pass,
                      struct parameter *options, int optcount)
{
    const uint32 conn_access_mask = SAMR_ACCESS_OPEN_DOMAIN |
                                    SAMR_ACCESS_ENUM_DOMAINS |
                                    SAMR_ACCESS_CONNECT_TO_SERVER;

    const uint32 dom_access_mask = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                   DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                   DOMAIN_ACCESS_CREATE_USER |
                                   DOMAIN_ACCESS_CREATE_ALIAS |
                                   DOMAIN_ACCESS_LOOKUP_INFO_2;

    const uint32 usr_access_mask = USER_ACCESS_GET_NAME_ETC |
                                   USER_ACCESS_GET_LOCALE |
                                   USER_ACCESS_GET_LOGONINFO |
                                   USER_ACCESS_GET_ATTRIBUTES |
                                   USER_ACCESS_CHANGE_PASSWORD |
                                   DELETE;
    const char *def_guestname = "Guest";
    const char *def_domainname = "Builtin";
    const int def_level = 0;

    NTSTATUS status = STATUS_SUCCESS;
    enum param_err perr = perr_success;
    handle_t samr_binding = NULL;
    int i = 0;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    ACCOUNT_HANDLE hUser = NULL;
    PSID sid = NULL;
    wchar16_t *names[1];
    wchar16_t *username = NULL;
    wchar16_t *domainname = NULL;
    uint32 *rids = NULL;
    uint32 *types = NULL;
    uint32 rids_count = 0;
    UserInfo *info = NULL;
    int32 level = 0;

    TESTINFO(t, hostname, user, pass);

    SET_SESSION_CREDS(hCreds);

    samr_binding = CreateSamrBinding(&samr_binding, hostname);
    if (samr_binding == NULL) return -1;

    perr = fetch_value(options, optcount, "username", pt_w16string, &username,
                       &def_guestname);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "domainname", pt_w16string, &domainname,
                       &def_domainname);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "level", pt_int32, &level,
                       &def_level);
    if (!perr_is_ok(perr)) perr_fail(perr);

    PARAM_INFO("domainname", pt_w16string, domainname);
    PARAM_INFO("username", pt_w16string, username);
    PARAM_INFO("level", pt_int32, &level);

    names[0] = username;

    /*
     * Simple user account querying
     */
    status = SamrConnect2(samr_binding, hostname, conn_access_mask, &hConn);
    if (status != 0) rpc_fail(status);

    status = SamrLookupDomain(samr_binding, hConn, domainname, &sid);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_binding, hConn, dom_access_mask, sid,
                            &hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrLookupNames(samr_binding, hDomain, 1, names, &rids, &types,
                             &rids_count);
    if (status != 0) rpc_fail(status);

    status = SamrOpenUser(samr_binding, hDomain, usr_access_mask, rids[0],
                          &hUser);
    if (status != 0) rpc_fail(status);

    SamrFreeMemory((void*)rids);
    SamrFreeMemory((void*)types);

    if (level == 0) {
        for (i = 1; i < 26; i++) {
            if (i == 5) continue;      /* infolevel 5 is still broken for some reason */

            INPUT_ARG_PTR(samr_binding);
            INPUT_ARG_PTR(hUser);
            INPUT_ARG_UINT(i);

            CALL_MSRPC(status = SamrQueryUserInfo(samr_binding, hUser,
                                                  (uint16)i, &info));
            if (status != STATUS_SUCCESS &&
                status != STATUS_INVALID_INFO_CLASS) rpc_fail(status);

            SamrFreeMemory((void*)info);
            info = NULL;
        }
    } else {
        INPUT_ARG_PTR(samr_binding);
        INPUT_ARG_PTR(hUser);
        INPUT_ARG_UINT(level);

        CALL_MSRPC(status = SamrQueryUserInfo(samr_binding, hUser,
                                              (uint16)level, &info));
        if (status != STATUS_SUCCESS &&
            status != STATUS_INVALID_INFO_CLASS) rpc_fail(status);

        SamrFreeMemory((void*)info);
        info = NULL;
    }

    status = SamrClose(samr_binding, hUser);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hConn);
    if (status != 0) rpc_fail(status);

    RELEASE_SESSION_CREDS;

done:
    SAFE_FREE(username);
    SAFE_FREE(domainname);

    SamrDestroyMemory();

    return (status == STATUS_SUCCESS);
}


#define DISPLAY_ALIAS_INFO(info, level)                           \
    if (verbose_mode ) {                                          \
        switch ((level)) {                                        \
        case 1:                                                   \
            OUTPUT_ARG_UNICODE_STRING(&(info)->all.name);         \
            OUTPUT_ARG_UNICODE_STRING(&(info)->all.description);  \
            OUTPUT_ARG_UINT((info)->all.num_members);             \
            break;                                                \
                                                                  \
        case 2:                                                   \
            OUTPUT_ARG_UNICODE_STRING(&(info)->name);             \
            break;                                                \
                                                                  \
        case 3:                                                   \
            OUTPUT_ARG_UNICODE_STRING(&(info)->description);      \
            break;                                                \
        }                                                         \
    }


int TestSamrQueryAlias(struct test *t, const wchar16_t *hostname,
                       const wchar16_t *user, const wchar16_t *pass,
                       struct parameter *options, int optcount)
{
    const uint32 conn_access_mask = SAMR_ACCESS_OPEN_DOMAIN |
                                    SAMR_ACCESS_ENUM_DOMAINS |
                                    SAMR_ACCESS_CONNECT_TO_SERVER;

    const uint32 dom_access_mask = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                   DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                   DOMAIN_ACCESS_CREATE_USER |
                                   DOMAIN_ACCESS_CREATE_ALIAS |
                                   DOMAIN_ACCESS_LOOKUP_INFO_2;

    const uint32 alias_access_mask = ALIAS_ACCESS_GET_MEMBERS |
                                     ALIAS_ACCESS_LOOKUP_INFO;

    const char *def_guestsname = "Guests";
    const char *def_domainname = "Builtin";
    const int def_level = 0;

    NTSTATUS status = STATUS_SUCCESS;
    enum param_err perr = perr_success;
    handle_t samr_binding = NULL;
    int i = 0;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    ACCOUNT_HANDLE hAlias = NULL;
    PSID sid = NULL;
    wchar16_t *names[1];
    wchar16_t *aliasname = NULL;
    wchar16_t *domainname = NULL;
    uint32 *rids = NULL;
    uint32 *types = NULL;
    uint32 rids_count = 0;
    AliasInfo *info = NULL;
    int32 level = 0;

    TESTINFO(t, hostname, user, pass);

    SET_SESSION_CREDS(hCreds);

    samr_binding = CreateSamrBinding(&samr_binding, hostname);
    if (samr_binding == NULL) return -1;

    perr = fetch_value(options, optcount, "aliasname", pt_w16string, &aliasname,
                       &def_guestsname);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "domainname", pt_w16string, &domainname,
                       &def_domainname);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "level", pt_int32, &level,
                       &def_level);
    if (!perr_is_ok(perr)) perr_fail(perr);

    PARAM_INFO("domainname", pt_w16string, domainname);
    PARAM_INFO("aliasname", pt_w16string, aliasname);
    PARAM_INFO("level", pt_int32, &level);

    names[0] = aliasname;

    /*
     * Simple alias account querying
     */
    status = SamrConnect2(samr_binding, hostname, conn_access_mask, &hConn);
    if (status != 0) rpc_fail(status);

    status = SamrLookupDomain(samr_binding, hConn, domainname, &sid);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_binding, hConn, dom_access_mask, sid,
                            &hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrLookupNames(samr_binding, hDomain, 1, names, &rids, &types,
                             &rids_count);
    if (status != 0) rpc_fail(status);

    status = SamrOpenAlias(samr_binding, hDomain, alias_access_mask, rids[0],
                           &hAlias);
    if (status != 0) rpc_fail(status);

    SamrFreeMemory((void*)rids);
    SamrFreeMemory((void*)types);

    if (level == 0) {
        for (i = 1; i <= 3; i++) {
            INPUT_ARG_PTR(samr_binding);
            INPUT_ARG_PTR(hAlias);
            INPUT_ARG_UINT(i);

            CALL_MSRPC(status = SamrQueryAliasInfo(samr_binding, hAlias,
                                                   (uint16)i, &info));
            if (status != STATUS_SUCCESS &&
                status != STATUS_INVALID_INFO_CLASS) rpc_fail(status);

            DISPLAY_ALIAS_INFO(info, i);

            SamrFreeMemory((void*)info);
            info = NULL;
        }
    } else {
        INPUT_ARG_PTR(samr_binding);
        INPUT_ARG_PTR(hAlias);
        INPUT_ARG_UINT(level);

        CALL_MSRPC(status = SamrQueryAliasInfo(samr_binding, hAlias,
                                              (uint16)level, &info));
        if (status != STATUS_SUCCESS &&
            status != STATUS_INVALID_INFO_CLASS) rpc_fail(status);

        DISPLAY_ALIAS_INFO(info, i);

        SamrFreeMemory((void*)info);
        info = NULL;
    }

    status = SamrClose(samr_binding, hAlias);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hConn);
    if (status != 0) rpc_fail(status);

    RELEASE_SESSION_CREDS;

done:
    SAFE_FREE(aliasname);
    SamrDestroyMemory();

    return (status == STATUS_SUCCESS);
}


int TestSamrAlias(struct test *t, const wchar16_t *hostname,
                  const wchar16_t *user, const wchar16_t *pass,
                  struct parameter *options, int optcount)
{
    const uint32 conn_access_mask = SAMR_ACCESS_OPEN_DOMAIN |
                                    SAMR_ACCESS_ENUM_DOMAINS |
                                    SAMR_ACCESS_CONNECT_TO_SERVER;

    const uint32 dom_access_mask = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                   DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                   DOMAIN_ACCESS_CREATE_USER |
                                   DOMAIN_ACCESS_CREATE_ALIAS |
                                   DOMAIN_ACCESS_LOOKUP_INFO_2;

    const uint32 alias_access_mask = ALIAS_ACCESS_LOOKUP_INFO |
                                     ALIAS_ACCESS_SET_INFO |
                                     ALIAS_ACCESS_ADD_MEMBER |
                                     ALIAS_ACCESS_REMOVE_MEMBER |
                                     ALIAS_ACCESS_GET_MEMBERS |
                                     DELETE;

    const char *testalias = "TestAlias";
    const char *testuser = "TestUser";
    const char *testalias_desc = "TestAlias Comment";

    NTSTATUS status = STATUS_SUCCESS;
    enum param_err perr = perr_success;
    handle_t samr_binding = NULL;
    uint32 user_rid = 0;
    uint32 *rids = NULL;
    uint32 *types = NULL;
    uint32 num_rids = 0;
    wchar16_t *aliasname = NULL;
    wchar16_t *aliasdesc = NULL;
    wchar16_t *username = NULL;
    wchar16_t *domname = NULL;
    int i = 0;
    uint32 rids_count = 0;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    ACCOUNT_HANDLE hAlias = NULL;
    ACCOUNT_HANDLE hUser = NULL;
    wchar16_t *names[1] = {0};
    PSID sid = NULL;
    PSID user_sid = NULL;
    AliasInfo *aliasinfo = NULL;
    AliasInfo setaliasinfo;
    PSID* member_sids = NULL;
    uint32 members_num = 0;
    int alias_created = 0;
    int user_created = 0;

    TESTINFO(t, hostname, user, pass);

    SET_SESSION_CREDS(hCreds);

    samr_binding = CreateSamrBinding(&samr_binding, hostname);
    if (samr_binding == NULL) return false;

    perr = fetch_value(options, optcount, "aliasname", pt_w16string, &aliasname, &testalias);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "aliasdesc", pt_w16string, &aliasdesc, &testalias_desc);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "username", pt_w16string, &username, &testuser);
    if (!perr_is_ok(perr)) perr_fail(perr);

    PARAM_INFO("aliasname", pt_w16string, aliasname);
    PARAM_INFO("aliasdesc", pt_w16string, aliasdesc);
    PARAM_INFO("username", pt_w16string, username);


    /*
     * Creating and querying/setting alias (in the host domain)
     */

    status = SamrConnect2(samr_binding, hostname, conn_access_mask, &hConn);
    if (status != 0) rpc_fail(status);

    status = GetSamDomainName(&domname, hostname);
    if (status != 0) rpc_fail(status);

    status = SamrLookupDomain(samr_binding, hConn, domname, &sid);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_binding, hConn, dom_access_mask, sid,
                            &hDomain);
    if (status != 0) rpc_fail(status);

    /*
     * Ensure alias to perform tests on
     */
    status = EnsureAlias(hostname, aliasname, &alias_created);
    if (status != 0) rpc_fail(status);

    names[0] = aliasname;

    status = SamrLookupNames(samr_binding, hDomain, 1, names,
                             &rids, &types, &rids_count);
    if (status != 0) rpc_fail(status);

    if (rids_count == 0 || rids_count != 1) {
        printf("Incosistency found when looking for alias name\n");
        rpc_fail(STATUS_UNSUCCESSFUL);
    }

    status = SamrOpenAlias(samr_binding, hDomain, alias_access_mask,
                           rids[0], &hAlias);
    if (status != 0) rpc_fail(status);

    if (rids) SamrFreeMemory((void*)rids);
    if (types) SamrFreeMemory((void*)types);
    rids  = NULL;
    types = NULL;

    /*
     * Ensure a user account which will soon become alias member
     */
    status = EnsureUserAccount(hostname, username, &user_created);
    if (status != 0) rpc_fail(status);

    names[0] = username;

    status = SamrLookupNames(samr_binding, hDomain, 1, names, &rids, &types,
                             &rids_count);
    if (status != 0) rpc_fail(status);

    if (rids_count == 0 || rids_count != 1) {
        printf("Incosistency found when looking for alias name\n");
        rpc_fail(STATUS_UNSUCCESSFUL);
    }

    user_rid = rids[0];

    if (rids) SamrFreeMemory((void*)rids);
    if (types) SamrFreeMemory((void*)types);
    rids  = NULL;
    types = NULL;

    for (i = 3; i > 1; i--) {
        INPUT_ARG_PTR(samr_binding);
        INPUT_ARG_PTR(hAlias);
        INPUT_ARG_UINT(i);

        CALL_MSRPC(status = SamrQueryAliasInfo(samr_binding, hAlias,
                                               (uint16)i, &aliasinfo));
        if (status != 0) rpc_fail(status);

        if (aliasinfo) SamrFreeMemory((void*)aliasinfo);
    }

    status = SamrQueryAliasInfo(samr_binding, hAlias, 1, &aliasinfo);
    if (status != 0) rpc_fail(status);

    status = InitUnicodeString(&setaliasinfo.description, aliasdesc);
    if (status != 0) rpc_fail(status);

    status = SamrSetAliasInfo(samr_binding, hAlias, 3, &setaliasinfo);
    if (status != 0) rpc_fail(status);

    status = MsRpcAllocateSidAppendRid(&user_sid, sid, user_rid);
    if (status != 0) rpc_fail(status);

    status = SamrGetAliasMembership(samr_binding, hDomain, &user_sid, 1,
                                    &rids, &num_rids);
    if (status != 0) rpc_fail(status);


    /*
     * Adding, deleting and querying members in alias
     */

    status = SamrAddAliasMember(samr_binding, hAlias, user_sid);
    if (status != 0) rpc_fail(status);

    status = SamrGetMembersInAlias(samr_binding, hAlias, &member_sids,
                                   &members_num);
    if (status != 0) rpc_fail(status);

    status = SamrDeleteAliasMember(samr_binding, hAlias, user_sid);
    if (status != 0) rpc_fail(status);


    /*
     * Cleanup
     */

    if (alias_created) {
        status = SamrDeleteDomAlias(samr_binding, hAlias);
        if (status != 0) rpc_fail(status);

    } else {
        status = SamrClose(samr_binding, hAlias);
        if (status != 0) rpc_fail(status);
    }

    if (user_created) {
        status = SamrDeleteUser(samr_binding, hUser);
        if (status != 0) rpc_fail(status);
    }

    status = SamrClose(samr_binding, hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hConn);
    if (status != 0) rpc_fail(status);

    FreeSamrBinding(&samr_binding);
    RELEASE_SESSION_CREDS;

done:
    if (aliasinfo) SamrFreeMemory((void*)aliasinfo);
    if (rids) SamrFreeMemory((void*)rids);
    if (member_sids) SamrFreeMemory((void*)member_sids);
    if (domname) SamrFreeMemory((void*)domname);
    if (rids) SamrFreeMemory((void*)rids);
    if (types) SamrFreeMemory((void*)types);

    FreeUnicodeString(&setaliasinfo.description);
    if (user_sid) MsRpcFreeSid(user_sid);

    SAFE_FREE(aliasname);
    SAFE_FREE(aliasdesc);
    SAFE_FREE(username);

    return (status == STATUS_SUCCESS);
}


int TestSamrUsersInAliases(struct test *t, const wchar16_t *hostname,
                           const wchar16_t *user, const wchar16_t *pass,
                           struct parameter *options, int optcount)
{
    const uint32 conn_access_mask = SAMR_ACCESS_OPEN_DOMAIN |
                                    SAMR_ACCESS_ENUM_DOMAINS |
                                    SAMR_ACCESS_CONNECT_TO_SERVER;

    const uint32 builtin_dom_access_mask = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                           DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                           DOMAIN_ACCESS_LOOKUP_INFO_2;
    const char *btin_sidstr = "S-1-5-32";

    NTSTATUS status = STATUS_SUCCESS;
    enum param_err perr = perr_success;
    handle_t samr_binding = NULL;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hBtinDomain = NULL;
    char *sidstr = NULL;
    PSID sid = NULL;
    DomainInfo *dominfo = NULL;

    TESTINFO(t, hostname, user, pass);

    SET_SESSION_CREDS(hCreds);

    samr_binding = CreateSamrBinding(&samr_binding, hostname);
    if (samr_binding == NULL) return false;

    /*
     * Querying user membership in aliases
     */

    perr = fetch_value(options, optcount, "sid", pt_string, &sidstr,
                       &btin_sidstr);
    if (!perr_is_ok(perr)) perr_fail(perr);

    RtlAllocateSidFromCString(&sid, sidstr);

    status = SamrConnect2(samr_binding, hostname, conn_access_mask,
                          &hConn);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_binding, hConn, builtin_dom_access_mask,
                            sid, &hBtinDomain);
    if (status == 0) {
        uint32 acct_flags = ACB_NORMAL;
        uint32 resume = 0;
        wchar16_t **names = NULL;
        uint32 *rids = NULL;
        uint32 num_entries = 0;
        uint32 i = 0;
        PSID alias_sid = NULL;

        do {
            status = SamrEnumDomainAliases(samr_binding, hBtinDomain,
                                           &resume, acct_flags, &names, &rids,
                                           &num_entries);

            for (i = 0; i < num_entries; i++) {
                uint32 *member_rids = NULL;
                uint32 count = 0;

                status = MsRpcAllocateSidAppendRid(&alias_sid,
                                                   sid,
                                                   rids[i]);

                /* there's actually no need to check status code here */
                status = SamrGetAliasMembership(samr_binding,
                                                hBtinDomain,
                                                &alias_sid, 1, &member_rids,
                                                &count);
                SAFE_FREE(alias_sid);

                if (member_rids) SamrFreeMemory((void*)member_rids);
            }

            if (names) SamrFreeMemory((void*)names);
            if (rids) SamrFreeMemory((void*)rids);
            names = NULL;
            rids  = NULL;

        } while (status == STATUS_MORE_ENTRIES);

        status = SamrQueryDomainInfo(samr_binding, hBtinDomain,
                                     (uint16)2, &dominfo);

        status = SamrClose(samr_binding, hBtinDomain);
    }

    status = SamrClose(samr_binding, hConn);
    if (status != 0) rpc_fail(status);

    FreeSamrBinding(&samr_binding);
    RELEASE_SESSION_CREDS;

done:
    RTL_FREE(&sid);
    SAFE_FREE(sidstr);

    return (status == STATUS_SUCCESS);
}


int TestSamrOpenDomain(struct test *t, const wchar16_t *hostname,
                       const wchar16_t *user, const wchar16_t *pass,
                       struct parameter *options, int optcount)
{
    const uint32 conn_access_mask = SAMR_ACCESS_OPEN_DOMAIN |
                                    SAMR_ACCESS_ENUM_DOMAINS |
                                    SAMR_ACCESS_CONNECT_TO_SERVER;

    const uint32 dom_access_mask = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                   DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                   DOMAIN_ACCESS_CREATE_USER |
                                   DOMAIN_ACCESS_CREATE_ALIAS |
                                   DOMAIN_ACCESS_LOOKUP_INFO_2 |
                                   DOMAIN_ACCESS_LOOKUP_INFO_1;

    const char *def_domainname = "BUILTIN";

    NTSTATUS status = STATUS_SUCCESS;
    enum param_err perr = perr_success;
    handle_t samr_binding = NULL;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    PSID sid = NULL;
    wchar16_t *domainname = NULL;

    SET_SESSION_CREDS(hCreds);

    perr = fetch_value(options, optcount, "domainname", pt_w16string,
                       &domainname, &def_domainname);
    if (!perr_is_ok(perr)) perr_fail(perr);

    samr_binding = CreateSamrBinding(&samr_binding, hostname);
    if (samr_binding == NULL) return false;

    status = SamrConnect2(samr_binding, hostname, conn_access_mask,
                          &hConn);
    if (status != 0) rpc_fail(status);

    status = SamrLookupDomain(samr_binding, hConn, domainname, &sid);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_binding, hConn, dom_access_mask,
                            sid, &hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hConn);
    if (status != 0) rpc_fail(status);

    FreeSamrBinding(&samr_binding);
    RELEASE_SESSION_CREDS;

done:
    if (sid) SamrFreeMemory((void*)sid);
    SAFE_FREE(domainname);

    return (status == STATUS_SUCCESS);
}


int TestSamrQueryDomain(struct test *t, const wchar16_t *hostname,
                        const wchar16_t *user, const wchar16_t *pass,
                        struct parameter *options, int optcount)
{
    const uint32 conn_access_mask = SAMR_ACCESS_OPEN_DOMAIN |
                                    SAMR_ACCESS_ENUM_DOMAINS |
                                    SAMR_ACCESS_CONNECT_TO_SERVER;

    const uint32 dom_access_mask = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                   DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                   DOMAIN_ACCESS_CREATE_USER |
                                   DOMAIN_ACCESS_CREATE_ALIAS |
                                   DOMAIN_ACCESS_LOOKUP_INFO_2 |
                                   DOMAIN_ACCESS_LOOKUP_INFO_1;

    NTSTATUS status = STATUS_SUCCESS;
    handle_t samr_binding = NULL;
    int i = 0;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    PSID sid = NULL;
    DomainInfo *dominfo = NULL;
    wchar16_t *domname = NULL;

    SET_SESSION_CREDS(hCreds);

    samr_binding = CreateSamrBinding(&samr_binding, hostname);
    if (samr_binding == NULL) return false;

    status = SamrConnect2(samr_binding, hostname, conn_access_mask,
                          &hConn);
    if (status != 0) rpc_fail(status);

    status = GetSamDomainName(&domname, hostname);
    if (status != 0) rpc_fail(status);

    status = SamrLookupDomain(samr_binding, hConn, domname, &sid);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_binding, hConn, dom_access_mask,
                            sid, &hDomain);
    if (status != 0) rpc_fail(status);

    for (i = 1; i < 13; i++) {
        if (i == 10) continue;

        INPUT_ARG_PTR(samr_binding);
        INPUT_ARG_PTR(hDomain);
        INPUT_ARG_INT(i);
        INPUT_ARG_PTR(dominfo);

        CALL_MSRPC(status = SamrQueryDomainInfo(samr_binding, hDomain,
                                                (uint16)i, &dominfo));

        OUTPUT_ARG_PTR(dominfo);

        if (dominfo) {
            SamrFreeMemory((void*)dominfo);
        }
    }

    status = SamrClose(samr_binding, hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hConn);
    if (status != 0) rpc_fail(status);

    FreeSamrBinding(&samr_binding);
    RELEASE_SESSION_CREDS;

done:
    if (sid) SamrFreeMemory((void*)sid);
    SAFE_FREE(domname);

    return (status == STATUS_SUCCESS);
}


#define DISPLAY_ACCOUNTS(type, flags, names, rids, num)         \
    {                                                           \
        uint32 i;                                               \
        for (i = 0; i < num; i++) {                             \
            wchar16_t *name = enum_names[i];                    \
            uint32 rid = enum_rids[i];                          \
                                                                \
            w16printfw(L"%hhs (flags=0x%08x): %ws (rid=0x%x)\n",\
                       (type), (flags), (name), (rid));         \
        }                                                       \
    }

int TestSamrEnumUsers(struct test *t, const wchar16_t *hostname,
                      const wchar16_t *user, const wchar16_t *pass,
                      struct parameter *options, int optcount)
{
    const uint32 conn_access_mask = SAMR_ACCESS_OPEN_DOMAIN |
                                    SAMR_ACCESS_ENUM_DOMAINS |
                                    SAMR_ACCESS_CONNECT_TO_SERVER;

    const uint32 dom_access_mask = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                   DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                   DOMAIN_ACCESS_CREATE_USER |
                                   DOMAIN_ACCESS_CREATE_ALIAS |
                                   DOMAIN_ACCESS_LOOKUP_INFO_2;

    const int def_specifydomain = 0;
    const char *def_domainname = "BUILTIN";
    const int def_acb_flags = 0;

    NTSTATUS status = STATUS_SUCCESS;
    handle_t samr_binding = NULL;
    enum param_err perr = perr_success;
    uint32 resume = 0;
    uint32 num_entries = 0;
    uint32 max_size = 0;
    uint32 acb_flags = 0;
    uint32 account_flags = 0;
    wchar16_t **enum_names = NULL;
    wchar16_t *domname = NULL;
    wchar16_t *domainname = NULL;
    uint32 *enum_rids = NULL;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    PSID sid = NULL;
    int specifydomain = 0;

    perr = fetch_value(options, optcount, "specifydomain", pt_int32,
                       &specifydomain, &def_specifydomain);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "domainname", pt_w16string,
                       &domainname, &def_domainname);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "acbflags", pt_uint32,
                       &acb_flags, &def_acb_flags);
    if (!perr_is_ok(perr)) perr_fail(perr);

    SET_SESSION_CREDS(hCreds);

    samr_binding = CreateSamrBinding(&samr_binding, hostname);
    if (samr_binding == NULL) return false;

    status = SamrConnect2(samr_binding, hostname, conn_access_mask,
                          &hConn);
    if (status != 0) rpc_fail(status);

    if (specifydomain) {
        domname = wc16sdup(domainname);

    } else {
        status = GetSamDomainName(&domname, hostname);
        if (status != 0) rpc_fail(status);
    }

    status = SamrLookupDomain(samr_binding, hConn, domname, &sid);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_binding, hConn, dom_access_mask,
                            sid, &hDomain);
    if (status != 0) rpc_fail(status);

    /*
     * Enumerating domain users
     */

    max_size = 128;
    resume = 0;
    account_flags = ACB_NORMAL;
    do {
        status = SamrEnumDomainUsers(samr_binding, hDomain, &resume,
                                     account_flags, max_size, &enum_names,
                                     &enum_rids, &num_entries);

        if (status == STATUS_SUCCESS ||
            status == STATUS_MORE_ENTRIES)
            DISPLAY_ACCOUNTS("User", account_flags, enum_names, enum_rids,
                             num_entries);

        if (enum_names) SamrFreeMemory((void*)enum_names);
        if (enum_rids) SamrFreeMemory((void*)enum_rids);

    } while (status == STATUS_MORE_ENTRIES);

    resume = 0;
    account_flags = ACB_DOMTRUST;
    do {
        status = SamrEnumDomainUsers(samr_binding, hDomain, &resume,
                                     account_flags, max_size, &enum_names,
                                     &enum_rids, &num_entries);
        if (status == STATUS_SUCCESS ||
            status == STATUS_MORE_ENTRIES)
            DISPLAY_ACCOUNTS("Domain trust", account_flags, enum_names,
                             enum_rids, num_entries);

        if (enum_names) SamrFreeMemory((void*)enum_names);
        if (enum_rids) SamrFreeMemory((void*)enum_rids);

    } while (status == STATUS_MORE_ENTRIES);

    resume = 0;
    account_flags = ACB_WSTRUST;
    do {
        status = SamrEnumDomainUsers(samr_binding, hDomain, &resume,
                                     account_flags, max_size, &enum_names,
                                     &enum_rids, &num_entries);
        if (status == STATUS_SUCCESS ||
            status == STATUS_MORE_ENTRIES)
            DISPLAY_ACCOUNTS("Workstation", account_flags, enum_names,
                             enum_rids, num_entries);

        if (enum_names) SamrFreeMemory((void*)enum_names);
        if (enum_rids) SamrFreeMemory((void*)enum_rids);

    } while (status == STATUS_MORE_ENTRIES);

    resume = 0;
    account_flags = acb_flags;
    do {
        status = SamrEnumDomainUsers(samr_binding, hDomain, &resume,
                                     account_flags, max_size, &enum_names,
                                     &enum_rids, &num_entries);
        if (status == STATUS_SUCCESS ||
            status == STATUS_MORE_ENTRIES)
            DISPLAY_ACCOUNTS("Account", account_flags,
                             enum_names, enum_rids, num_entries);

        if (enum_names) SamrFreeMemory((void*)enum_names);
        if (enum_rids) SamrFreeMemory((void*)enum_rids);

    } while (status == STATUS_MORE_ENTRIES);


    SamrClose(samr_binding, hDomain);
    if (status != 0) rpc_fail(status);

    SamrClose(samr_binding, hConn);
    if (status != 0) rpc_fail(status);

    FreeSamrBinding(&samr_binding);
    RELEASE_SESSION_CREDS;

done:
    if (sid) {
        SamrFreeMemory((void*)sid);
    }

    SAFE_FREE(domname);
    SAFE_FREE(domainname);

    return (status == STATUS_SUCCESS);
}


int TestSamrQueryDisplayInfo(struct test *t, const wchar16_t *hostname,
                             const wchar16_t *user, const wchar16_t *pass,
                             struct parameter *options, int optcount)
{
    const uint32 conn_access_mask = SAMR_ACCESS_OPEN_DOMAIN |
                                    SAMR_ACCESS_ENUM_DOMAINS |
                                    SAMR_ACCESS_CONNECT_TO_SERVER;

    const uint32 dom_access_mask = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                   DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                   DOMAIN_ACCESS_CREATE_USER |
                                   DOMAIN_ACCESS_CREATE_ALIAS |
                                   DOMAIN_ACCESS_LOOKUP_INFO_2;

    const int def_specifydomain = 0;
    const char *def_domainname = "BUILTIN";
    const int def_level = 0;
    const int def_max_entries = 64;
    const int def_buf_size = 512;

    NTSTATUS status = STATUS_SUCCESS;
    handle_t samr_binding = NULL;
    enum param_err perr = perr_success;
    wchar16_t *domname = NULL;
    wchar16_t *domainname = NULL;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    PSID sid = NULL;
    int specifydomain = 0;
    int32 level = 0;
    uint32 start_idx = 0;
    uint32 max_entries = 0;
    uint32 buf_size = 0;
    uint32 total_size = 0;
    uint32 returned_size = 0;
    SamrDisplayInfo *info = NULL;

    perr = fetch_value(options, optcount, "specifydomain", pt_int32,
                       &specifydomain, &def_specifydomain);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "domainname", pt_w16string,
                       &domainname, &def_domainname);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "level", pt_int32, &level,
                       &def_level);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "maxentries", pt_int32, &max_entries,
                       &def_max_entries);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "maxsize", pt_int32, &buf_size,
                       &def_buf_size);
    if (!perr_is_ok(perr)) perr_fail(perr);

    SET_SESSION_CREDS(hCreds);

    samr_binding = CreateSamrBinding(&samr_binding, hostname);
    if (samr_binding == NULL) return false;

    status = SamrConnect2(samr_binding, hostname, conn_access_mask,
                          &hConn);
    if (status != 0) rpc_fail(status);

    if (specifydomain) {
        domname = wc16sdup(domainname);

    } else {
        status = GetSamDomainName(&domname, hostname);
        if (status != 0) rpc_fail(status);
    }

    status = SamrLookupDomain(samr_binding, hConn, domname, &sid);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_binding, hConn, dom_access_mask,
                            sid, &hDomain);
    if (status != 0) rpc_fail(status);

    if (level == 0) {
        for (level = 1; level <= 5; level++) {
            start_idx = 0;

            do {
                CALL_MSRPC(status = SamrQueryDisplayInfo(samr_binding,
                                                         hDomain,
                                                         (uint16)level,
                                                         start_idx,
                                                         max_entries,
                                                         buf_size,
                                                         &total_size,
                                                         &returned_size,
                                                         &info));
                switch (level) {
                case 1:
                    if (info && info->info1.count) {
                        start_idx =
                            info->info1.entries[info->info1.count - 1].idx + 1;
                    }
                    break;
                case 2:
                    if (info && info->info2.count) {
                        start_idx =
                            info->info2.entries[info->info2.count - 1].idx + 1;
                    }
                    break;
                case 3:
                    if (info && info->info3.count) {
                        start_idx =
                            info->info3.entries[info->info3.count - 1].idx + 1;
                    }
                    break;
                case 4:
                    if (info && info->info4.count) {
                        start_idx =
                            info->info4.entries[info->info4.count - 1].idx + 1;
                    }
                    break;
                case 5:
                    if (info && info->info5.count) {
                        start_idx =
                            info->info5.entries[info->info5.count - 1].idx + 1;
                    }
                    break;
                }

                if (info) {
                    SamrFreeMemory(info);
                }
            } while (status == STATUS_MORE_ENTRIES);
        }

    } else {
        do {
            CALL_MSRPC(status = SamrQueryDisplayInfo(samr_binding, hDomain,
                                                     (uint16)level, start_idx,
                                                     max_entries, buf_size,
                                                     &total_size, &returned_size,
                                                     &info));
            switch (level) {
            case 1:
                if (info && info->info1.count) {
                    start_idx =
                        info->info1.entries[info->info1.count - 1].idx + 1;
                }
                break;
            case 2:
                if (info && info->info2.count) {
                    start_idx =
                        info->info2.entries[info->info2.count - 1].idx + 1;
                }
                break;
            case 3:
                if (info && info->info3.count) {
                    start_idx =
                        info->info3.entries[info->info3.count - 1].idx + 1;
                }
                break;
            case 4:
                if (info && info->info4.count) {
                    start_idx =
                        info->info4.entries[info->info4.count - 1].idx + 1;
                }
                break;
            case 5:
                if (info && info->info5.count) {
                    start_idx =
                        info->info5.entries[info->info5.count - 1].idx + 1;
                }
                break;
            }

            if (info) {
                SamrFreeMemory(info);
            }

        } while (status == STATUS_MORE_ENTRIES);
    }

    SamrClose(samr_binding, hDomain);
    if (status != 0) rpc_fail(status);

    SamrClose(samr_binding, hConn);
    if (status != 0) rpc_fail(status);

    FreeSamrBinding(&samr_binding);
    RELEASE_SESSION_CREDS;

done:
    if (sid) {
        SamrFreeMemory((void*)sid);
    }

    SAFE_FREE(domname);
    SAFE_FREE(domainname);

    return (status == STATUS_SUCCESS);
}


int TestSamrEnumDomains(struct test *t, const wchar16_t *hostname,
                        const wchar16_t *user, const wchar16_t *pass,
                        struct parameter *options, int optcount)
{
    const uint32 conn_access_mask = SAMR_ACCESS_OPEN_DOMAIN |
                                    SAMR_ACCESS_ENUM_DOMAINS |
                                    SAMR_ACCESS_CONNECT_TO_SERVER;

    const uint32 def_max_size = (uint32)(-1);

    NTSTATUS status = STATUS_SUCCESS;
    enum param_err perr = perr_success;
    handle_t samr_binding = NULL;
    uint32 resume = 0;
    uint32 num_entries = 0;
    uint32 max_size = 0;
    wchar16_t **enum_domains = NULL;
    CONNECT_HANDLE hConn = NULL;

    perr = fetch_value(options, optcount, "maxsize", pt_uint32,
                       &max_size, &def_max_size);
    if (!perr_is_ok(perr)) perr_fail(perr);

    SET_SESSION_CREDS(hCreds);

    samr_binding = CreateSamrBinding(&samr_binding, hostname);
    if (samr_binding == NULL) return false;

    status = SamrConnect2(samr_binding, hostname, conn_access_mask,
                          &hConn);
    if (status != 0) rpc_fail(status);

    resume = 0;

    do {
        status = SamrEnumDomains(samr_binding, hConn, &resume, max_size,
                                 &enum_domains, &num_entries);

        DISPLAY_DOMAINS(enum_domains, num_entries);

        if (enum_domains) SamrFreeMemory((void*)enum_domains);

    } while (status == STATUS_MORE_ENTRIES);

    status = SamrClose(samr_binding, hConn);
    if (status != 0) rpc_fail(status);

    FreeSamrBinding(&samr_binding);
    RELEASE_SESSION_CREDS;

done:
    return (status == STATUS_SUCCESS);
}


int TestSamrCreateUserAccount(struct test *t, const wchar16_t *hostname,
                              const wchar16_t *user, const wchar16_t *pass,
                              struct parameter *options, int optcount)
{
    const uint32 conn_access_mask = SAMR_ACCESS_OPEN_DOMAIN |
                                    SAMR_ACCESS_ENUM_DOMAINS |
                                    SAMR_ACCESS_CONNECT_TO_SERVER;

    const uint32 dom_access_mask = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                   DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                   DOMAIN_ACCESS_CREATE_USER |
                                   DOMAIN_ACCESS_CREATE_ALIAS |
                                   DOMAIN_ACCESS_LOOKUP_INFO_2;

    const uint32 usr_access_mask = USER_ACCESS_GET_NAME_ETC |
                                   USER_ACCESS_GET_LOCALE |
                                   USER_ACCESS_GET_LOGONINFO |
                                   USER_ACCESS_GET_ATTRIBUTES |
                                   USER_ACCESS_CHANGE_PASSWORD |
                                   DELETE;

    const uint32 account_flags = ACB_NORMAL;
    const char *newuser = "Testuser";

    NTSTATUS status = STATUS_SUCCESS;
    enum param_err perr = perr_success;
    handle_t samr_binding = NULL;
    wchar16_t *newusername = NULL;
    wchar16_t *domname = NULL;
    uint32 rid = 0;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    ACCOUNT_HANDLE hAccount = NULL;
    PSID sid = NULL;

    perr = fetch_value(options, optcount, "username", pt_w16string,
                       &newusername, &newuser);
    if (!perr_is_ok(perr)) perr_fail(perr);


    SET_SESSION_CREDS(hCreds);

    /*
     * Make sure there's no account with the same name already
     */
    status = CleanupAccount(hostname, newusername);
    if (status != 0) goto done;

    samr_binding = CreateSamrBinding(&samr_binding, hostname);
    if (samr_binding == NULL) return false;

    /*
     * Creating and deleting user account
     */

    status = SamrConnect2(samr_binding, hostname, conn_access_mask,
                          &hConn);
    if (status != 0) rpc_fail(status);

    status = GetSamDomainName(&domname, hostname);
    if (status != 0) rpc_fail(status);

    status = SamrLookupDomain(samr_binding, hConn, domname, &sid);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_binding, hConn, dom_access_mask,
                            sid, &hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrCreateUser(samr_binding, hDomain, newusername,
                            account_flags, &hAccount, &rid);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hAccount);
    if (status != 0) rpc_fail(status);

    status = SamrOpenUser(samr_binding, hDomain, usr_access_mask,
                          rid, &hAccount);
    if (status != 0) rpc_fail(status);

    status = SamrDeleteUser(samr_binding, hAccount);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hConn);
    if (status != 0) rpc_fail(status);

    FreeSamrBinding(&samr_binding);
    RELEASE_SESSION_CREDS;

done:
    if (sid) SamrFreeMemory((void*)sid);

    SAFE_FREE(newusername);
    SAFE_FREE(domname);

    return (status == STATUS_SUCCESS);
}


int TestSamrCreateAlias(struct test *t, const wchar16_t *hostname,
                        const wchar16_t *user, const wchar16_t *pass,
                        struct parameter *options, int optcount)
{
    const uint32 conn_access_mask = SAMR_ACCESS_OPEN_DOMAIN |
                                    SAMR_ACCESS_ENUM_DOMAINS |
                                    SAMR_ACCESS_CONNECT_TO_SERVER;

    const uint32 dom_access_mask = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                   DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                   DOMAIN_ACCESS_CREATE_USER |
                                   DOMAIN_ACCESS_CREATE_ALIAS |
                                   DOMAIN_ACCESS_LOOKUP_INFO_2;

    const uint32 alias_access = ALIAS_ACCESS_LOOKUP_INFO |
                                ALIAS_ACCESS_SET_INFO |
                                DELETE;

    const char *def_aliasname = "Testalias";

    NTSTATUS status = STATUS_SUCCESS;
    enum param_err perr = perr_success;
    handle_t samr_binding = NULL;
    wchar16_t *newaliasname = NULL;
    wchar16_t *domname = NULL;
    uint32 rid = 0;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    ACCOUNT_HANDLE hAccount = NULL;
    PSID sid = NULL;

    perr = fetch_value(options, optcount, "aliasname", pt_w16string,
                       &newaliasname, &def_aliasname);
    if (!perr_is_ok(perr)) perr_fail(perr);

    SET_SESSION_CREDS(hCreds);

    status = CleanupAlias(hostname, newaliasname);
    if (status != 0) rpc_fail(status);

    samr_binding = CreateSamrBinding(&samr_binding, hostname);
    if (samr_binding == NULL) return false;

    status = SamrConnect2(samr_binding, hostname, conn_access_mask,
                          &hConn);
    if (status != 0) rpc_fail(status);

    status = GetSamDomainName(&domname, hostname);
    if (status != 0) rpc_fail(status);

    status = SamrLookupDomain(samr_binding, hConn, domname, &sid);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_binding, hConn, dom_access_mask,
                            sid, &hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrCreateDomAlias(samr_binding, hDomain, newaliasname,
                                alias_access, &hAccount, &rid);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hAccount);
    if (status != 0) rpc_fail(status);

    status = SamrOpenAlias(samr_binding, hDomain, alias_access, rid,
                           &hAccount);
    if (status != 0) rpc_fail(status);

    status = SamrDeleteDomAlias(samr_binding, hAccount);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hConn);
    if (status != 0) rpc_fail(status);

    FreeSamrBinding(&samr_binding);
    RELEASE_SESSION_CREDS;

done:
    if (sid) SamrFreeMemory((void*)sid);

    SAFE_FREE(domname);
    SAFE_FREE(newaliasname);

    return (status == STATUS_SUCCESS);
}


int TestSamrCreateGroup(struct test *t, const wchar16_t *hostname,
                        const wchar16_t *user, const wchar16_t *pass,
                        struct parameter *options, int optcount)
{
    const uint32 conn_access_mask = SAMR_ACCESS_OPEN_DOMAIN |
                                    SAMR_ACCESS_ENUM_DOMAINS |
                                    SAMR_ACCESS_CONNECT_TO_SERVER;

    const uint32 dom_access_mask = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                   DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                   DOMAIN_ACCESS_CREATE_USER |
                                   DOMAIN_ACCESS_CREATE_ALIAS |
                                   DOMAIN_ACCESS_CREATE_GROUP |
                                   DOMAIN_ACCESS_LOOKUP_INFO_2;

    const uint32 group_access = GROUP_ACCESS_LOOKUP_INFO |
                                GROUP_ACCESS_SET_INFO |
                                DELETE;

    const char *def_groupname = "Testgroup";

    NTSTATUS status = STATUS_SUCCESS;
    enum param_err perr = perr_success;
    handle_t samr_binding = NULL;
    wchar16_t *newgroupname = NULL;
    wchar16_t *domname = NULL;
    uint32 rid = 0;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    ACCOUNT_HANDLE hAccount = NULL;
    PSID sid = NULL;

    perr = fetch_value(options, optcount, "groupname", pt_w16string,
                       &newgroupname, &def_groupname);
    if (!perr_is_ok(perr)) perr_fail(perr);

    SET_SESSION_CREDS(hCreds);

    samr_binding = CreateSamrBinding(&samr_binding, hostname);
    if (samr_binding == NULL) return false;

    status = SamrConnect2(samr_binding, hostname, conn_access_mask,
                          &hConn);
    if (status != 0) rpc_fail(status);

    status = GetSamDomainName(&domname, hostname);
    if (status != 0) rpc_fail(status);

    status = SamrLookupDomain(samr_binding, hConn, domname, &sid);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_binding, hConn, dom_access_mask,
                            sid, &hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrCreateDomGroup(samr_binding, hDomain, newgroupname,
                                group_access, &hAccount, &rid);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hAccount);
    if (status != 0) rpc_fail(status);

    status = SamrOpenGroup(samr_binding, hDomain, group_access, rid,
                           &hAccount);
    if (status != 0) rpc_fail(status);

    status = SamrDeleteDomGroup(samr_binding, hAccount);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hConn);
    if (status != 0) rpc_fail(status);

    FreeSamrBinding(&samr_binding);
    RELEASE_SESSION_CREDS;

done:
    if (sid) SamrFreeMemory((void*)sid);

    SAFE_FREE(domname);
    SAFE_FREE(newgroupname);

    return (status == STATUS_SUCCESS);
}


int TestSamrSetUserPassword(struct test *t, const wchar16_t *hostname,
                            const wchar16_t *user, const wchar16_t *pass,
                            struct parameter *options, int optcount)
{
    const uint32 conn_access_mask = SAMR_ACCESS_OPEN_DOMAIN |
                                    SAMR_ACCESS_ENUM_DOMAINS |
                                    SAMR_ACCESS_CONNECT_TO_SERVER;
    const uint32 dom_access_mask = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                   DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                   DOMAIN_ACCESS_CREATE_USER |
                                   DOMAIN_ACCESS_CREATE_ALIAS |
                                   DOMAIN_ACCESS_LOOKUP_INFO_1;

    const uint32 usr_access_mask = USER_ACCESS_GET_NAME_ETC |
                                   USER_ACCESS_GET_LOCALE |
                                   USER_ACCESS_GET_LOGONINFO |
                                   USER_ACCESS_GET_ATTRIBUTES |
                                   USER_ACCESS_CHANGE_PASSWORD |
                                   USER_ACCESS_SET_PASSWORD |
                                   USER_ACCESS_SET_ATTRIBUTES |
                                   DELETE;

    const char *newuser = "Testuser";
    const char *testpass = "JustTesting30$";
    const uint32 def_primary_gid = 513;

    NTSTATUS status = STATUS_SUCCESS;
    RPCSTATUS rpcstatus;
    enum param_err perr = perr_success;
    handle_t samr_binding;
    int newacct;
    wchar16_t *newusername, *domname;
    uint32 rid;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    ACCOUNT_HANDLE hAccount = NULL;
    PSID sid = NULL;
    uint32 *rids, *types, rids_count;
    wchar16_t *names[1];
    UserInfo userinfo;
    UserInfo26 *info26 = NULL;
    unsigned char initval[16] = {0};
    wchar16_t *password;
    unsigned char *sess_key;
    unsigned16 sess_key_len;
    unsigned char digested_sess_key[16] = {0};
    struct md5context ctx;
    UserInfo9 *info9 = NULL;
    uint32 primary_gid = 0;

    memset((void*)&userinfo, 0, sizeof(userinfo));

    perr = fetch_value(options, optcount, "username", pt_w16string,
                       &newusername, &newuser);
    if (!perr_is_ok(perr)) perr_fail(perr)

    perr = fetch_value(options, optcount, "password", pt_w16string,
                       &password, &testpass);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "primarygid", pt_uint32,
                       &primary_gid, &def_primary_gid);
    if (!perr_is_ok(perr)) perr_fail(perr);

    SET_SESSION_CREDS(hCreds);

    samr_binding = CreateSamrBinding(&samr_binding, hostname);
    if (samr_binding == NULL) return false;

    status = EnsureUserAccount(hostname, newusername, &newacct);
    if (status != 0) rpc_fail(status);

    /*
     * Creating and deleting user account
     */

    status = SamrConnect2(samr_binding, hostname, conn_access_mask,
                          &hConn);
    if (status != 0) rpc_fail(status);

    GetSessionKey(samr_binding, &sess_key, &sess_key_len, &rpcstatus);
    if (rpcstatus != 0) return false;

    status = GetSamDomainName(&domname, hostname);
    if (status != 0) rpc_fail(status);

    status = SamrLookupDomain(samr_binding, hConn, domname, &sid);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_binding, hConn, dom_access_mask,
                            sid, &hDomain);
    if (status != 0) rpc_fail(status);

    names[0] = newusername;

    status = SamrLookupNames(samr_binding, hDomain, 1, names, &rids, &types,
                             &rids_count);
    if (status != 0) rpc_fail(status);

    rid = rids[0];

    status = SamrOpenUser(samr_binding, hDomain, usr_access_mask,
                          rid, &hAccount);
    if (status != 0) rpc_fail(status);

    info9 = &userinfo.info9;
    info9->primary_gid = primary_gid;

    status = SamrSetUserInfo(samr_binding, hAccount, 9, &userinfo);
    if (status != 0) rpc_fail(status);

    info26 = &userinfo.info26;
    EncodePassBufferW16(info26->password.data, password);
    info26->password_len = strlen(testpass);

    memset(initval, 0, sizeof(initval));

    md5init(&ctx);
    md5update(&ctx, initval, 16);
    md5update(&ctx, sess_key, (unsigned int)sess_key_len);
    md5final(&ctx, digested_sess_key);

    rc4(info26->password.data, 516, digested_sess_key, 16);
    memcpy((void*)&info26->password.data, initval, 16);

    status = SamrSetUserInfo(samr_binding, hAccount, 26, &userinfo);
    if (status != 0) rpc_fail(status);

    if (newacct) {
        status = SamrDeleteUser(samr_binding, hAccount);
        if (status != 0) rpc_fail(status);
    }

    status = SamrClose(samr_binding, hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding, hConn);
    if (status != 0) rpc_fail(status);

    FreeSamrBinding(&samr_binding);
    RELEASE_SESSION_CREDS;

done:
    if (sid) SamrFreeMemory((void*)sid);
    if (rids) SamrFreeMemory((void*)rids);
    if (types) SamrFreeMemory((void*)types);

    SAFE_FREE(newusername);
    SAFE_FREE(password);
    SAFE_FREE(domname);
    SAFE_FREE(sess_key);

    return (status == STATUS_SUCCESS);
}


int TestSamrMultipleConnections(struct test *t, const wchar16_t *hostname,
                                const wchar16_t *user, const wchar16_t *pass,
                                struct parameter *options, int optcount)
{
    const uint32 conn_access = SAMR_ACCESS_OPEN_DOMAIN |
                               SAMR_ACCESS_ENUM_DOMAINS |
                               SAMR_ACCESS_CONNECT_TO_SERVER;
    NTSTATUS status = STATUS_SUCCESS;
    handle_t samr_binding1 = NULL;
    handle_t samr_binding2 = NULL;
    CONNECT_HANDLE hConn1 = NULL;
    CONNECT_HANDLE hConn2 = NULL;
    unsigned char *key1 = NULL;
    unsigned char *key2 = NULL;
    unsigned16 key_len1, key_len2;
    RPCSTATUS st = 0;

    samr_binding1 = NULL;
    samr_binding2 = NULL;

    SET_SESSION_CREDS(hCreds);

    samr_binding1 = CreateSamrBinding(&samr_binding1, hostname);
    if (samr_binding1 == NULL) return false;

    status = SamrConnect2(samr_binding1, hostname, conn_access, &hConn1);
    if (status != 0) rpc_fail(status);

    GetSessionKey(samr_binding1, &key1, &key_len1, &st);
    if (st != 0) return false;

    samr_binding2 = CreateSamrBinding(&samr_binding2, hostname);
    if (samr_binding2 == NULL) return false;

    status = SamrConnect2(samr_binding2, hostname, conn_access, &hConn2);
    if (status != 0) rpc_fail(status);

    GetSessionKey(samr_binding2, &key2, &key_len2, &st);
    if (st != 0) return false;

    status = SamrClose(samr_binding1, hConn1);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_binding2, hConn2);
    if (status != 0) rpc_fail(status);

    FreeSamrBinding(&samr_binding1);
    FreeSamrBinding(&samr_binding2);

    RELEASE_SESSION_CREDS;

done:
    return (status == STATUS_SUCCESS);
}


int TestSamrChangeUserPassword(struct test *t, const wchar16_t *hostname,
                               const wchar16_t *user, const wchar16_t *pass,
                               struct parameter *options, int optcount)
{
    const char *defusername = "TestUser";
    const char *defoldpass = "";
    const char *defnewpass = "secret";

    NTSTATUS status = STATUS_SUCCESS;
    enum param_err perr = perr_success;
    handle_t samr_b = NULL;
    wchar16_t *username = NULL;
    wchar16_t *oldpassword = NULL;
    wchar16_t *newpassword = NULL;
    uint8 old_nthash[16] = {0};
    uint8 new_nthash[16] = {0};
    uint8 old_lmhash[16] = {0};
    size_t oldlen, newlen;
    uint8 ntpassbuf[516] = {0};
    uint8 ntverhash[16] = {0};

    perr = fetch_value(options, optcount, "username", pt_w16string,
                       &username, &defusername);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "oldpass", pt_w16string,
                       &oldpassword, &defoldpass);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "newpass", pt_w16string,
                       &newpassword, &defnewpass);
    if (!perr_is_ok(perr)) perr_fail(perr);

    SET_SESSION_CREDS(hCreds);

    samr_b = CreateSamrBinding(&samr_b, hostname);
    if (samr_b == NULL) return STATUS_UNSUCCESSFUL;

    oldlen = wc16slen(oldpassword);
    newlen = wc16slen(newpassword);

    /* prepare NT password hashes */
    md4hash(old_nthash, oldpassword);
    md4hash(new_nthash, newpassword);

    /* prepare LM password hash */
    deshash(old_lmhash, oldpassword);

    /* encode password buffer */
    EncodePassBufferW16(ntpassbuf, newpassword);
    rc4(ntpassbuf, 516, old_nthash, 16);

    /* encode NT verifier */
    des56(ntverhash, old_nthash, 8, new_nthash);
    des56(&ntverhash[8], &old_nthash[8], 8, &new_nthash[7]);

    status = SamrChangePasswordUser2(samr_b, hostname, username, ntpassbuf,
                                     ntverhash, 0, NULL, NULL);
    if (status != 0) rpc_fail(status);

    FreeSamrBinding(&samr_b);
    RELEASE_SESSION_CREDS;

done:

    SAFE_FREE(username);
    SAFE_FREE(oldpassword);
    SAFE_FREE(newpassword);

    return (status == STATUS_SUCCESS);
}


int TestSamrEnumAliases(struct test *t, const wchar16_t *hostname,
                        const wchar16_t *user, const wchar16_t *pass,
                        struct parameter *options, int optcount)
{
    const uint32 conn_access_mask = SAMR_ACCESS_OPEN_DOMAIN |
                                    SAMR_ACCESS_ENUM_DOMAINS |
                                    SAMR_ACCESS_CONNECT_TO_SERVER;

    const uint32 dom_access_mask = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                   DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                   DOMAIN_ACCESS_CREATE_USER |
                                   DOMAIN_ACCESS_CREATE_ALIAS |
                                   DOMAIN_ACCESS_LOOKUP_INFO_2;

    const int def_specifydomain = 0;
    const char *def_domainname = "BUILTIN";

    NTSTATUS status = STATUS_SUCCESS;
    handle_t samr_binding;
    enum param_err perr = perr_success;
    uint32 resume, num_entries, max_size;
    uint32 account_flags;
    wchar16_t **enum_names, *domname, *domainname;
    uint32 *enum_rids;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    PSID sid = NULL;
    int specifydomain;

    perr = fetch_value(options, optcount, "specifydomain", pt_int32,
                       &specifydomain, &def_specifydomain);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "domainname", pt_w16string,
                       &domainname, &def_domainname);
    if (!perr_is_ok(perr)) perr_fail(perr);


    SET_SESSION_CREDS(hCreds);

    samr_binding = CreateSamrBinding(&samr_binding, hostname);
    if (samr_binding == NULL) return false;

    status = SamrConnect2(samr_binding, hostname, conn_access_mask,
                          &hConn);
    if (status != 0) rpc_fail(status);

    if (specifydomain) {
        domname = wc16sdup(domainname);

    } else {
        status = GetSamDomainName(&domname, hostname);
        if (status != 0) rpc_fail(status);
    }

    status = SamrLookupDomain(samr_binding, hConn, domname, &sid);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_binding, hConn, dom_access_mask,
                            sid, &hDomain);
    if (status != 0) rpc_fail(status);

    /*
     * Enumerating domain aliases
     */

    max_size = 128;
    resume = 0;
    account_flags = ACB_NORMAL;
    do {
        enum_names = NULL;
        enum_rids  = NULL;

        status = SamrEnumDomainAliases(samr_binding, hDomain, &resume,
                                       account_flags, &enum_names, &enum_rids,
                                       &num_entries);

        if (status == STATUS_SUCCESS ||
            status == STATUS_MORE_ENTRIES)
            DISPLAY_ACCOUNTS("Alias", account_flags, enum_names, enum_rids,
                             num_entries);

        if (enum_names) SamrFreeMemory((void*)enum_names);
        if (enum_rids) SamrFreeMemory((void*)enum_rids);

    } while (status == STATUS_MORE_ENTRIES);

    SamrClose(samr_binding, hDomain);
    if (status != 0) rpc_fail(status);

    SamrClose(samr_binding, hConn);
    if (status != 0) rpc_fail(status);

    RELEASE_SESSION_CREDS;

done:
    if (sid) SamrFreeMemory((void*)sid);

    SAFE_FREE(domname);
    SAFE_FREE(domainname);

    return true;
}


int TestSamrGetUserGroups(struct test *t, const wchar16_t *hostname,
                          const wchar16_t *user, const wchar16_t *pass,
                          struct parameter *options, int optcount)
{
    const uint32 conn_access = SAMR_ACCESS_OPEN_DOMAIN |
                               SAMR_ACCESS_ENUM_DOMAINS;
    const uint32 domain_access = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                 DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                 DOMAIN_ACCESS_CREATE_USER |
                                 DOMAIN_ACCESS_LOOKUP_INFO_2;

    const uint32 user_access = USER_ACCESS_GET_NAME_ETC |
                               USER_ACCESS_GET_LOCALE |
                               USER_ACCESS_GET_LOGONINFO |
                               USER_ACCESS_GET_ATTRIBUTES |
                               USER_ACCESS_GET_GROUPS |
                               USER_ACCESS_GET_GROUP_MEMBERSHIP;

    const uint32 lsa_access = LSA_ACCESS_LOOKUP_NAMES_SIDS;

    const char *def_username = "Guest";
    const int def_resolvesids = 0;
    const uint32 def_resolvelevel = 6;

    NTSTATUS status = STATUS_SUCCESS;
    enum param_err perr = perr_success;
    handle_t samr_b = NULL;
    handle_t lsa_b = NULL;
    PSID domsid = NULL;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    ACCOUNT_HANDLE hUser = NULL;
    POLICY_HANDLE hPolicy = NULL;
    wchar16_t *username = NULL;
    int resolvesids = 0;
    uint32 resolve_level = 0;
    wchar16_t *names[1] = {0};
    uint32 *rids = NULL;
    uint32 *types = NULL;
    uint32 rids_count = 0;
    uint32 *grp_rids = NULL;
    uint32 *grp_attrs = NULL;
    uint32 grp_count = 0;
    int i = 0;
    PSID* grp_sids = NULL;
    wchar16_t **grp_sidstrs = NULL;
    SidArray sid_array = {0};
    RefDomainList *domains = NULL;
    TranslatedName *trans_names = NULL;
    uint32 names_count = 0;
    wchar16_t *grp_name = NULL;
    wchar16_t *dom_name = NULL;

    perr = fetch_value(options, optcount, "username", pt_w16string,
                       &username, &def_username);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "resolvesids", pt_int32,
                       &resolvesids, &def_resolvesids);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "resolvelevel", pt_uint32,
                       &resolve_level, &def_resolvelevel);
    if (!perr_is_ok(perr)) perr_fail(perr);

    PARAM_INFO("username", pt_w16string, username);
    PARAM_INFO("resolvesids", pt_int32, &resolvesids);
    PARAM_INFO("resolvelevel", pt_uint32, &resolve_level);

    SET_SESSION_CREDS(hCreds);

    status = GetSamDomainSid(&domsid, hostname);
    if (status != 0) rpc_fail(status);

    samr_b = CreateSamrBinding(&samr_b, hostname);
    if (samr_b == NULL) return false;

    status = SamrConnect2(samr_b, hostname, conn_access, &hConn);
    if (status != 0) rpc_fail(status);

    status = SamrOpenDomain(samr_b, hConn, domain_access, domsid, &hDomain);
    if (status != 0) rpc_fail(status);

    names[0] = username;
    status = SamrLookupNames(samr_b, hDomain, 1, names, &rids, &types,
                             &rids_count);
    if (status != 0) rpc_fail(status);

    status = SamrOpenUser(samr_b, hDomain, user_access, rids[0], &hUser);
    if (status != 0) rpc_fail(status);

    INPUT_ARG_PTR(samr_b);
    INPUT_ARG_PTR(hUser);
    INPUT_ARG_PTR(grp_rids);
    INPUT_ARG_PTR(grp_attrs);

    CALL_MSRPC(status = SamrGetUserGroups(samr_b, hUser, &grp_rids,
                                          &grp_attrs, &grp_count));

    OUTPUT_ARG_PTR(grp_rids);
    OUTPUT_ARG_PTR(grp_attrs);
    OUTPUT_ARG_PTR(grp_count);

    grp_sids = (PSID*) malloc(sizeof(PSID) * grp_count);
    if (grp_sids == NULL) rpc_fail(STATUS_NO_MEMORY);

    grp_sidstrs = (wchar16_t**) malloc(sizeof(wchar16_t*) * grp_count);
    if (grp_sidstrs == NULL) rpc_fail(STATUS_NO_MEMORY);

    if (resolvesids) {
        sid_array.num_sids = grp_count;
        sid_array.sids = (SidPtr*) malloc(sizeof(SidPtr) * grp_count);
        if (sid_array.sids == NULL) rpc_fail(STATUS_NO_MEMORY);
    }

    for (i = 0; i < grp_count; i++) {
        status = MsRpcAllocateSidAppendRid(&grp_sids[i],
                                           domsid,
                                           grp_rids[i]);
        if (status != 0) rpc_fail(status);

        status = RtlAllocateWC16StringFromSid(&grp_sidstrs[i], grp_sids[i]);
        if (status != 0) rpc_fail(status);

        if (resolvesids) {
            sid_array.sids[i].sid = grp_sids[i];
        }
    }

    if (resolvesids) {
        lsa_b = CreateLsaBinding(&lsa_b, hostname);
        if (lsa_b == NULL) rpc_fail(STATUS_UNSUCCESSFUL);

        status = LsaOpenPolicy2(lsa_b, hostname, NULL, lsa_access, &hPolicy);
        if (status != 0) rpc_fail(status);

        status = LsaLookupSids(lsa_b, hPolicy, &sid_array, &domains,
                               &trans_names, resolve_level, &names_count);
        if (status != 0) rpc_fail(status);
    }

    for (i = 0; i < grp_count; i++) {
        w16printfw(L"%ws", grp_sidstrs[i]);

        if (resolvesids && i < names_count) {
            LsaDomainInfo *di = NULL;
            TranslatedName *tn = &(trans_names[i]);

            grp_name = GetFromUnicodeString(&tn->name);
            if (grp_name == NULL) rpc_fail(STATUS_NO_MEMORY);

            di = &(domains->domains[tn->sid_index]);
            dom_name = GetFromUnicodeStringEx(&di->name);
            if (dom_name == NULL) rpc_fail(STATUS_NO_MEMORY);

            w16printfw(L" [%ws\\%ws]", dom_name, grp_name);

            SAFE_FREE(grp_name);
            SAFE_FREE(dom_name);
        }

        printf("\n");
    }

    status = SamrClose(samr_b, hUser);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_b, hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_b, hConn);
    if (status != 0) rpc_fail(status);

    if (resolvesids) {
        status = LsaClose(lsa_b, hPolicy);
        if (status != 0) rpc_fail(status);

        FreeLsaBinding(&lsa_b);
    }

    FreeSamrBinding(&samr_b);
    RELEASE_SESSION_CREDS;

done:
    SAFE_FREE(username);
    if (domsid) MsRpcFreeSid(domsid);
    if (rids) SamrFreeMemory((void*)rids);
    if (types) SamrFreeMemory((void*)types);
    if (grp_rids) SamrFreeMemory((void*)grp_rids);
    if (grp_attrs) SamrFreeMemory((void*)grp_attrs);

    if (trans_names) LsaRpcFreeMemory((void*)trans_names);
    if (domains) LsaRpcFreeMemory((void*)domains);

    for (i = 0; i < grp_count; i++) {
        MsRpcFreeSid(grp_sids[i]);
        RTL_FREE(&grp_sidstrs[i]);
    }
    SAFE_FREE(grp_sids);
    SAFE_FREE(grp_sidstrs);
    SAFE_FREE(sid_array.sids);

    SAFE_FREE(grp_name);
    SAFE_FREE(dom_name);

    SamrDestroyMemory();
    LsaRpcDestroyMemory();

    return (status == STATUS_SUCCESS);
}


int TestSamrGetUserAliases(struct test *t, const wchar16_t *hostname,
                           const wchar16_t *user, const wchar16_t *pass,
                           struct parameter *options, int optcount)
{
    const uint32 conn_access = SAMR_ACCESS_OPEN_DOMAIN |
                               SAMR_ACCESS_ENUM_DOMAINS;
    const uint32 domain_access = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                 DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                 DOMAIN_ACCESS_LOOKUP_INFO_2;

    const uint32 btin_access = DOMAIN_ACCESS_OPEN_ACCOUNT |
                               DOMAIN_ACCESS_ENUM_ACCOUNTS |
                               DOMAIN_ACCESS_LOOKUP_INFO_2;

    const uint32 lsa_access = LSA_ACCESS_LOOKUP_NAMES_SIDS;

    const char *btin_sidstr = "S-1-5-32";

    const char *def_username = "Guest";
    const int def_resolvesids = 0;
    const uint32 def_resolvelevel = 6;

    NTSTATUS status = STATUS_SUCCESS;
    enum param_err perr = perr_success;
    handle_t samr_b = NULL;
    handle_t lsa_b = NULL;
    PSID btinsid = NULL;
    PSID domsid = NULL;
    POLICY_HANDLE hPolicy = NULL;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hBtinDomain = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    wchar16_t *username = NULL;
    int resolvesids = 0;
    wchar16_t *names[1];
    RefDomainList *usr_domains = NULL;
    TranslatedSid2 *trans_sids = NULL;
    PSID usr_sid = NULL;
    uint32 sids_count = 0;
    uint32 level = 0;
    uint32 resolve_level = 0;
    uint32 *btin_rids = NULL;
    uint32 *dom_rids = NULL;
    uint32 btin_rids_count = 0;
    uint32 dom_rids_count = 0;
    SidArray sid_array = {0};
    uint32 alias_count = 0;
    int i = 0;
    wchar16_t **alias_sidstrs = NULL;
    RefDomainList *alias_domains = NULL;
    TranslatedName *trans_names = NULL;
    uint32 names_count = 0;
    wchar16_t *alias_name = NULL;
    wchar16_t *dom_name = NULL;

    perr = fetch_value(options, optcount, "username", pt_w16string,
                       &username, &def_username);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "resolvesids", pt_int32,
                       &resolvesids, &def_resolvesids);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "resolvelevel", pt_uint32,
                       &resolve_level, &def_resolvelevel);
    if (!perr_is_ok(perr)) perr_fail(perr);

    PARAM_INFO("username", pt_w16string, username);
    PARAM_INFO("resolvesids", pt_int32, &resolvesids);
    PARAM_INFO("resolvelevel", pt_uint32, &resolve_level);

    SET_SESSION_CREDS(hCreds);

    /*
     * Resolve username to sid first
     */
    lsa_b = CreateLsaBinding(&lsa_b, hostname);
    if (lsa_b == NULL) rpc_fail(STATUS_UNSUCCESSFUL);

    status = LsaOpenPolicy2(lsa_b, hostname, NULL, lsa_access, &hPolicy);
    if (status != 0) rpc_fail(status);

    names[0] = username;
    level = LSA_LOOKUP_NAMES_ALL;

    status = LsaLookupNames2(lsa_b, hPolicy, 1, names, &usr_domains, &trans_sids,
                             level, &sids_count);
    if (status != 0) rpc_fail(status);

    if (sids_count == 0) rpc_fail(STATUS_UNSUCCESSFUL);

    /* Create user account sid */
    MsRpcDuplicateSid(&domsid, usr_domains->domains[trans_sids[0].index].sid);
    if (domsid == NULL) rpc_fail(STATUS_NO_MEMORY);

    status = MsRpcAllocateSidAppendRid(&usr_sid, domsid, trans_sids[0].rid);
    if (status != 0) rpc_fail(status);

    samr_b = CreateSamrBinding(&samr_b, hostname);
    if (samr_b == NULL) return false;

    status = SamrConnect2(samr_b, hostname, conn_access, &hConn);
    if (status != 0) rpc_fail(status);

    RtlAllocateSidFromCString(&btinsid, btin_sidstr);

    status = SamrOpenDomain(samr_b, hConn, btin_access, btinsid, &hBtinDomain);
    if (status != 0) rpc_fail(status);

    INPUT_ARG_PTR(samr_b);
    INPUT_ARG_PTR(hBtinDomain);
    INPUT_ARG_PTR(usr_sid);
    INPUT_ARG_UINT(sids_count);

    CALL_MSRPC(status = SamrGetAliasMembership(samr_b,
                                               hBtinDomain,
                                               &usr_sid,
                                               sids_count,
                                               &btin_rids,
                                               &btin_rids_count));

    OUTPUT_ARG_PTR(&btin_rids);
    OUTPUT_ARG_UINT(btin_rids_count);

    status = SamrOpenDomain(samr_b, hConn, domain_access, domsid, &hDomain);
    if (status != 0) rpc_fail(status);

    INPUT_ARG_PTR(samr_b);
    INPUT_ARG_PTR(hDomain);
    INPUT_ARG_PTR(usr_sid);
    INPUT_ARG_UINT(sids_count);

    CALL_MSRPC(status = SamrGetAliasMembership(samr_b,
                                               &hDomain,
                                               &usr_sid,
                                               sids_count,
                                               &dom_rids,
                                               &dom_rids_count));

    OUTPUT_ARG_PTR(&dom_rids);
    OUTPUT_ARG_UINT(dom_rids_count);

    alias_count = btin_rids_count + dom_rids_count;
    alias_sidstrs = (wchar16_t**) malloc(sizeof(wchar16_t*) * alias_count);
    if (alias_sidstrs == NULL) rpc_fail(STATUS_NO_MEMORY);

    if (resolvesids) {
        sid_array.num_sids = alias_count;
        sid_array.sids = (SidPtr*) malloc(sizeof(SidPtr) * alias_count);
        if (sid_array.sids == NULL) rpc_fail(STATUS_NO_MEMORY);
    }

    for (i = 0; i < alias_count; i++) {
        PSID alias_sid = NULL;
        PSID dom_sid = (i < btin_rids_count) ? btinsid : domsid;
        uint32 rid = 0;

        if (i < btin_rids_count) {
            rid = btin_rids[i];
        } else {
            rid = dom_rids[i - btin_rids_count];
        }

        status = MsRpcAllocateSidAppendRid(&alias_sid, dom_sid, rid);
        if (status != 0) rpc_fail(status);

        status = RtlAllocateWC16StringFromSid(&alias_sidstrs[i], alias_sid);
        if (status != 0) rpc_fail(status);

        if (resolvesids) {
            sid_array.sids[i].sid = alias_sid;
        }
    };

    if (resolvesids) {
        lsa_b = CreateLsaBinding(&lsa_b, hostname);
        if (lsa_b == NULL) rpc_fail(STATUS_UNSUCCESSFUL);

        status = LsaOpenPolicy2(lsa_b, hostname, NULL, lsa_access, &hPolicy);
        if (status != 0) rpc_fail(status);

        status = LsaLookupSids(lsa_b, hPolicy, &sid_array, &alias_domains,
                               &trans_names, resolve_level, &names_count);
        if (status != 0) rpc_fail(status);
    }

    for (i = 0; i < alias_count; i++) {
        w16printfw(L"%ws", alias_sidstrs[i]);

        if (resolvesids && i < names_count) {
            LsaDomainInfo *di = NULL;
            TranslatedName *tn = &(trans_names[i]);

            alias_name = GetFromUnicodeString(&tn->name);
            if (alias_name == NULL) rpc_fail(STATUS_NO_MEMORY);

            di = &(alias_domains->domains[tn->sid_index]);
            dom_name = GetFromUnicodeStringEx(&di->name);
            if (dom_name == NULL) rpc_fail(STATUS_NO_MEMORY);

            w16printfw(L" [%ws\\%ws]", dom_name, alias_name);

            SAFE_FREE(alias_name);
            SAFE_FREE(dom_name);
        }

        printf("\n");
    }

    status = LsaClose(lsa_b, hPolicy);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_b, hBtinDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_b, hDomain);
    if (status != 0) rpc_fail(status);

    status = SamrClose(samr_b, hConn);
    if (status != 0) rpc_fail(status);

    FreeLsaBinding(&lsa_b);
    FreeSamrBinding(&samr_b);

    RELEASE_SESSION_CREDS;

done:
    SAFE_FREE(username);
    RTL_FREE(&btinsid);
    if (domsid) MsRpcFreeSid(domsid);
    if (usr_sid) MsRpcFreeSid(usr_sid);
    if (usr_domains) LsaRpcFreeMemory((void*)usr_domains);
    if (trans_sids) LsaRpcFreeMemory((void*)trans_sids);
    if (alias_domains) LsaRpcFreeMemory((void*)alias_domains);
    if (trans_names) LsaRpcFreeMemory((void*)trans_names);
    if (btin_rids) SamrFreeMemory((void*)btin_rids);
    if (dom_rids) SamrFreeMemory((void*)dom_rids);

    for (i = 0; i < alias_count; i++) {
        if (resolvesids) {
            MsRpcFreeSid(sid_array.sids[i].sid);
        }
        RTL_FREE(&alias_sidstrs[i]);
    }

    SAFE_FREE(sid_array.sids);
    SAFE_FREE(alias_sidstrs);

    return (status == STATUS_SUCCESS);
}


int TestSamrQuerySecurity(struct test *t, const wchar16_t *hostname,
                          const wchar16_t *user, const wchar16_t *pass,
                          struct parameter *options, int optcount)
{
    const PSTR pszDefSidStr = "S-1-5-32-544";
    const ULONG ulDefSecurityInfo = OWNER_SECURITY_INFORMATION;

    const DWORD dwConnAccess = SAMR_ACCESS_OPEN_DOMAIN |
                               SAMR_ACCESS_ENUM_DOMAINS;
    const DWORD dwDomainAccess = DOMAIN_ACCESS_OPEN_ACCOUNT |
                                 DOMAIN_ACCESS_ENUM_ACCOUNTS |
                                 DOMAIN_ACCESS_CREATE_USER |
                                 DOMAIN_ACCESS_LOOKUP_INFO_2;

    const DWORD dwUserAccess = READ_CONTROL |
                               USER_ACCESS_GET_NAME_ETC |
                               USER_ACCESS_GET_LOCALE |
                               USER_ACCESS_GET_LOGONINFO |
                               USER_ACCESS_GET_ATTRIBUTES |
                               USER_ACCESS_GET_GROUPS |
                               USER_ACCESS_GET_GROUP_MEMBERSHIP;

    NTSTATUS ntStatus = STATUS_SUCCESS;
    enum param_err perr = perr_success;
    PSID pSid = NULL;
    PSID pDomainSid = NULL;
    PWSTR pwszSidStr = NULL;
    ULONG ulRid = 0;
    ULONG ulSecurityInfo = 0;
    handle_t bSamr = NULL;
    CONNECT_HANDLE hConn = NULL;
    DOMAIN_HANDLE hDomain = NULL;
    ACCOUNT_HANDLE hUser = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE pSecDescRel = NULL;
    UINT32 ulSecDescRelLen = 0;
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecDesc = NULL;
    ULONG ulSecDescLen = 1024;
    PACL pDacl = NULL;
    ULONG ulDaclLen = 1024;
    PACL pSacl = NULL;
    ULONG ulSaclLen = 1024;
    PSID pOwnerSid = NULL;
    ULONG ulOwnerSidLen = 1024;
    PSID pGroupSid = NULL;
    ULONG ulGroupSidLen = 1024;

    TESTINFO(t, hostname, user, pass);

    perr = fetch_value(options, optcount, "sid", pt_sid,
                       &pSid, &pszDefSidStr);
    if (!perr_is_ok(perr)) perr_fail(perr);

    perr = fetch_value(options, optcount, "security_info", pt_uint32,
                       &ulSecurityInfo, &ulDefSecurityInfo);
    if (!perr_is_ok(perr)) perr_fail(perr);

    ntStatus = RtlAllocateWC16StringFromSid(&pwszSidStr,
                                            pSid);
    if (ntStatus != 0) rpc_fail(ntStatus);

    PARAM_INFO("sid", pt_w16string, pwszSidStr);
    PARAM_INFO("security_info", pt_uint32, &ulSecurityInfo);

    SET_SESSION_CREDS(hCreds);

    CreateSamrBinding(&bSamr, hostname);
    if (bSamr == NULL) return false;

    ntStatus = SamrConnect2(bSamr,
                            hostname,
                            dwConnAccess,
                            &hConn);
    if (ntStatus != 0) rpc_fail(ntStatus);

    /* Create domain SID from account SID */
    ntStatus = RtlDuplicateSid(&pDomainSid,
                               pSid);
    if (ntStatus != 0) rpc_fail(ntStatus);

    pDomainSid->SubAuthorityCount--;

    ntStatus = SamrOpenDomain(bSamr,
                              hConn,
                              dwDomainAccess,
                              pDomainSid,
                              &hDomain);
    if (ntStatus != 0) rpc_fail(ntStatus);

    ulRid = pSid->SubAuthority[pSid->SubAuthorityCount - 1];

    ntStatus = SamrOpenUser(bSamr,
                            hDomain,
                            dwUserAccess,
                            ulRid,
                            &hUser);
    if (ntStatus != 0) rpc_fail(ntStatus);

    ntStatus = SamrQuerySecurity(bSamr,
                                 hUser,
                                 ulSecurityInfo,
                                 &pSecDescRel,
                                 &ulSecDescRelLen);
    if (ntStatus != 0) rpc_fail(ntStatus);

    ulSecDescLen = 1024;
    ntStatus = RTL_ALLOCATE(&pSecDesc,
                            VOID,
                            ulSecDescLen);
    if (ntStatus != 0) rpc_fail(ntStatus);

    ulDaclLen = 1024;
    ntStatus = RTL_ALLOCATE(&pDacl,
                            VOID,
                            ulDaclLen);
    if (ntStatus != 0) rpc_fail(ntStatus);

    ulSaclLen = 1024;
    ntStatus = RTL_ALLOCATE(&pSacl,
                            VOID,
                            ulSaclLen);
    if (ntStatus != 0) rpc_fail(ntStatus);

    ulOwnerSidLen = 1024;
    ntStatus = RTL_ALLOCATE(&pOwnerSid,
                            VOID,
                            ulOwnerSidLen);
    if (ntStatus != 0) rpc_fail(ntStatus);

    ulGroupSidLen = 1024;
    ntStatus = RTL_ALLOCATE(&pGroupSid,
                            VOID,
                            ulGroupSidLen);
    if (ntStatus != 0) rpc_fail(ntStatus);


    ntStatus = RtlSelfRelativeToAbsoluteSD(pSecDescRel,
                                           pSecDesc,
                                           &ulSecDescLen,
                                           pDacl,
                                           &ulDaclLen,
                                           pSacl,
                                           &ulSaclLen,
                                           pOwnerSid,
                                           &ulOwnerSidLen,
                                           pGroupSid,
                                           &ulGroupSidLen);
    if (ntStatus != 0) rpc_fail(ntStatus);

    ntStatus = SamrClose(bSamr, hUser);
    if (ntStatus != 0) rpc_fail(ntStatus);

    ntStatus = SamrClose(bSamr, hDomain);
    if (ntStatus != 0) rpc_fail(ntStatus);

    ntStatus = SamrClose(bSamr, hConn);
    if (ntStatus != 0) rpc_fail(ntStatus);

done:
    if (pSecDesc)
    {
        SamrFreeMemory(pSecDesc);
    }

    RTL_FREE(&pwszSidStr);
    RTL_FREE(&pSid);
    RTL_FREE(&pDomainSid);
    RTL_FREE(&pSecDesc);

    return (ntStatus == STATUS_SUCCESS);
}


void SetupSamrTests(struct test *t)
{
    NTSTATUS status = STATUS_SUCCESS;

    status = SamrInitMemory();
    if (status) return;

    AddTest(t, "SAMR-QUERY-USER", TestSamrQueryUser);
    AddTest(t, "SAMR-QUERY-ALIAS", TestSamrQueryAlias);
    AddTest(t, "SAMR-ALIAS", TestSamrAlias);
    AddTest(t, "SAMR-ALIAS-MEMBERS", TestSamrUsersInAliases);
    AddTest(t, "SAMR-QUERY-DOMAIN", TestSamrQueryDomain);
    AddTest(t, "SAMR-ENUM-USERS", TestSamrEnumUsers);
    AddTest(t, "SAMR-OPEN-DOMAIN", TestSamrOpenDomain);
    AddTest(t, "SAMR-ENUM-DOMAINS", TestSamrEnumDomains);
    AddTest(t, "SAMR-CREATE-USER", TestSamrCreateUserAccount);
    AddTest(t, "SAMR-CREATE-ALIAS", TestSamrCreateAlias);
    AddTest(t, "SAMR-CREATE-GROUP", TestSamrCreateGroup);
    AddTest(t, "SAMR-USER-PASSWORD", TestSamrSetUserPassword);
    AddTest(t, "SAMR-MULTIPLE-CONNECTION", TestSamrMultipleConnections);
    AddTest(t, "SAMR-USER-PASSWORD-CHANGE", TestSamrChangeUserPassword);
    AddTest(t, "SAMR-ENUM-ALIASES", TestSamrEnumAliases);
    AddTest(t, "SAMR-GET-USER-GROUPS", TestSamrGetUserGroups);
    AddTest(t, "SAMR-GET-USER-ALIASES", TestSamrGetUserAliases);
    AddTest(t, "SAMR-QUERY-DISPLAY-INFO", TestSamrQueryDisplayInfo);
    AddTest(t, "SAMR-CONNECT", TestSamrConnect);
    AddTest(t, "SAMR-DOMAINS", TestSamrDomains);
    AddTest(t, "SAMR-DOMAINS-QUERY", TestSamrDomainsQuery);
    AddTest(t, "SAMR-USERS", TestSamrUsers);
    AddTest(t, "SAMR-ALIASES", TestSamrAliases);
    AddTest(t, "SAMR-QUERY-SECURITY", TestSamrQuerySecurity);
}


/*
local variables:
mode: c
c-basic-offset: 4
indent-tabs-mode: nil
tab-width: 4
end:
*/
