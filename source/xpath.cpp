#include "path.h"
#include "debug.h"

KG_NAMESPACE_BEGIN(xzero)

bool KG_IsDirExisted(const char * pszPath)
{
    bool        bResult     = false;
    int         nRetCode    = 0;
#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
    DWORD       dwFileAttrs = 0;
#else                                                                   // linux   platform
    struct stat buf;
#endif // KG_PLATFORM_WINDOWS

    KG_PROCESS_C_STR_ERROR(pszPath);

    nRetCode = ::_access(pszPath, 00);
    KG_PROCESS_ERROR_Q(0 == nRetCode);

#ifdef KG_PLATFORM_WINDOWS                                              // windows platform

    dwFileAttrs = ::GetFileAttributes(pszPath);
    KG_PROCESS_ERROR_Q(INVALID_FILE_ATTRIBUTES != dwFileAttrs);
    KG_PROCESS_ERROR_Q(dwFileAttrs & FILE_ATTRIBUTE_DIRECTORY);

#else                                                                   // linux   platform

    nRetCode = ::stat(pszPath, &buf);
    KG_PROCESS_ERROR_Q(0 == nRetCode);

    nRetCode = S_ISDIR(buf.st_mode);
    KG_PROCESS_ERROR_Q(nRetCode);

#endif // KG_PLATFORM_WINDOWS

    bResult = true;
Exit0:
    return bResult;
}

bool KG_IsFileExisted(const char * pszPath)
{
    bool        bResult     = false;
    int         nRetCode    = 0;
#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
    DWORD       dwFileAttrs = 0;
#else                                                                   // linux   platform
    struct stat buf;
#endif // KG_PLATFORM_WINDOWS

    KG_PROCESS_C_STR_ERROR(pszPath);

    nRetCode = ::_access(pszPath, 00);
    KG_PROCESS_ERROR_Q(0 == nRetCode);

#ifdef KG_PLATFORM_WINDOWS                                              // windows platform

    dwFileAttrs = ::GetFileAttributes(pszPath);
    KG_PROCESS_ERROR_Q(INVALID_FILE_ATTRIBUTES != dwFileAttrs);
    KG_PROCESS_ERROR_Q((dwFileAttrs & FILE_ATTRIBUTE_ARCHIVE) | (dwFileAttrs & FILE_ATTRIBUTE_NORMAL));

#else                                                                   // linux   platform

    nRetCode = ::stat(pszPath, &buf);
    KG_PROCESS_ERROR_Q(0 == nRetCode);

    nRetCode = S_ISREG(buf.st_mode);
    KG_PROCESS_ERROR_Q(nRetCode);

#endif // KG_PLATFORM_WINDOWS

    bResult = true;
Exit0:
    return bResult;
}

bool KG_IsPathSeparator(int c)
{
    return '/' == c || '\\' == c;
}

bool l_CreatePath(const char *pszDirPath)
{
    bool bResult  = false;
    int  nRetCode = 0;

    KG_PROCESS_PTR_ERROR(pszDirPath);
    KG_PROCESS_SUCCESS('\0' == pszDirPath[0]);                          // empty directory

    nRetCode = ::_access(pszDirPath, 00);
    KG_PROCESS_SUCCESS(0 == nRetCode);                                  // directory exists

#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
    nRetCode = ::CreateDirectory(pszDirPath, NULL);
    KG_PROCESS_ERROR(nRetCode);
#else                                                                   // linux   platform
    nRetCode = ::mkdir(pszDirPath, 0777);
    KG_PROCESS_ERROR(0 == nRetCode);
#endif  // KG_PLATFORM_WINDOWS

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

bool KG_CreatePath(const char * pszPath)
{
    bool  bResult                 = false;
    int   nRetCode                = 0;
    char  pPath[KG_MAX_FILE_PATH] = {'\0'};
    char *pIter                   = pPath;
    char  c                       = 0;

    KG_PROCESS_C_STR_ERROR(pszPath);
    ::strcpy(pPath, pszPath);

    while (*pIter)
    {
        c = *pIter;                                                     // current character
        if (KG_IsPathSeparator(c) && *(pIter + 1))
        {
            *pIter   = '\0';
            nRetCode = l_CreatePath(pPath);
            *pIter   = c;
            KG_PROCESS_ERROR(nRetCode);
        }
        pIter++;
    }

    nRetCode = l_CreatePath(pPath);                                     // create last level directory
    KG_PROCESS_ERROR(nRetCode);

    bResult = true;
Exit0:
    return bResult;
}

bool KG_DeleteFile(const char *pszPath)
{
    bool bResult  = false;
    int  nRetCode = 0;

    KG_PROCESS_C_STR_ERROR(pszPath);

    nRetCode = ::remove(pszPath);
    KG_PROCESS_ERROR(nRetCode);

    bResult = true;
Exit0:
    return bResult;
}

KG_NAMESPACE_END
