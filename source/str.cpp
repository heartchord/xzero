#include "str.h"
#include "debug.h"

#include <string.h>

#pragma warning(disable:4996)

KG_NAMESPACE_BEGIN(xzero)

bool KG_Strncpy(char *pBuff, const char *pszStr, size_t uCount)
{
    bool bResult = false;

    KG_PROCESS_PTR_ERROR(pBuff);
    KG_PROCESS_PTR_ERROR(pszStr);

    ::strncpy(pBuff, pszStr, uCount);
    pBuff[sizeof(pBuff) - 1] = '\0';

    bResult = true;
Exit0:
    return bResult;
}

int KG_Snprintf(char *pBuff, size_t uBuffSize, const char *pszFormat, ...)
{
    int     nResult = -1;
    va_list args;

    KG_PROCESS_PTR_ERROR(pBuff);
    KG_PROCESS_ERROR(uBuffSize > 0);

    va_start(args, pszFormat);

#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
    nResult = ::_vsnprintf(pBuff, uBuffSize, pszFormat, args);
#else                                                                   // linux   platform
    nResult = ::vsnprintf (pBuff, uBuffSize, pszFormat, args);
#endif // KG_PLATFORM_WINDOWS

    if(nResult >= (int)uBuffSize || nResult < 0)
    { // windows : formated_str_len == uBuffSize, ret = uBuffSize; formated_str_len > uBuffSize, ret < 0.
      // linux   : vsnprintf returns formated_str_len.
        KG_ASSERT(false && "[Warning] string will be truncated!");
        pBuff[uBuffSize - 1] = '\0';
        nResult = uBuffSize - 1;
    }

    va_end(args);

Exit0:
    return nResult;
}

int KG_Strcasecmp(const char *pszStr1, const char *pszStr2)
{
    int nResult = -1;

    KG_PROCESS_PTR_ERROR(pszStr1);
    KG_PROCESS_PTR_ERROR(pszStr2);

#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
    nResult = ::stricmp(pszStr1, pszStr2);
#else                                                                   // linux   platform
    nResult = ::strcasecmp(pszStr1, pszStr2);
#endif // KG_PLATFORM_WINDOWS

Exit0:
    return nResult;
}

int KG_Strncasecmp(const char *pszStr1, const char *pszStr2, size_t uCount)
{
    int nResult = -1;

    KG_PROCESS_PTR_ERROR(pszStr1);
    KG_PROCESS_PTR_ERROR(pszStr2);

#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
    nResult = ::strnicmp(pszStr1, pszStr2, uCount);
#else                                                                   // linux   platform
    nResult = ::strncasecmp(pszStr1, pszStr2, uCount);
#endif // KG_PLATFORM_WINDOWS

Exit0:
    return nResult;
}

int KG_BinData2HexStr(const char *pData, size_t uCount, char *pBuff, size_t uBuffSize)
{
    int           nResult     = -1;
    size_t        uCurPos     = 0;
    size_t        nCurBuffPos = 0;
    unsigned char c           = 0;

    KG_PROCESS_PTR_ERROR(pData);
    KG_PROCESS_PTR_ERROR(pBuff);
    KG_PROCESS_ERROR(uCount >= 0 && uBuffSize > 0);
    KG_PROCESS_ERROR(uBuffSize >= uCount * 2 + 1);                      // Include '\0'

    for (; uCurPos < uCount; uCurPos++)
    {
        c = ((unsigned char)pData[uCurPos]) >> 4;
        if (c < 10)
        {
            pBuff[nCurBuffPos++] = c + '0';
        }
        else
        {
            pBuff[nCurBuffPos++] = c - 10 + 'A';
        }

        c = ((unsigned char)pData[uCurPos]) & 0x0F;
        if (c < 10)
        {
            pBuff[nCurBuffPos++] = c + '0';
        }
        else
        {
            pBuff[nCurBuffPos++] = c - 10 + 'A';
        }
    }

    pBuff[nCurBuffPos] = '\0';
    nResult = nCurBuffPos;
Exit0:
    return nResult;
}

int KG_HexStr2BinData(const char *pszHexStr, size_t uCount, char *pBuff, size_t uBuffSize)
{
    int            nResult = -1;
    unsigned char  ah      = 0;
    unsigned char  al      = 0;
    unsigned char *pIter   = NULL;

    KG_PROCESS_PTR_ERROR(pszHexStr);
    KG_PROCESS_PTR_ERROR(pBuff);
    KG_PROCESS_ERROR(uCount >= 0);
    KG_PROCESS_ERROR(0 == uCount % 2);
    KG_PROCESS_ERROR(uBuffSize >= uCount / 2);

    pIter = (unsigned char*)pBuff;
    for (size_t i = 0; i < uCount; i += 2)
    {
        // get byte high
        ah = pszHexStr[i];
        if ((ah >= '0') && (ah <= '9'))
        {
            ah = (unsigned char)(ah - '0');
        }
        else
        {
            ah = (unsigned char)(ah - 'A' + 10);
        }

        // get byte low
        al = pszHexStr[i + 1];
        if ((al >= '0') && (al <= '9'))
        {
            al = (unsigned char)(al - '0');
        }
        else
        {
            al = (unsigned char)(al - 'A' + 10);
        }

        // set struct bye
        *pIter++ = (unsigned char)(ah * 16 + al);
    }

    nResult = uCount / 2;
Exit0:
    return nResult;
}

KG_NAMESPACE_END
