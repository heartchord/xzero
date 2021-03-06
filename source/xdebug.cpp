#include "xdebug.h"
# pragma warning(disable: 4996)


KG_NAMESPACE_BEGIN(xzero)

void KG_DebugPrint(const char *pszFmt, ...)
{
#ifdef _DEBUG
    char buff[KG_DEBUG_DEFAULTBUFFSIZE];

    va_list va;
    va_start(va, pszFmt);
    ::vsprintf(buff, pszFmt, va);
    va_end(va);

#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
    ::OutputDebugString(buff);
#endif // KG_PLATFORM_WINDOWS

    ::printf("%s", buff);
#endif
}

void KG_DebugPrintln(const char *pszFmt, ...)
{
#ifdef _DEBUG
    char buff[KG_DEBUG_DEFAULTBUFFSIZE];

    va_list va;
    va_start(va, pszFmt);
    ::vsprintf(buff, pszFmt, va);
    va_end(va);

    ::strcat(buff, "\n");

#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
    ::OutputDebugString(buff);
#endif // KG_PLATFORM_WINDOWS

    ::printf("%s", buff);
#endif
}

void KG_DebugMessageBox(const char *pszFmt, ...)
{
    char szMsg[KG_DEBUG_DEFAULTBUFFSIZE];

    va_list va;
    va_start(va, pszFmt);
    ::vsprintf(szMsg, pszFmt, va);
    va_end(va);

    KG_DebugPrintln(szMsg);

#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
    GUITHREADINFO info;
    ::memset(&info, 0, sizeof(info));
    info.cbSize = sizeof(info);

    HWND hWnd = NULL;
    if (::GetGUIThreadInfo(NULL, &info))
    {
        hWnd = info.hwndActive;
    }
    ::MessageBox(hWnd, szMsg, 0, MB_OK | MB_ICONINFORMATION);
#endif // KG_PLATFORM_WINDOWS
}

void KG_PrintlnInHex(void *pvMsg, size_t uBytes)
{
    bool           bResult = false;
    unsigned char *p       = NULL;

    KG_PROCESS_PTR_ERROR(pvMsg);
    KG_PROCESS_ERROR(uBytes >= 0);

    p = (unsigned char *)pvMsg;

    for (size_t i = 0; i < uBytes; i++)
    {
        ::printf("%02X", p[i]);
    }

    ::printf("\n");

    bResult = true;
Exit0:
    return;
}

void KG_AssertFailed(const char *pszCondition, const char *pszFileName, int nLineNum)
{
    KG_DebugMessageBox("[ASSERT FAILED] : Condition - %s, File - %s, Line - %i", pszCondition, pszFileName, nLineNum);
    assert(0);
}

KG_NAMESPACE_END
