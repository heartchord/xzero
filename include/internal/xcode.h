#pragma once

#include "xpublic.h"

KG_NAMESPACE_BEGIN(xzero)

class KG_KSGCode : private KG_UnConstructable
{
public:
	static bool Encode(BYTE *pbyBuff, DWORD uSize, DWORD *puKey);
	static bool Decode(BYTE *pbyBuff, DWORD uSize, DWORD *puKey);
};

KG_NAMESPACE_END
