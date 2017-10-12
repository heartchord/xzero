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

bool KG_LuaScript::LoadFromBuff(DWORD dwScriptId, const char *pszScriptName, const char *pBuff, DWORD dwFileSize)
{
    bool              bResult            = false;
    int               nRetCode           = 0;
    DWORD             dwPreviousScriptId = m_dwActiveScriptId;                              // save current active script.
    KG_LuaScriptInfo *pScriptInfo        = NULL;

    KG_PROCESS_C_STR_ERROR(pszScriptName);
    KG_PROCESS_PTR_ERROR(pBuff);

    // switch active script id
    m_dwActiveScriptId = dwScriptId;

    if (0 == dwScriptId)
    { // execute some script temporarily.
        nRetCode = luaL_loadbuffer(m_pLuaState, pBuff, dwFileSize, pszScriptName);          // loads a buffer as a Lua chunk.
        KG_PROCESS_ERROR(0 == nRetCode);

        // call a function in protected mode.
        nRetCode = lua_pcall(m_pLuaState, 0, 0, 0);
        KG_PROCESS_ERROR(0 == nRetCode);

        KG_PROCESS_SUCCESS(true);
    }

    // load some script in server life-time.
    pScriptInfo = &m_scriptInfoMap[dwScriptId];
    nRetCode    = snprintf(pScriptInfo->m_szName, sizeof(pScriptInfo->m_szName), "%s", pszScriptName);
    KG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(pScriptInfo->m_szName));        // include '\0'.

    // associate this script to lua.
    nRetCode = _AssociateScriptToLua(dwScriptId);
    KG_PROCESS_ERROR(nRetCode);

    // write this hash id to file local table.
    nRetCode = AddLocalInteger(dwScriptId, KG_LUA_SCRIPT_ID, dwScriptId);

    // load this buff as a Lua chunk.
    nRetCode = luaL_loadbuffer(m_pLuaState, pBuff, dwFileSize, pszScriptName);
    KG_PROCESS_ERROR(0 == nRetCode);
    /* --------- */
    /* | chunk | */
    /* --------- */

    // lua_rawgeti(m_pLuaState, LUA_GLOBALSINDEX, dwScriptID);
    /* --------- */
    /* |   t   | */
    /* --------- */
    /* | chunk | */
    /* --------- */

    //KG_PROCESS_ERROR(lua_istable(m_pLuaState, -1));
    //lua_setfenv(m_pLuaState, -2);
    /* --------- */
    /* | chunk | */
    /* --------- */

    // calls a function in protected mode.
    nRetCode = lua_pcall(m_pLuaState, 0, 0, 0);
    KG_PROCESS_ERROR(0 == nRetCode);

Exit1:
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

bool KG_LuaScript::AddGlobalInteger(const char *pszVarName, int nValue)
{
    bool bResult = false;

    KG_PROCESS_C_STR_ERROR(pszVarName);

    // push gt onto the stack.
    lua_pushglobaltable(m_pLuaState);                                   // lua_rawgeti(m_pLuaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS)
    /* --------- */
    /* |   gt  | */
    /* --------- */

    // push key and value onto the stack.
    lua_pushstring(m_pLuaState, pszVarName);
    lua_pushnumber(m_pLuaState, nValue    );
    /* --------- */
    /* | value | */
    /* --------- */
    /* |  name | */
    /* --------- */
    /* |   gt  | */
    /* --------- */

    // gt[name] = value
    lua_settable(m_pLuaState, -3);                                      // t[k] = v, where t is the value at the given index, v is the value at the top of the stack, and k is the value just below the top.

    // remove the gt from the stack
    lua_pop(m_pLuaState, 1);                                            // pops n elements from the stack.

    bResult = true;
Exit0:
    return bResult;
}

bool KG_LuaScript::AddGlobalString(const char *pszVarName, const char *pszValue)
{
    bool bResult = false;

    KG_PROCESS_C_STR_ERROR(pszVarName);
    KG_PROCESS_C_STR_ERROR(pszValue  );

    // push gt onto the stack.
    lua_pushglobaltable(m_pLuaState);                                   // lua_rawgeti(m_pLuaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS)
    /* --------- */
    /* |   gt  | */
    /* --------- */

    // push key and value onto the stack.
    lua_pushstring(m_pLuaState, pszVarName);
    lua_pushstring(m_pLuaState, pszValue  );
    /* --------- */
    /* | value | */
    /* --------- */
    /* |  name | */
    /* --------- */
    /* |   gt  | */
    /* --------- */

    // gt[name] = value
    lua_settable(m_pLuaState, -3);                                      // t[k] = v, where t is the value at the given index, v is the value at the top of the stack, and k is the value just below the top.

    // remove the gt from the stack
    lua_pop(m_pLuaState, 1);                                            // pops n elements from the stack.

    bResult = true;
Exit0:
    return bResult;
}

