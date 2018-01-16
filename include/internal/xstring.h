#pragma once

#include "xpublic.h"

KG_NAMESPACE_BEGIN(xzero)

extern const char *g_pszBoolStrPairs[6];
extern const char *g_pszTrueStrList[3];
extern const char *g_pszFalseStrList[3];

bool KG_Strncpy       (char *pBuff, const char *pszStr, size_t uCount);
int  KG_Snprintf      (char *pBuff, size_t uBuffSize, const char *pszFormat, ...);
int  KG_Strcasecmp    (const char *pszStr1, const char *pszStr2);
int  KG_Strncasecmp   (const char *pszStr1, const char *pszStr2, size_t uCount);
int  KG_BinData2HexStr(const char *pData, size_t uCount, char *pBuff, size_t uBuffSize);
int  KG_HexStr2BinData(const char *pszHexStr, size_t uCount, char *pBuff, size_t uBuffSize);

KG_NAMESPACE_END
