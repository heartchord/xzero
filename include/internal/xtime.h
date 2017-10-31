#pragma once

#include "xpublic.h"

#include <time.h>

KG_NAMESPACE_BEGIN(xzero)

DWORD KG_GetTickCount();
struct tm *KG_LocalTime(const time_t *pTimep, struct tm *pResult);

KG_NAMESPACE_END
