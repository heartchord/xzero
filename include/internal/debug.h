#pragma once

#include "public.h"

#ifdef _DEBUG                                                           // debug mode
#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif // _DEBUG
#endif // KG_PLATFORM_WINDOWS

#include <stdarg.h>
#include <stdlib.h>

#undef  KG_DEBUG_DEFAULTBUFFSIZE
#define KG_DEBUG_DEFAULTBUFFSIZE 1024

KG_NAMESPACE_BEGIN(xzero)

void KG_DebugPrint     (const char *pFmt, ...);
void KG_DebugPrintln   (const char *pFmt, ...);
void KG_DebugMessageBox(const char *pMsg, ...);
void KG_PrintlnInHex   (void *pvMsg, const size_t uBytes);
void KG_AssertFailed   (const char *pCondition, const char *pFileName, int nLineNum);

class KG_CodeSegmentMemLeakDetect : private KG_UnCopyable
{
public:
    KG_CodeSegmentMemLeakDetect() {}
    ~KG_CodeSegmentMemLeakDetect() {}

public:
    void Begin();
    void End();

#ifdef _DEBUG                                                           // debug mode
#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
private:
    _CrtMemState m_s1;
    _CrtMemState m_s2;
    _CrtMemState m_s3;
#endif // _DEBUG
#endif // KG_PLATFORM_WINDOWS
};

inline void KG_CodeSegmentMemLeakDetect::Begin()
{
#ifdef _DEBUG                                                           // debug mode
#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
    ::_CrtMemCheckpoint(&m_s1);
#endif // _DEBUG
#endif // KG_PLATFORM_WINDOWS
}

inline void KG_CodeSegmentMemLeakDetect::End()
{
#ifdef _DEBUG                                                           // debug mode
#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
    ::_CrtMemCheckpoint(&m_s2);
    if (::_CrtMemDifference(&m_s3, &m_s1, &m_s2))
    {
        _CrtMemDumpStatistics(&m_s3);
    }
#endif // _DEBUG
#endif // KG_PLATFORM_WINDOWS
}

class KG_GlobalMemLeakDetect : private KG_UnCopyable
{
public:
    KG_GlobalMemLeakDetect();
    ~KG_GlobalMemLeakDetect() {}
};

inline KG_GlobalMemLeakDetect::KG_GlobalMemLeakDetect()
{
#ifdef _DEBUG                                                           // debug mode
#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
    int nTmpFlag = ::_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    nTmpFlag |= _CRTDBG_LEAK_CHECK_DF;
    ::_CrtSetDbgFlag(nTmpFlag);
#endif // _DEBUG
#endif // KG_PLATFORM_WINDOWS
}

KG_NAMESPACE_END

#ifdef _DEBUG
    #define KG_ASSERT(x) assert(x)
    //#define KG_ASSERT(x) if (!(x)) xzero::KG_AssertFailed(#x, __FILE__, __LINE__)
#else
    #define KG_ASSERT(x) 0
#endif