bool KG_LuaScript::AddLocalInteger(DWORD dwScriptId, const char* pszVarName, int nValue)
{
    bool bResult  = false;
    int  nRetCode = 0;

    KG_PROCESS_C_STR_ERROR(pszVarName);

    if (0 == dwScriptId)
    { // add to global variable.
        nRetCode = AddGlobalInteger(pszVarName, nValue);
        KG_PROCESS_ERROR(nRetCode);
    }

    // push gt onto the stack.
    lua_pushglobaltable(m_pLuaState);                                   // lua_rawgeti(m_pLuaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS)
    /* --------- */
    /* |   gt  | */
    /* --------- */

    lua_rawgeti(m_pLuaState, -1, dwScriptId);                           // pushes onto the stack the value t[n], where t is the table at the given index, n = dwScriptId.
    /* --------- */
    /* |   t   | */
    /* --------- */
    /* |   gt  | */
    /* --------- */

    nRetCode = lua_istable(m_pLuaState, -1);
    KG_PROCESS_ERROR(nRetCode);

    // push key and value onto the stack.
    lua_pushstring(m_pLuaState, pszVarName);
    lua_pushnumber(m_pLuaState, nValue    );
    /* --------- */
    /* | value | */
    /* --------- */
    /* |  name | */
    /* --------- */
    /* |   t   | */
    /* --------- */
    /* |   gt  | */
    /* --------- */

    lua_settable(m_pLuaState, -3);                                      // t[k] = v, where t is the value at the given index, v is the value at the top of the stack, and k is the value just below the top.
    /* --------- */
    /* |   t   | */
    /* --------- */
    /* |   gt  | */
    /* --------- */

    // remove the gt from the stack.
    lua_pop(m_pLuaState, 2);

    bResult = true;
Exit0:
    return bResult;
}

bool KG_LuaScript::AddLocalString(DWORD dwScriptId, const char *pszVarName, const char *pszValue)
{
    bool bResult  = false;
    int  nRetCode = 0;

    KG_PROCESS_C_STR_ERROR(pszVarName);
    KG_PROCESS_C_STR_ERROR(pszValue  );

    if (0 == dwScriptId)
    { // add to global variable.
        nRetCode = AddGlobalString(pszVarName, pszValue);
        KG_PROCESS_ERROR(nRetCode);
    }

    // push gt onto the stack.
    lua_pushglobaltable(m_pLuaState);                                   // lua_rawgeti(m_pLuaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS)
    /* --------- */
    /* |   gt  | */
    /* --------- */

    lua_rawgeti(m_pLuaState, -1, dwScriptId);                           // pushes onto the stack the value t[n], where t is the table at the given index, n = dwScriptId.
    /* --------- */
    /* |   t   | */
    /* --------- */
    /* |   gt  | */
    /* --------- */

    nRetCode = lua_istable(m_pLuaState, -1);
    KG_PROCESS_ERROR(nRetCode);

    // push key and value onto the stack.
    lua_pushstring(m_pLuaState, pszVarName);
    lua_pushstring(m_pLuaState, pszValue  );
    /* --------- */
    /* | value | */
    /* --------- */
    /* |  name | */
    /* --------- */
    /* |   t   | */
    /* --------- */
    /* |   gt  | */
    /* --------- */

    lua_settable(m_pLuaState, -3);                                      // t[k] = v, where t is the value at the given index, v is the value at the top of the stack, and k is the value just below the top.
    /* --------- */
    /* |   t   | */
    /* --------- */
    /* |   gt  | */
    /* --------- */

    // remove the gt from the stack.
    lua_pop(m_pLuaState, 2);

    bResult = true;
Exit0:
    return bResult;
}

// create a new table associated to the specified metatable and save it to gt[dwScriptID]
bool KG_LuaScript::_AssociateScriptToLua(DWORD dwScriptId)
{
    bool bResult       = false;
    int  nStackTopIdx  = 0;                                             // the index of the top element in the stack.
    int  nMetaTableIdx = 0;                                             // the index of metatable in the stack.

    KG_PROCESS_ERROR(0 != dwScriptId);

    // push gt onto the stack.
    lua_pushglobaltable(m_pLuaState);                                   // lua_rawgeti(m_pLuaState, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS)
    /* --------- */
    /* |   gt  | */
    /* --------- */

    // create an empty table(t) on top of the stack.
    lua_newtable(m_pLuaState);                                          // creates a new empty table and pushes it onto the stack.
    nStackTopIdx  = lua_gettop(m_pLuaState);
    nMetaTableIdx = nStackTopIdx;
    /* -------------------------- */
    /* |   t   | <- nMetaTableIdx */
    /* -------------------------- */
    /* |   gt  |                  */
    /* -------------------------- */

    // push the metatable(mt) on top of the stack.
    lua_rawgeti(m_pLuaState, LUA_REGISTRYINDEX, m_nMetaTableRIdx);      // pushes onto the stack the value t[n], where t is the table at the given index, n = m_nMTRef.
    /* -------------------------- */
    /* |   mt  |                  */
    /* -------------------------  */
    /* |   t   | <- nMetaTableIdx */
    /* -------------------------- */
    /* |   gt  |                  */
    /* -------------------------- */

    // pop the metatable(mt) and set it as the new metatable of t.
    lua_setmetatable(m_pLuaState, nMetaTableIdx);                       // pops a table from the stack and sets it as the new metatable for the value at the given index.
    /* ---------      */
    /* |   t   | <= mt*/
    /* ---------      */
    /* |   gt  |      */
    /* -------------- */

    // save script table to gt.
    lua_rawseti(m_pLuaState, -2, dwScriptId);                           // t[i] = v, where t is the table at the given index and v is the value at the top of the stack.

    // remove the gt from the stack
    lua_pop(m_pLuaState, 2);                                            // pops n elements from the stack.

    bResult = true;
Exit0:
    return bResult;
}

KG_NAMESPACE_END
