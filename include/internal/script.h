#pragma once

#include "public.h"

#include "../../../x3rdlibrary/include/lua-5.3.4/lua.h"
#include "../../../x3rdlibrary/include/lua-5.3.4/lualib.h"
#include "../../../x3rdlibrary/include/lua-5.3.4/lauxlib.h"

#include <map>
#include <vector>

#undef  KG_MAX_SCIRPT_NAME_LEN
#define KG_MAX_SCIRPT_NAME_LEN  256

#undef  KG_LUA_SCRIPT_ID
#define KG_LUA_SCRIPT_ID        "ScriptId"

KG_NAMESPACE_BEGIN(xzero)

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*                                                                                                                                [Script D]                          [Script D]                */
/*                                                            [Script B]                          [Script B]             ---- includeInfo(KG_LuaIncludeInfo) -> scriptInfo(KG_LuaScriptInfo)    */
/*    [Script A]              [Script A]            ---- includeInfo(KG_LuaIncludeInfo) -> scriptInfo(KG_LuaScriptInfo) -|--- ...                                                               */
/*                                                  |                                                                    ---- includeInfo(KG_LuaIncludeInfo) -> scriptInfo(KG_LuaScriptInfo)    */
/*dwScriptId(DWORD) -> scriptInfo(KG_LuaScriptInfo)-|--- ...                                                                      [Script E]                          [Script E]                */
/*                                                  |                                                                                                                                           */
/*                                                  ---- includeInfo(KG_LuaIncludeInfo) -> scriptInfo(KG_LuaScriptInfo)                                                                         */
/*                                                            [Script C]                          [Script C]                                                                                    */
/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

struct KG_LuaScriptInfo;
struct KG_LuaIncludeInfo
{
    DWORD             m_dwScriptId;                                     // lua script id
    KG_LuaScriptInfo *m_pScriptInfo;                                    // lua script info

    KG_LuaIncludeInfo()
    {
        m_dwScriptId  = 0;
        m_pScriptInfo = NULL;
    }
};

struct KG_LuaScriptInfo
{
    char                           m_szName[KG_MAX_SCIRPT_NAME_LEN];    // lua script name(path)
    std::vector<KG_LuaIncludeInfo> m_IncludeScripts;                    // all included files of this script

    KG_LuaScriptInfo()
    {
        KG_ZeroMemory(m_szName, sizeof(m_szName));
    }
};

typedef std::map<DWORD, KG_LuaScriptInfo> KG_LuaScriptInfoMap;

class KG_LuaScript
{
public:
    KG_LuaScript();
    virtual ~KG_LuaScript();

public:
    bool LoadFromBuff(DWORD dwScriptId, const char *pszScriptName, const char *pBuff, DWORD dwFileSize);

    // add global variable.
    bool AddGlobalInteger(const char *pszVarName, int         nValue  );
    bool AddGlobalString (const char *pszVarName, const char *pszValue);

    // add local variable.
    bool AddLocalInteger(DWORD dwScriptId, const char *pszVarName, int         nValue  );
    bool AddLocalString (DWORD dwScriptId, const char *pszVarName, const char *pszValue);

private:
    bool _AssociateScriptToLua(DWORD dwScriptID);

private:
    lua_State *         m_pLuaState;                                    // lua virtual machine instance.
    DWORD               m_dwActiveScriptId;                             // active lua script id.
    KG_LuaScriptInfoMap m_scriptInfoMap;                                // all lua script associated info map.
    int                 m_nMetaTableRIdx;                               // the metatable reference.
};

KG_NAMESPACE_END
