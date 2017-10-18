#include "str.h"
#include "debug.h"

#include <string.h>

KG_NAMESPACE_BEGIN(xzero)

bool KG_Strncpy(char *pDstStr, const char *pszSrcStr, int nCount)
{
    bool bResult = false;

    KG_PROCESS_PTR_ERROR(pDstStr);
    KG_PROCESS_PTR_ERROR(pszSrcStr);

    ::strncpy(pDstStr, pszSrcStr, nCount);
    pDstStr[sizeof(pDstStr) - 1] = '\0';

    bResult = true;
Exit0:
    return bResult;
}

KG_NAMESPACE_END
