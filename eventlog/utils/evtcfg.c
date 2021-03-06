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

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        evtcfg.c
 *
 * Abstract:
 *
 *        Likewise Security and Authentication Subsystem (EVTSS)
 *
 *        Registry Configuration
 *
 */
#include "includes.h"

struct __EVT_CONFIG_REG
{
    HANDLE hConnection;
    HKEY hKey;
    PSTR pszConfigKey;
    PSTR pszPolicyKey;
};

DWORD
EVTProcessConfig(
    PCSTR pszConfigKey,
    PCSTR pszPolicyKey,
    PEVT_CONFIG_TABLE pConfig,
    DWORD dwConfigEntries
    )
{
    DWORD dwError = 0;
    DWORD dwEntry;

    PEVT_CONFIG_REG pReg = NULL;

    dwError = EVTOpenConfig(pszConfigKey, pszPolicyKey, &pReg);
    BAIL_ON_EVT_ERROR(dwError);

    if ( pReg == NULL )
    {
        goto error;
    }

    for (dwEntry = 0; dwEntry < dwConfigEntries; dwEntry++)
    {
        dwError = 0;
        switch (pConfig[dwEntry].Type)
        {
            case EVTTypeString:
                dwError = EVTReadConfigString(
                            pReg,
                            pConfig[dwEntry].pszName,
                            pConfig[dwEntry].bUsePolicy,
                            pConfig[dwEntry].pValue);
                break;

            case EVTTypeDword:
                dwError = EVTReadConfigDword(
                            pReg,
                            pConfig[dwEntry].pszName,
                            pConfig[dwEntry].bUsePolicy,
                            pConfig[dwEntry].dwMin,
                            pConfig[dwEntry].dwMax,
                            pConfig[dwEntry].pValue);
                break;

            case EVTTypeBoolean:
                dwError = EVTReadConfigBoolean(
                            pReg,
                            pConfig[dwEntry].pszName,
                            pConfig[dwEntry].bUsePolicy,
                            pConfig[dwEntry].pValue);
                break;

            case EVTTypeEnum:
                dwError = EVTReadConfigEnum(
                            pReg,
                            pConfig[dwEntry].pszName,
                            pConfig[dwEntry].bUsePolicy,
                            pConfig[dwEntry].dwMin,
                            pConfig[dwEntry].dwMax,
                            pConfig[dwEntry].ppszEnumNames,
                            pConfig[dwEntry].pValue);
                break;

            default:
                break;
        }
        BAIL_ON_NON_LWREG_ERROR(dwError);
        dwError = 0;
    }

cleanup:
    EVTCloseConfig(pReg);
    pReg = NULL;

    return dwError;

error:
    goto cleanup;
}

DWORD
EVTOpenConfig(
    PCSTR pszConfigKey,
    PCSTR pszPolicyKey,
    PEVT_CONFIG_REG *ppReg
    )
{
    DWORD dwError = 0;

    PEVT_CONFIG_REG pReg = NULL;

    dwError = EVTAllocateMemory(sizeof(EVT_CONFIG_REG), (PVOID*)&pReg);
    BAIL_ON_EVT_ERROR(dwError);

    dwError = EVTAllocateString(pszConfigKey, &(pReg->pszConfigKey));
    BAIL_ON_EVT_ERROR(dwError);

    dwError = EVTAllocateString(pszPolicyKey, &(pReg->pszPolicyKey));
    BAIL_ON_EVT_ERROR(dwError);

    dwError = RegOpenServer(&(pReg->hConnection));
    if ( dwError )
    {
        dwError = 0;
        goto error;
    }

    dwError = RegOpenKeyExA(
            pReg->hConnection,
            NULL,
            HKEY_THIS_MACHINE,
            0,
            KEY_READ,
            &(pReg->hKey));
    if (dwError)
    {
        dwError = 0;
        goto error;
    }

cleanup:

    *ppReg = pReg;

    return dwError;

error:

    EVTCloseConfig(pReg);
    pReg = NULL;

    goto cleanup;
}

VOID
EVTCloseConfig(
    PEVT_CONFIG_REG pReg
    )
{
    if ( pReg )
    {
        EVT_SAFE_FREE_STRING(pReg->pszConfigKey);

        EVT_SAFE_FREE_STRING(pReg->pszPolicyKey);
        if ( pReg->hConnection )
        {
            if ( pReg->hKey )
            {
                RegCloseKey(pReg->hConnection, pReg->hKey);
                pReg->hKey = NULL;
            }
            RegCloseServer(pReg->hConnection);
            pReg->hConnection = NULL;
        }

        EVT_SAFE_FREE_MEMORY(pReg);
    }
}

