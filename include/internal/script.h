#pragma once

#include "public.h"

#include "../../../x3rdlibrary/include/lua-5.1.5/lua.h"
#include "../../../x3rdlibrary/include/lua-5.1.5/lualib.h"
#include "../../../x3rdlibrary/include/lua-5.1.5/lauxlib.h"

#include <map>
#include <vector>

#undef  KG_MAX_SCIRPT_NAME_LEN
#define KG_MAX_SCIRPT_NAME_LEN  256

#undef  KG_LUA_SCRIPT_ID
#define KG_LUA_SCRIPT_ID        "ScriptId"

#undef  KG_LUA_SCRIPT_THIS
#define KG_LUA_SCRIPT_THIS      "KG_LuaScriptThis"

KG_NAMESPACE_BEGIN(xzero)

template <class T>
inline void KG_CloseLuaStateSafely(T *&p)
{
    // check if type is complete 
    typedef char TypeMustBeComplete[ sizeof(T) ? 1 : -1 ];
    (void)sizeof(TypeMustBeComplete);

    if (NULL != p)
    {
        lua_close(p);
        p = NULL;
    }
}

template <class T>
inline void KG_UnRefLuaStateSafely(T *&p)
{
    // check if type is complete 
    typedef char TypeMustBeComplete[ sizeof(T) ? 1 : -1 ];
    (void)sizeof(TypeMustBeComplete);

    if (NULL != p)
    {
        lua_close(p);
        p = NULL;
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*                                                                                                                                [Script D]                          [Script D]                */
/*                                                            [Script B]                          [Script B]             ---- includeData(KG_LuaIncludeData) -> scriptData(KG_LuaScriptData)    */
/*    [Script A]              [Script A]            ---- includeData(KG_LuaIncludeData) -> scriptData(KG_LuaScriptData) -|--- ...                                                               */
/*                                                  |                                                                    ---- includeData(KG_LuaIncludeData) -> scriptData(KG_LuaScriptData)    */
/*dwScriptId(DWORD) -> scriptData(KG_LuaScriptData)-|--- ...                                                                      [Script E]                          [Script E]                */
/*                                                  |                                                                                                                                           */
/*                                                  ---- includeData(KG_LuaIncludeData) -> scriptData(KG_LuaScriptData)                                                                         */
/*                                                            [Script C]                          [Script C]                                                                                    */
/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

struct KG_LuaScriptData;
struct KG_LuaIncludeData
{
    DWORD             m_dwScriptId;                                     // lua script id
    KG_LuaScriptData *m_pScriptData;                                    // lua script data

    KG_LuaIncludeData()
    {
        m_dwScriptId  = 0;
        m_pScriptData = NULL;
    }
};

struct KG_LuaScriptData
{
    char                           m_szName[KG_MAX_SCIRPT_NAME_LEN];    // lua script name(path)
    std::vector<KG_LuaIncludeData> m_IncludeScripts;                    // all included files of this script

    KG_LuaScriptData()
    {
        KG_ZeroMemory(m_szName, sizeof(m_szName));
    }
};

typedef std::map<DWORD, KG_LuaScriptData> KG_LuaScriptDataMap;

class KG_LuaScriptV51
{
public:
    KG_LuaScriptV51();
    virtual ~KG_LuaScriptV51();

public:
    bool Create();
    void Destroy();

    bool LoadFromFile(const char *pszFileName, DWORD *pdwScriptId);
    bool LoadFromBuff(DWORD dwScriptId, const char *pszScriptName, const char *pBuff, DWORD dwFileSize);

    // add global variable.
    bool AddGlobalInteger(const char *pszVarName, int         nValue  );
    bool AddGlobalString (const char *pszVarName, const char *pszValue);

    // add local variable.
    bool AddLocalInteger(DWORD dwScriptId, const char *pszVarName, int         nValue  );
    bool AddLocalString (DWORD dwScriptId, const char *pszVarName, const char *pszValue);

    bool IsScriptLoaded(DWORD dwScriptId) const;
    bool IsScriptIncluded(DWORD dwScriptId, DWORD dwIncludedScriptId);

    DWORD             GetActiveScriptId() const;
    KG_LuaScriptData *GetScriptData(DWORD dwScriptId);


private:
    bool _AssociateScriptToLua(DWORD dwScriptID);

private:
    static int _Include(lua_State* L);

private:
    lua_State *         m_pLuaState;                                    // lua virtual machine instance.
    DWORD               m_dwActiveScriptId;                             // active lua script id.
    KG_LuaScriptDataMap m_scriptDataMap;                                // all lua script associated data map.
    int                 m_nMetaTableRIdx;                               // the metatable reference.
};

KG_NAMESPACE_END
