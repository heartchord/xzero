#include "hash.h"

KG_NAMESPACE_BEGIN(xzero)

DWORD KG_KSGStringHash(const char * const cszToHash)
{
    DWORD dwResult = static_cast<DWORD>(-1);
    DWORD dwID     = 0;

    KG_PROCESS_PTR_ERROR_Q(cszToHash);

    for (int i = 0; '\0' != cszToHash[i]; i++)
    {
        dwID = (dwID + (i + 1) * cszToHash[i]) % 0x8000000B * 0xFFFFFFEF;
    }

    dwResult = (dwID ^ 0x12345678);
Exit0:
    return dwResult;
}

DWORD KG_KSGFileNameHash(const char * const cszToHash)
{
    DWORD dwResult = static_cast<DWORD>(-1);
    DWORD dwID     = 0;
    char  c        = 0;

    KG_PROCESS_PTR_ERROR_Q(cszToHash);

    for (int i = 0; '\0' != cszToHash[i]; i++)
    {
        c = cszToHash[i];
        if (c >= 'A' && c <= 'Z')
        { // 'A' - 'Z' to 'a' - 'z'
            c += 0x20;
        }
        else if (c == '/')
        {
            c = '\\';
        }

        dwID = (dwID + (i + 1) * c) % 0x8000000b * 0xffffffef;
    }

    dwResult = (dwID ^ 0x12345678);
Exit0:
    return dwResult;
}

KG_NAMESPACE_END
