#pragma once

#include "xpublic.h"

KG_NAMESPACE_BEGIN(xzero)

bool KG_IsDirExisted (const char *pszPath);
bool KG_IsFileExisted(const char *pszPath);
bool KG_IsPathSeparator(int c);

bool KG_CreatePath(const char *pszPath);

bool KG_DeleteDir (const char *pszPath);
bool KG_DeleteFile(const char *pszPath);

KG_NAMESPACE_END
