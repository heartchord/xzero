#pragma once

#include "public.h"
#include "debug.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../x3rdlibrary/include/lua-5.1.5/lua.h"
#include "../../../x3rdlibrary/include/lua-5.1.5/lualib.h"
#include "../../../x3rdlibrary/include/lua-5.1.5/lstate.h"
#include "../../../x3rdlibrary/include/lua-5.1.5/lauxlib.h"

#ifdef __cplusplus
}
#endif

#include <map>
#include <vector>

#undef  KG_MAX_SCIRPT_NAME_LEN
#define KG_MAX_SCIRPT_NAME_LEN  256

#undef  KG_LUA_SCRIPT_ID
#define KG_LUA_SCRIPT_ID        "ScriptId"

#undef  KG_LUA_SCRIPT_THIS
#define KG_LUA_SCRIPT_THIS      "KG_LuaScriptThis"

KG_NAMESPACE_BEGIN(xzero)

inline void KG_CloseLuaStateSafely(lua_State *&pLuaState)
{
    if (NULL != pLuaState)
    {
        lua_close(pLuaState);
        pLuaState = NULL;
    }
}

inline void KG_LuaUnRefSafely(lua_State *pLuaState, int &nRIdx)
{
    if (LUA_NOREF != nRIdx)
    {
        KG_ASSERT(pLuaState);
        luaL_unref(pLuaState, LUA_REGISTRYINDEX, nRIdx);
        nRIdx = LUA_NOREF;
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
    bool Init();                                                                                            // init env
    void UnInit();                                                                                          // uninit env
    void Release();                                                                                         // uninit env and destroy instance

    bool LoadFromFile(const char *pszFileName, DWORD *pdwScriptId);                                         // load chunk from file
    bool LoadFromBuff(DWORD dwScriptId, const char *pszScriptName, const char *pBuff, DWORD dwFileSize);    // load chunk from buffer

    // add global variable.
    bool AddGlobalInteger(const char *pszVarName, int         nValue  );                                    // add global variable
    bool AddGlobalString (const char *pszVarName, const char *pszValue);                                    // add global variable

    // add local variable.
    bool AddLocalInteger(DWORD dwScriptId, const char *pszVarName, int         nValue  );                   // add local variable
    bool AddLocalString (DWORD dwScriptId, const char *pszVarName, const char *pszValue);                   // add local variable

    bool IsScriptLoaded(DWORD dwScriptId) const;                                                            // is script loaded?
    bool IsScriptIncluded(DWORD dwScriptId, DWORD dwIncludedScriptId);                                      // has script A included script B?

    DWORD             GetActiveScriptId() const;                                                            // get script id of current invoked script.
    KG_LuaScriptData *GetScriptData(DWORD dwScriptId);                                                      // get associated data of script.
    lua_State *       GetLuaState();                                                                        // get lua_State instance.


private:
    bool _AssociateScriptToLua(DWORD dwScriptID);
    bool _GetVarInIncludeScripts(const KG_LuaIncludeData &includes, const char *pszValueName);              // try to access variable's value from all including files

private:
    static int _LuaInclude(lua_State* L);                                                                   // custom "Include" keyword.
    static int _LuaIndex(lua_State* L);                                                                     // the "__index" function of mt.

private:
    lua_State *         m_pLuaState;                                                                        // lua virtual machine instance.
    DWORD               m_dwActiveScriptId;                                                                 // active lua script id.
    KG_LuaScriptDataMap m_scriptDataMap;                                                                    // all lua script associated data map.
    int                 m_nMetaTableRIdx;                                                                   // the metatable reference.
};

KG_NAMESPACE_END