DWORD
EVTReadConfigString(
    PEVT_CONFIG_REG pReg,
    PCSTR   pszName,
    BOOLEAN bUsePolicy,
    PSTR    *ppszValue
    )
{
    DWORD dwError = 0;

    BOOLEAN bGotValue = FALSE;
    PSTR pszValue = NULL;
    char szValue[MAX_VALUE_LENGTH];
    DWORD dwType;
    DWORD dwSize;

    if ( bUsePolicy )
    {
        dwSize = sizeof(szValue);
        memset(szValue, 0, dwSize);
        dwError = RegGetValueA(
                    pReg->hConnection,
                    pReg->hKey,
                    pReg->pszPolicyKey,
                    pszName,
                    RRF_RT_REG_SZ,
                    &dwType,
                    szValue,
                    &dwSize);
        if (!dwError)
            bGotValue = TRUE;
    }

    if (!bGotValue )
    {
        dwSize = sizeof(szValue);
        memset(szValue, 0, dwSize);
        dwError = RegGetValueA(
                    pReg->hConnection,
                    pReg->hKey,
                    pReg->pszConfigKey,
                    pszName,
                    RRF_RT_REG_SZ,
                    &dwType,
                    szValue,
                    &dwSize);
        if (!dwError)
            bGotValue = TRUE;
    }

    if (bGotValue)
    {
        dwError = EVTAllocateString(szValue, &pszValue);
        BAIL_ON_EVT_ERROR(dwError);

        EVT_SAFE_FREE_STRING(*ppszValue);
        *ppszValue = pszValue;
        pszValue = NULL;
    }

    dwError = 0;

cleanup:
    EVT_SAFE_FREE_STRING(pszValue);

    return dwError;

error:
    goto cleanup;
}

DWORD
EVTReadConfigDword(
    PEVT_CONFIG_REG pReg,
    PCSTR pszName,
    BOOLEAN bUsePolicy,
    DWORD dwMin,
    DWORD dwMax,
    PDWORD pdwValue
    )
{
    DWORD dwError = 0;

    BOOLEAN bGotValue = FALSE;
    DWORD dwValue;
    DWORD dwSize;
    DWORD dwType;

    if (bUsePolicy)
    {
        dwSize = sizeof(dwValue);
        dwError = RegGetValueA(
                    pReg->hConnection,
                    pReg->hKey,
                    pReg->pszPolicyKey,
                    pszName,
                    RRF_RT_REG_DWORD,
                    &dwType,
                    (PBYTE)&dwValue,
                    &dwSize);
        if (!dwError)
        {
            bGotValue = TRUE;
        }
    }

    if (!bGotValue)
    {
        dwSize = sizeof(dwValue);
        dwError = RegGetValueA(
                    pReg->hConnection,
                    pReg->hKey,
                    pReg->pszConfigKey,
                    pszName,
                    RRF_RT_REG_DWORD,
                    &dwType,
                    (PBYTE)&dwValue,
                    &dwSize);
        if (!dwError)
        {
            bGotValue = TRUE;
        }
    }

    if (bGotValue)
    {
        if ( dwMin <= dwValue && dwValue <= dwMax)
            *pdwValue = dwValue;
    }

    dwError = 0;

    return dwError;
}

DWORD
EVTReadConfigBoolean(
    PEVT_CONFIG_REG pReg,
    PCSTR pszName,
    BOOLEAN bUsePolicy,
    PBOOLEAN pbValue
    )
{

    DWORD dwError = 0;

    DWORD dwValue = *pbValue == TRUE ? 0x00000001 : 0x00000000;

    dwError = EVTReadConfigDword(
                pReg,
                pszName,
                bUsePolicy,
                0,
                -1,
                &dwValue);
    BAIL_ON_EVT_ERROR(dwError);

    *pbValue = dwValue ? TRUE : FALSE;

cleanup:

    return dwError;

error:
    goto cleanup;
}

DWORD
EVTReadConfigEnum(
    PEVT_CONFIG_REG pReg,
    PCSTR   pszName,
    BOOLEAN bUsePolicy,
    DWORD   dwMin,
    DWORD   dwMax,
    const PCSTR   *ppszEnumNames,
    PDWORD  pdwValue
    )
{
    DWORD dwError = 0;
    PSTR pszValue = NULL;

    DWORD dwEnumIndex;

    dwError = EVTReadConfigString(
                pReg,
                pszName,
                bUsePolicy,
                &pszValue);
    BAIL_ON_EVT_ERROR(dwError);

    if (pszValue != NULL )
    {
        for (dwEnumIndex = 0;
             dwEnumIndex <= dwMax - dwMin;
             dwEnumIndex++)
        {
            if(!strcasecmp(pszValue, ppszEnumNames[dwEnumIndex]))
            {
                *pdwValue = dwEnumIndex + dwMin;
                goto cleanup;
            }
        }
    }

cleanup:
    EVT_SAFE_FREE_STRING(pszValue);
    return dwError;

error:
    goto cleanup;
}

