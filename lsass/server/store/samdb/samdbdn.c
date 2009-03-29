#include "includes.h"

static
DWORD
SamDbGetDnToken(
    PWSTR            pwszObjectNameCursor,
    DWORD            dwAvailableLen,
    PSAMDB_DN_TOKEN* ppDnToken,
    PDWORD           pdwLenUsed
    );

static
PSAMDB_DN_TOKEN
SamDbReverseTokenList(
    PSAMDB_DN_TOKEN pTokenList
    );

DWORD
SamDbParseDN(
    PWSTR       pwszObjectDN,
    PSAM_DB_DN* ppDN
    )
{
    DWORD dwError = 0;
    PWSTR pwszObjectNameCursor = NULL;
    DWORD dwAvailableLen = 0;
    PSAM_DB_DN pDN = NULL;

    if (!pwszObjectDN || !*pwszObjectDN)
    {
        dwError = LSA_ERROR_INVALID_PARAMETER;
        BAIL_ON_SAMDB_ERROR(dwError);
    }

    dwError = DirectoryAllocateMemory(
                    sizeof(SAM_DB_DN),
                    (PVOID*)&pDN);
    BAIL_ON_SAMDB_ERROR(dwError);

    dwError = DirectoryAllocateStringW(
                    pwszObjectDN,
                    &pDN->pwszDN);
    BAIL_ON_SAMDB_ERROR(dwError);

    dwAvailableLen = wc16slen(pwszObjectDN);
    pwszObjectNameCursor = pDN->pwszDN;

    do
    {
        PSAMDB_DN_TOKEN pToken = NULL;
        DWORD dwLenUsed;

        dwError = SamDbGetDnToken(
                        pwszObjectNameCursor,
                        dwAvailableLen,
                        &pToken,
                        &dwLenUsed);
        BAIL_ON_SAMDB_ERROR(dwError);

        pToken->pNext = pDN->pTokenList;
        pDN->pTokenList = pToken;

        pwszObjectNameCursor += dwLenUsed;
        dwAvailableLen -= dwLenUsed;

    } while (dwAvailableLen);

    pDN->pTokenList = SamDbReverseTokenList(pDN->pTokenList);

    *ppDN = pDN;

cleanup:

    return dwError;

error:

    *ppDN = NULL;

    if (pDN)
    {
        SamDbFreeDN(pDN);
    }

    goto cleanup;
}

DWORD
SamDbGetDNComponents(
    PSAM_DB_DN pDN,
    PWSTR*     ppwszObjectName,
    PWSTR*     ppwszDomainName,
    PWSTR*     ppwszParentDN
    )
{
    DWORD dwError = 0;
    wchar16_t wszDot[] = { '.', 0};
    PSAMDB_DN_TOKEN pToken = pDN->pTokenList;
    PSAMDB_DN_TOKEN pDCToken = NULL;
    PSAMDB_DN_TOKEN pParentDNToken = NULL;
    DWORD dwObjectNameLen = 0;
    DWORD dwDomainNameLen = 0;
    PWSTR pwszObjectName = NULL;
    PWSTR pwszParentDN = NULL;
    PWSTR pwszDomainName = NULL;

    if (!pDN || !pDN->pTokenList)
    {
        dwError = LSA_ERROR_INVALID_PARAMETER;
        BAIL_ON_SAMDB_ERROR(dwError);
    }

    if (pToken->tokenType != SAMDB_DN_TOKEN_TYPE_DC)
    {
        dwObjectNameLen = pToken->dwLen * sizeof(wchar16_t);

        pToken = pToken->pNext;

        pParentDNToken = pToken;
    }

    while (pToken && (pToken->tokenType != SAMDB_DN_TOKEN_TYPE_DC))
    {
        pToken = pToken->pNext;
    }
    pDCToken = pToken;

    while (pToken)
    {
        if (dwDomainNameLen)
        {
            dwDomainNameLen += sizeof(wszDot[0]);
        }
        dwDomainNameLen += pToken->dwLen * sizeof(wchar16_t);

        if (pToken->tokenType != SAMDB_DN_TOKEN_TYPE_DC)
        {
            dwError = LSA_ERROR_INVALID_LDAP_DN;
            BAIL_ON_SAMDB_ERROR(dwError);
        }
    }

    if (dwObjectNameLen)
    {
        dwError = DirectoryAllocateMemory(
                        dwObjectNameLen + sizeof(wchar16_t),
                        (PVOID*)&pwszObjectName);
        BAIL_ON_SAMDB_ERROR(dwError);

        pToken = pDN->pTokenList;

        memcpy( (PBYTE)pwszObjectName,
                (PBYTE)pToken->pwszToken,
                pToken->dwLen * sizeof(wchar16_t));
    }

    if (pParentDNToken)
    {
        dwError = DirectoryAllocateStringW(
                    pParentDNToken->pwszToken,
                    &pwszParentDN);
        BAIL_ON_SAMDB_ERROR(dwError);
    }

    if (dwDomainNameLen)
    {
        PWSTR pwszCursor = NULL;

        dwError = DirectoryAllocateMemory(
                        dwDomainNameLen + sizeof(wchar16_t),
                        (PVOID*)&pwszDomainName);
        BAIL_ON_SAMDB_ERROR(dwError);

        pwszCursor = pwszDomainName;
        for (pToken = pDCToken;pToken; pToken = pToken->pNext)
        {
            if (pToken != pDCToken)
            {
                *pwszCursor = wszDot[0];

                pwszCursor++;
            }

            memcpy( (PBYTE)pwszCursor,
                    (PBYTE)pToken->pwszToken,
                    pToken->dwLen * sizeof(wchar16_t));

            pwszCursor += pToken->dwLen;
        }
    }

    *ppwszObjectName = pwszObjectName;
    *ppwszParentDN   = pwszParentDN;
    *ppwszDomainName = pwszDomainName;

cleanup:

    return dwError;

error:

    *ppwszObjectName = NULL;
    *ppwszParentDN   = NULL;
    *ppwszDomainName = NULL;

    DIRECTORY_FREE_MEMORY(pwszObjectName);
    DIRECTORY_FREE_MEMORY(pwszParentDN);
    DIRECTORY_FREE_MEMORY(pwszDomainName);

    goto cleanup;
}

