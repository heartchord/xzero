#pragma once

#include "public.h"

#include <stdarg.h>

#undef  KG_DEBUG_DEFAULTBUFFSIZE
#define KG_DEBUG_DEFAULTBUFFSIZE 1024

KG_NAMESPACE_BEGIN(xzero)

void KG_DebugPrint     (const char *pFmt, ...);
void KG_DebugPrintln   (const char *pFmt, ...);
void KG_DebugMessageBox(const char *pMsg, ...);
void KG_AssertFailed   (const char *pCondition, const char *pFileName, int nLineNum);

KG_NAMESPACE_END

#ifdef _DEBUG
    #define KG_ASSERT(x) assert(x)
    //#define KG_ASSERT(x) if (!(x)) xzero::KG_AssertFailed(#x, __FILE__, __LINE__)
#else
    #define KG_ASSERT(x) 0
#endif
