#pragma once

#include "public.h"

KG_NAMESPACE_BEGIN(xzero)

#ifdef KG_PLATFORM_WINDOWS                                              // windows platform

inline long KG_InterlockedIncrement(long *plValue)
{
#ifdef _MT                                                              // multi-thread
    return ::InterlockedIncrement(plValue);
#else
    return ++value;
#endif // _MT
}

inline long KG_InterlockedDecrement(long *plValue)
{
#ifdef _MT                                                              // multi-thread
    return ::InterlockedDecrement(plValue);
#else
    return --value;
#endif // _MT
}

inline long KG_InterlockedExchange(volatile long *plTarget, long lValue)
{
#ifdef _MT                                                              // multi-thread
    return ::InterlockedExchange(plTarget, lValue);
#else
    long lTemp = *plTarget;
    *plTarget  = lValue;
    return lTemp;
#endif
}

inline long KG_InterlockedExchangeAdd(long *plValue, long lAddValue)
{
#ifdef _MT                                                              // multi-thread
    return ::InterlockedExchangeAdd(plValue, lAddValue);
#else
    long lResult = *plValue;
    *plValue += lAddValue;
    return lResult;
#endif // _MT
}

#else                                                                   // linux platform

inline long KG_InterlockedIncrement(long *plValue)
{
    long lResult;
    __asm__ __volatile__
    (
        "movl $0x01, %%eax\n"                                           // 0x01 -> eax
        "lock xaddl %%eax, (%1)\n"                                      // exchange plValue and eax, plValue += eax
        "incl %%eax\n"                                                  // eax += 1
        :
            "=a"(lResult)
        :
            "c"(plValue)
        :
            "memory"
    );
    return lResult;
}

inline long KG_InterlockedDecrement(long *plValue)
{
    long lResult;
    __asm__ __volatile__
    (
        "movl $0xffffffff, %%eax\n"
        "lock xaddl %%eax, (%1)\n"
        "decl %%eax\n"
        :
            "=a"(lResult)
        :
            "c"(plValue)
        :
            "memory"
    );
    return lResult;
}

inline long KG_InterlockedExchange(volatile long *pTarget, long lValue)
{
    __asm__ __volatile__
    (
        "xchgl %k0, %1"                                                 // by default, 'xchgl' will lock memory bus when executing£¬%k0 : 32-bits register,
        :
            "=r" (lValue)                                               // %0 : any universal register
        :
            "m" (*((volatile long *)pTarget)), "0" (lValue)
        :
            "memory"
    );
    return lValue;
}

inline long KG_InterlockedExchangeAdd(long *plValue, long lAddValue)
{
    long lResult;
    __asm__ __volatile__
    (
        "lock xaddl %%eax, (%1)\n"
        :
            "=a"(lResult)
        :
            "c"(plValue), "a"(lAddValue)
        :
            "memory"
    );
    return lResult;
}

#endif  // KG_PLATFORM_WINDOWS

KG_NAMESPACE_END
