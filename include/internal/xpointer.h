#pragma once

#include "public.h"

KG_NAMESPACE_BEGIN(xzero)

template <class T>
inline void KG_DeletePtrSafely(T *&p)
{
    // check if type is complete
    typedef char TypeMustBeComplete[ sizeof(T) ? 1 : -1 ];
    (void)sizeof(TypeMustBeComplete);

    if (NULL != p)
    {
        delete p;
        p = NULL;
    }
}

template <class T>
inline void KG_DeleteConstPtrSafely(T *const p)
{
    // check if type is complete
    typedef char TypeMustBeComplete[ sizeof(T) ? 1 : -1 ];
    (void)sizeof(TypeMustBeComplete);

    if (NULL != p)
    {
        delete p;
    }
}

template <class T>
inline void KG_DeleteArrayPtrSafely(T *&p)
{
    // check if type is complete 
    typedef char TypeMustBeComplete[ sizeof(T) ? 1 : -1 ];
    (void)sizeof(TypeMustBeComplete);

    if (NULL != p)
    {
        delete[] p;
        p = NULL;
    }
}

template <class T>
inline void KG_DeleteConstArrayPtrSafely(T *const p)
{
    // check if type is complete 
    typedef char TypeMustBeComplete[ sizeof(T) ? 1 : -1 ];
    (void)sizeof(TypeMustBeComplete);

    if (NULL != p)
    {
        delete[] p;
    }
}

template <class T>
inline void KG_FreePtrSafely(T *&p)
{
    if (NULL != p)
    {
        ::free(p);
        p = NULL;
    }
}

template <class T>
inline void KG_FreeConstPtrSafely(T *const p)
{
    if (NULL != p)
    {
        ::free(p);
    }
}

template <class T>
inline void KG_FreeGlobalPtrSafely(T *&p)
{
#ifdef KG_PLATFORM_WINDOWS
    if (NULL != p)
    {
        ::GlobalFree(p);
        p = NULL;
    }
#endif
}

template <class T>
inline void KG_FreeGlobalConstPtrSafely(T * const p)
{
#ifdef KG_PLATFORM_WINDOWS
    if (NULL != p)
    {
        ::GlobalFree(p);
    }
#endif
}

template <class T>
inline void KG_ReleaseObjSafely(T * &p)
{
    // check if type is complete 
    typedef char TypeMustBeComplete[ sizeof(T) ? 1 : -1 ];
    (void)sizeof(TypeMustBeComplete);

    if (NULL != p)
    {
        p->Release();
        p = NULL;
    }
}

KG_NAMESPACE_END
