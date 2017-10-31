#pragma once

#include "xpublic.h"

#include <vector>
#include <memory>

KG_NAMESPACE_BEGIN(xzero)

// TArg = TArg, TRet = TRet
template <class TArg, class TRet>
class IKG_Function : private KG_UnCopyable
{
public:
    IKG_Function() {}
    virtual ~IKG_Function()
    {
        KG_DebugPrintln("IKG_Function::~IKG_Function() is invoked.\n");
    }

public:
    TRet operator()(TArg arg)
    {
        return _Invoke(arg);
    };

private:
    virtual TRet _Invoke(TArg arg) = 0;
};

// Targ = void, TRet = TRet
template <class TRet>
class IKG_Function<void, TRet> : private KG_UnCopyable
{
public:
    IKG_Function() {}
    virtual ~IKG_Function()
    {
        KG_DebugPrintln("IKG_Function::~IKG_Function() is invoked.\n");
    }

public:
    TRet operator()(void)
    {
        return _Invoke();
    };

private:
    virtual TRet _Invoke(void) = 0;
};

// TObj = TObj, Targ = Targ, TRet = TRet
template <class TObj, class TArg, class TRet>
class KG_Function : public IKG_Function<TArg, TRet>
{
    typedef TRet (TObj::*TFunc)(TArg arg);

private: // forbid default constructor
    KG_Function();
public:
    KG_Function(TObj *pObj, TFunc func) : m_pObj(pObj), m_Func(func)
    {
        KG_ASSERT(NULL != m_pObj);
        KG_ASSERT(NULL != m_Func);
    }

private:
    virtual TRet _Invoke(TArg arg)
    {
        return (m_pObj->*m_Func)(arg);
    }

private:
    TObj *m_pObj;
    TFunc m_Func;
};

// TObj = TObj, Targ = void, TRet = TRet
template <class TObj, class TRet>
class KG_Function<TObj, void, TRet> : public IKG_Function<void, TRet>
{
    typedef TRet (TObj::*TFunc)(void);

private: // forbid default constructor
    KG_Function();
public:
    KG_Function(TObj *pObj, TFunc func) : m_pObj(pObj), m_Func(func)
    {
        KG_ASSERT(NULL != m_pObj);
        KG_ASSERT(NULL != m_Func);
    }

private:
    virtual TRet _Invoke(void)
    {
        return (m_pObj->*m_Func)();
    }

private:
    TObj *m_pObj;
    TFunc m_Func;
};

// TObj = void, Targ = TArg, TRet = TRet
template <class TArg, class TRet>
class KG_Function<void, TArg, TRet> : public IKG_Function<TArg, TRet>
{
    typedef TRet (*TFunc)(TArg arg);

private: // forbid default constructor
    KG_Function();
public:
    KG_Function(TFunc func) : m_Func(func)
    {
        KG_ASSERT(NULL != m_Func);
    }

private:
    TRet _Invoke(TArg arg)
    {
        return (*m_Func)(arg);
    }

private:
    TFunc m_Func;
};

// TObj = void, Targ = void, TRet = TRet
template <class TRet>
class KG_Function<void, void, TRet> : public IKG_Function<void, TRet>
{
    typedef TRet (*TFunc)(void);

private: // forbid default constructor
    KG_Function();
public:
    KG_Function(TFunc func) : m_Func(func)
    {
        KG_ASSERT(NULL != m_Func);
    }

private:
    TRet _Invoke(void)
    {
        return (*m_Func)();
    }

private:
    TFunc m_Func;
};

template <class TArg, class TRet>
class KG_DelegateBase : private KG_UnCopyable
{
    typedef IKG_Function<TArg, TRet> TFunc;
    typedef TFunc *                  PFunc;
    typedef std::shared_ptr<TFunc>   SPFunc;

protected:
    std::vector<SPFunc> m_DelegateSet;

public:
    KG_DelegateBase();
    virtual ~KG_DelegateBase();

public:
    bool Add(PFunc f);
    bool Remove(PFunc f);
    void Clear();

private:
    int Find(PFunc f);
};

template <class TArg, class TRet>
inline KG_DelegateBase<TArg, TRet>::KG_DelegateBase()
{
}

template <class TArg, class TRet>
inline KG_DelegateBase<TArg, TRet>::~KG_DelegateBase()
{
    Clear();
}

template <class TArg, class TRet>
inline bool KG_DelegateBase<TArg, TRet>::Add(PFunc f)
{
    int nResult  = false;
    int nRetCode = -1;

    KG_PROCESS_PTR_ERROR(f);

    nRetCode = Find(f);
    KG_PROCESS_ERROR(nRetCode < 0);

    m_DelegateSet.push_back(SPFunc(f));

    nResult = true;
Exit0:
    return nResult;
}

template <class TArg, class TRet>
inline bool KG_DelegateBase<TArg, TRet>::Remove(PFunc f)
{
    int nResult  = false;
    int nRetCode = -1;

    KG_PROCESS_PTR_ERROR(f);

    nRetCode = Find(f);
    KG_PROCESS_ERROR(nRetCode >= 0);

    m_DelegateSet.erase(nRetCode);

    nResult = true;
Exit0:
    return nResult;
}

template <class TArg, class TRet>
inline void KG_DelegateBase<TArg, TRet>::Clear()
{
    m_DelegateSet.clear();
}

template <class TArg, class TRet>
inline int KG_DelegateBase<TArg, TRet>::Find(PFunc f)
{
    int nResult = -1;

    for (int i = 0; i < m_DelegateSet.size(); i++)
    {
        if (m_DelegateSet[i].get() != f)
        {
            continue;
        }

        nResult = i;
        break;
    }

    return nResult;
}

template <class TArg, class TRet>
class KG_Delegate : public KG_DelegateBase<TArg, TRet>
{
public:
    void operator()(TArg arg)
    {
        return _Invoke(arg);
    };

private:
    void _Invoke(TArg arg)
    {
        for (int i = 0; i < m_DelegateSet.size(); i++)
        {
            if (m_DelegateSet[i])
            {
                (*m_DelegateSet[i])(arg);
            }
        }
    }
};

template <class TRet>
class KG_Delegate<void, TRet> : public KG_DelegateBase<void, TRet>
{
public:
    void operator()(void)
    {
        return _Invoke();
    };

private:
    void _Invoke(void)
    {
        for (unsigned i = 0; i < m_DelegateSet.size(); i++)
        {
            if (m_DelegateSet[i])
            {
                (*m_DelegateSet[i])();
            }
        }
    }
};

template <class TArg, class TRet>
inline KG_Delegate<TArg, TRet> &operator+=(KG_Delegate<TArg, TRet> &lhs, IKG_Function<TArg, TRet> *rhs)
{
    lhs.Add(rhs);
    return lhs;
}

template <class TArg, class TRet>
inline KG_Delegate<TArg, TRet> &operator-=(KG_Delegate<TArg, TRet> &lhs, IKG_Function<TArg, TRet> *rhs)
{
    lhs.Del(rhs);
    return lhs;
}

KG_NAMESPACE_END