VOID
SamDbFreeDN(
    PSAM_DB_DN pDN
    )
{
    while (pDN->pTokenList)
    {
        PSAMDB_DN_TOKEN pToken = pDN->pTokenList;

        pDN->pTokenList = pDN->pTokenList->pNext;

        DirectoryFreeMemory(pToken);
    }

    DIRECTORY_FREE_MEMORY(pDN->pwszDN);

    DirectoryFreeMemory(pDN);
}

static
DWORD
SamDbGetDnToken(
    PWSTR            pwszObjectNameCursor,
    DWORD            dwAvailableLen,
    PSAMDB_DN_TOKEN* ppDnToken,
    PDWORD           pdwLenUsed
    )
{
    DWORD  dwError = 0;
    wchar16_t wszCNPrefix[] = {'C', 'N', 0};
    DWORD dwLenCNPrefix = sizeof(wszCNPrefix) - sizeof(wchar16_t);
    wchar16_t wszDCPrefix[] = {'D', 'C', 0};
    DWORD dwLenDCPrefix = sizeof(wszDCPrefix) - sizeof(wchar16_t);
    wchar16_t wszOUPrefix[] = {'O', 'U', 0};
    DWORD dwLenOUPrefix = sizeof(wszOUPrefix) - sizeof(wchar16_t);
    wchar16_t wszComma[] = {',', 0};
    DWORD dwLenUsed = 0;
    PSAMDB_DN_TOKEN pToken = NULL;

    dwError = DirectoryAllocateMemory(
                sizeof(SAMDB_DN_TOKEN),
                (PVOID*)&pToken);
    BAIL_ON_SAMDB_ERROR(dwError);

    if ((dwAvailableLen > dwLenCNPrefix) &&
        !memcmp(pwszObjectNameCursor, wszCNPrefix, dwLenCNPrefix))
    {
        pToken->tokenType = SAMDB_DN_TOKEN_TYPE_CN;

        pwszObjectNameCursor += dwLenCNPrefix;
        dwAvailableLen -= dwLenCNPrefix;
        dwLenUsed += dwLenCNPrefix;
    }
    else
    if ((dwAvailableLen > dwLenOUPrefix) &&
        !memcmp(pwszObjectNameCursor, wszOUPrefix, dwLenOUPrefix))
    {
        pToken->tokenType = SAMDB_DN_TOKEN_TYPE_OU;

        pwszObjectNameCursor += dwLenOUPrefix;
        dwAvailableLen -= dwLenOUPrefix;
        dwLenUsed += dwLenOUPrefix;
    }
    else
    if ((dwAvailableLen > dwLenDCPrefix) &&
        !memcmp(pwszObjectNameCursor, wszDCPrefix, dwLenDCPrefix))
    {
        pToken->tokenType = SAMDB_DN_TOKEN_TYPE_DC;

        pwszObjectNameCursor += dwLenDCPrefix;
        dwAvailableLen -= dwLenDCPrefix;
        dwLenUsed += dwLenDCPrefix;
    }
    else
    {
        dwError = LSA_ERROR_INVALID_LDAP_DN;
        BAIL_ON_SAMDB_ERROR(dwError);
    }

    if (!dwAvailableLen)
    {
        dwError = LSA_ERROR_INVALID_LDAP_DN;
        BAIL_ON_SAMDB_ERROR(dwError);
    }

    pToken->pwszToken = pwszObjectNameCursor;
    while (dwAvailableLen && (*pwszObjectNameCursor++ != wszComma[0]))
    {
        dwAvailableLen--;
        pToken->dwLen++;
        dwLenUsed++;
    }

    if (dwAvailableLen && (*pwszObjectNameCursor) == wszComma[0])
    {
        dwLenUsed++;
    }

    *ppDnToken = pToken;
    *pdwLenUsed = dwLenUsed;

cleanup:

    return dwError;

error:

    *ppDnToken = NULL;
    *pdwLenUsed = 0;

    DIRECTORY_FREE_MEMORY(pToken);

    goto cleanup;
}

static
PSAMDB_DN_TOKEN
SamDbReverseTokenList(
    PSAMDB_DN_TOKEN pTokenList
    )
{
    PSAMDB_DN_TOKEN pP = NULL;
    PSAMDB_DN_TOKEN pQ = pTokenList;
    PSAMDB_DN_TOKEN pR = NULL;

    while( pQ ) {
        pR = pQ->pNext;
        pQ->pNext = pP;
        pP = pQ;
        pQ = pR;
    }

    return pP;
}

