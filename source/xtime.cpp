#include "xtime.h"

#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
#include <mmsystem.h>
#else                                                                   // linux platform
#endif // KG_PLATFORM_WINDOWS

KG_NAMESPACE_BEGIN(xzero)

/*------------------------------------------------------------------------------------------------------*/
/* [windows platform]                                                                                   */
/*     1. ::GetTickCount returns elapsed milliseconds from the start of the os to now. It probably      */
/* represents 49.71 days and will be zero in more than 49.71 days. The time is not sent in real time,   */
/* but sent by the system in every 18ms. Therefore, the minimum accuracy is 18ms. Its performance is    */
/* good and accuracy is bad(>= 18ms).                                                                   */
/*     2. ::tmGetTime returns elapsed milliseconds from the start of the os to now. It probably         */
/* represents 49.71 days and will be zero in more than 49.71 days. Its accuracy depends on local os     */
/* environment. Its performance is good and accuracy is medium(1 - 18ms).                               */
/*     3. ::QueryPerformanceCounter, ::QueryPerformanceFrequency provides high precision timer. Its     */
/* performance is bad and accuracy is good(100ns).                                                      */
/* [linux platform]                                                                                     */
/*     1. ::time returns elapsed seconds from [UTC 1970-1-1 0:0:0] to now. Its accuracy is in seconds.  */
/*     2. ::gettimeofday returns elapsed seconds from [UTC 1970-1-1 0:0:0] to now. Its accuracy is in   */
/* microseconds.                                                                                        */
/*     3. ::clock_gettime returns elapsed seconds specified by its first parameter. Its accuracy is in  */
/* nanoseconds.                                                                                         */
/*         (1) CLOCK_REALTIME :           from [UTC 1970-1-1 0:0:0]   to [now].                         */
/*         (2) LOCK_MONOTONIC :           from [os startup time]      to [now].                         */
/*         (3) CLOCK_PROCESS_CPUTIME_ID : from [process startup time] to [now].                         */
/*         (4) CLOCK_THREAD_CPUTIME_ID :  from [thread startup time] to [now].                          */
/*------------------------------------------------------------------------------------------------------*/

#ifdef KG_PLATFORM_WINDOWS                                              // windows platform

#pragma comment(lib,"winmm.lib")

DWORD KG_GetTickCount()
{
    static DWORD l_uStartTime = 0;
    DWORD        uCurTime     = 0;
    DWORD        uElapsedTime = 0;

    uCurTime = ::timeGetTime();

    if (0 == l_uStartTime)
    {
        l_uStartTime = uCurTime;
    }

    uElapsedTime = uCurTime - l_uStartTime;
    if (uElapsedTime <= 0)
    {
        uElapsedTime = 1;
    }

    return uElapsedTime;
}

struct tm *KG_LocalTime(const time_t *pTimep, struct tm *pResult)
{
    struct tm *pTm = ::localtime(pTimep);
    if (NULL != pResult && NULL != pTm)
    {
        *pResult = *pTm;
    }

    return pTm;
}

#else                                                                   // linux platform

DWORD KG_GetTickCount()
{
    int             nRetCode     = 0;
    static uint64_t l_uStartTime = 0;
    uint64_t        uCurTime     = 0;
    DWORD           uElapsedTime = 0;
    timespec        t;

    nRetCode = ::clock_gettime(CLOCK_MONOTONIC, &t);
    if (0 != nRetCode)
    {
        KG_ASSERT(false);
        return 1;
    }

    uCurTime = ((uint64_t)t.tv_sec) * 1000 + t.tv_nsec / (1000 * 1000);

    if (l_uStartTime == 0)
    {
        l_uStartTime = uCurTime;
    }

    uElapsedTime = (DWORD)(uCurTime - l_uStartTime);
    if (uElapsedTime <= 0)
    {
        uElapsedTime = 1;
    }

    return uElapsedTime;
}

struct tm *KG_LocalTime(const time_t *pTimep, struct tm *pResult)
{
    return localtime_r(pTimep, pResult);
}

#endif // KG_PLATFORM_WINDOWS

KG_NAMESPACE_END
