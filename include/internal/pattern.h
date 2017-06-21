#pragma once

#include "public.h"

KG_NAMESPACE_BEGIN(xzero)

template <class T>
class KG_Singleton : private KG_UnConstructable
{
public:
    static T& GetInstanceRef()
    {
        static T l_sInstance;
        return l_sInstance;
    }

    static T* GetInstancePtr()
    {
        return &GetInstanceRef();
    }
};

KG_NAMESPACE_END

#define KG_SINGLETON_DCL_STRICT(T)       \
    friend class xzero::KG_Singleton<T>; \
    private:                             \
        T() {}                           \
        T(const T &) {}                  \
        T &operator=(const T &) {}       \
    public:                              \
        ~T() {}
#define KG_SINGLETON_DCL(T) friend class xzero::KG_Singleton<T>;
#define KG_SINGLETON_REF(T) xzero::KG_Singleton<T>::GetInstanceRef()
#define KG_SINGLETON_PTR(T) xzero::KG_Singleton<T>::GetInstancePtr()
