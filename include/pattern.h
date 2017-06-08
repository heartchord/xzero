#pragma once

#include "public.h"

namespace xzero
{
    template <class T>
    class KG_Singleton : private KG_UnConstructable
    {
    public:
        static T& GetInstance()
        {
            static T l_sInstance;
            return l_sInstance;
        }
    };

    #define KG_SINGLETON_DECLARE(T) friend class xzero::KG_Singleton<T>
    #define KG_SINGLETON_REF(T)     (xzero::KG_Singleton<T>::GetInstance())
    #define KG_SINGLETON_PTR(T)     (&xzero::KG_Singleton<T>::GetInstance())
}