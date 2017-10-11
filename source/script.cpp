#include "script.h"
#include "debug.h"

KG_NAMESPACE_BEGIN(xzero)

KG_LuaScript::KG_LuaScript()
{
    m_pLuaState        = NULL;
    m_dwActiveScriptId = 0;
}

KG_LuaScript::~KG_LuaScript()
{
}

bool KG_LuaScript::LoadFromBuff(DWORD dwScriptID, const char *pcScriptName, const char *pcBuff, DWORD dwFileSize)
{
    bool              bResult            = false;
    int               nRetCode           = 0;
    DWORD             dwPreviousScriptId = m_dwActiveScriptId;                              // save current active script.
    KG_LuaScriptInfo *pScriptInfo        = NULL;

    KG_PROCESS_C_STR_ERROR(pcScriptName);
    KG_PROCESS_PTR_ERROR(pcBuff);

    // switch active script id
    m_dwActiveScriptId = dwScriptID;

    if (0 == dwScriptID)
    { // execute some script temporarily.
        nRetCode = luaL_loadbuffer(m_pLuaState, pcBuff, dwFileSize, pcScriptName);          // loads a buffer as a Lua chunk.
        KG_PROCESS_ERROR(0 == nRetCode);
        KG_PROCESS_SUCCESS(true);
    }

    // load some script in server life-time.
    pScriptInfo = &m_scriptInfoMap[dwScriptID];
    nRetCode    = snprintf(pScriptInfo->m_szName, sizeof(pScriptInfo->m_szName), "%s", pcScriptName);
    KG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(pScriptInfo->m_szName));        // include '\0'.

    //_MakeEnv(dwScriptID);

    //AddInteger(dwScriptID, LUA_SCRIPT_ID, dwScriptID);

    // nRetCode = luaL_loadbuffer(m_pLuaState, pcBuff, dwFileSize, pcScriptName);      // loads a buffer as a Lua chunk.
  //      KG_PROCESS_ERROR(0 == nRetCode);

  //      lua_rawgeti(m_pLuaState, LUA_GLOBALSINDEX, dwScriptID);
		//KG_PROCESS_ERROR(lua_istable(m_pLuaState, -1));
		//lua_setfenv(m_pLuaState, -2);

Exit1:
    nRetCode = lua_pcall(m_pLuaState, 0, 0, 0);                                         // calls a function in protected mode.
    KG_PROCESS_ERROR(0 == nRetCode);
    bResult = true;
Exit0:
    m_dwActiveScriptId = dwPreviousScriptId;                                            // restore to saved active script
    if (!bResult)
    {
        const char *pcErrorMsg = lua_tostring(m_pLuaState, -1);
        if (pcErrorMsg)
        { // TODO : write error message to log file.
        }
    }

    return bResult;
}

// create a new table associated to the specified metatable and save it to gt[dwScriptID]
bool KG_LuaScript::_AssociateScriptToLua(DWORD dwScriptID)
{
    bool bResult = false;
    int  nTopIdx = 0;                                                   // the index of the top element in the stack.
    int  nMTIdx  = 0;                                                   // the index of metatable in the stack.

    KG_PROCESS_ERROR(0 != dwScriptID);

    // create an empty table(t) on top of the stack.
    lua_newtable(m_pLuaState);                                          // creates a new empty table and pushes it onto the stack.
    nTopIdx = lua_gettop(m_pLuaState);
    nMTIdx  = nTopIdx;
    /* --------- */
    /* |   t   | */
    /* --------- */

    // push the metatable(mt) on top of the stack.
    lua_rawgeti(m_pLuaState, LUA_REGISTRYINDEX, m_nMTRef);              // pushes onto the stack the value t[n], where t is the table at the given index, n = m_nMTRef.
    /* --------- */
    /* |   mt  | */
    /* --------- */
    /* |   t   | */
    /* --------- */

    // pop the metatable(mt) and set it as the new metatable of t.
    lua_setmetatable(m_pLuaState, nMTIdx);                              // pops a table from the stack and sets it as the new metatable for the value at the given index.
    /* ---------      */
    /* |   t   | <- mt*/
    /* ---------      */

    //保存环境到全局table
    //lua_rawset(m_pLuaState, LUA_REGISTRYINDEX, m_nGTRef);   // t[k] = v, t is the value at the given index, v is the value at the top of the stack, and k is the value just below the top.

    bResult = true;
Exit0:
    return bResult;
}

KG_NAMESPACE_END
