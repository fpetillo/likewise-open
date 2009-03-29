#include "includes.h"

DWORD
SamDbOpen(
    PHANDLE phDirectory
    )
{
    DWORD dwError = 0;
    PSAM_DIRECTORY_CONTEXT pDirContext = NULL;

    dwError = SamDbBuildDirectoryContext(
                    gSamGlobals.pDbInstanceLock,
                    gSamGlobals.pObjectClassAttrMaps,
                    gSamGlobals.dwNumObjectClassAttrMaps,
                    gSamGlobals.pAttrMaps,
                    gSamGlobals.dwNumMaps,
                    &pDirContext);
    BAIL_ON_SAMDB_ERROR(dwError);

    *phDirectory = (HANDLE)pDirContext;

cleanup:

    return dwError;

error:

    *(phDirectory) = (HANDLE)NULL;

    if (pDirContext)
    {
        SamDbFreeDirectoryContext(pDirContext);
    }

    goto cleanup;
}
