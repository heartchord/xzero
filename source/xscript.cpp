#include "xhash.h"
#include "xtime.h"
#include "xfile.h"
#include "xpath.h"
#include "xdebug.h"
#include "xscript.h"
#include "xstring.h"
#include "xpointer.h"

KG_NAMESPACE_BEGIN(xzero)

KG_LuaScriptV51::KG_LuaScriptV51()
{
    m_pLuaState        = NULL;
    m_dwActiveScriptId = 0;
    m_nMetaTableRIdx   = LUA_NOREF;
}

KG_LuaScriptV51::~KG_LuaScriptV51()
{
}

bool KG_LuaScriptV51::Init()
{
    bool bResult      = false;
    int  nStackTopIdx = 0;

    m_nMetaTableRIdx = LUA_NOREF;

    KG_PROCESS_ERROR(NULL == m_pLuaState);

    m_pLuaState = luaL_newstate();
    KG_PROCESS_PTR_ERROR(m_pLuaState);

    luaL_openlibs(m_pLuaState);
    lua_register(m_pLuaState, "Include", _LuaInclude);

    lua_pushstring(m_pLuaState, KG_LUA_SCRIPT_THIS);
    lua_pushlightuserdata(m_pLuaState, this);
    /* ---------------------- */
    /* |        this        | */
    /* ---------------------- */
    /* | "KG_LuaScriptThis" | */
    /* ---------------------- */
    // gt["KG_LuaScriptThis"] = this
    lua_settable(m_pLuaState, LUA_GLOBALSINDEX);                        // t[k] = v, where t is the value at the given index, v is the value at the top of the stack, and k is the value just below the top.

    lua_newtable(m_pLuaState);
    nStackTopIdx = lua_gettop(m_pLuaState);
    lua_pushstring(m_pLuaState, "__index");
    /* ----------------------------- */
    /* | "__index" |                 */
    /* ----------------------------- */
    /* |     t     | <= nStackTopIdx */
    /* ----------------------------- */

    lua_pushcfunction(m_pLuaState, KG_LuaScriptV51::_LuaIndex);
    /* ---------------------------------------------- */
    /* | KG_LuaScriptV51::_LuaIndex |                 */
    /* ---------------------------------------------- */
    /* |       "__index"            |                 */
    /* ---------------------------------------------- */
    /* |            t               | <= nStackTopIdx */
    /* ---------------------------------------------- */

    lua_settable(m_pLuaState, nStackTopIdx);                            // t["__index"] = KG_LuaScriptV51::_LuaIndex
    /* ----------------------------- */
    /* |     t     | <= nStackTopIdx */
    /* ----------------------------- */

    m_nMetaTableRIdx = luaL_ref(m_pLuaState, LUA_REGISTRYINDEX);        // creates and returns a reference in the table at index t, for the object at the top of the stack (and pops the object).

    bResult = true;
Exit0:
    if (!bResult)
    {
        KG_LuaUnRefSafely(m_pLuaState, m_nMetaTableRIdx);
        KG_CloseLuaStateSafely(m_pLuaState);
    }
    return bResult;
}

void KG_LuaScriptV51::UnInit()
{
    KG_LuaUnRefSafely(m_pLuaState, m_nMetaTableRIdx);

    if (m_pLuaState)
    {
        stringtable *pStrTb = &G(m_pLuaState)->strt;
        lu_int32     uNuse  = pStrTb->nuse;
        lu_int32     uSize  = pStrTb->size;
        printf("[LUA] Global strt size: %u/%d\n", uNuse, uSize);
    }

    KG_CloseLuaStateSafely(m_pLuaState);

    m_dwActiveScriptId = 0;
    m_scriptDataMap.clear();
    //m_mapFunctionData.clear();
}

void KG_LuaScriptV51::Release()
{
    UnInit();
    delete this;
}

bool KG_LuaScriptV51::LoadFromFile(const char *pszFileName, DWORD *pdwScriptId)
{
    bool    bResult    = false;
    int     nRetCode   = 0;
    DWORD   dwFileSize = 0;
    DWORD   dwReadSize = 0;
    char *  pBuf       = NULL;
    KG_File fs;

    KG_PROCESS_PTR_ERROR(pdwScriptId);
    KG_PROCESS_C_STR_ERROR(pszFileName);

    *pdwScriptId = KG_KSGFileNameHash(pszFileName);
    KG_PROCESS_ERROR(*pdwScriptId > 0);

    nRetCode = IsScriptLoaded(*pdwScriptId);
    KG_PROCESS_SUCCESS(nRetCode);                                       // file has been loaded.

    nRetCode = fs.Open(pszFileName, "rb");
    KG_PROCESS_ERROR(nRetCode);

    dwFileSize = fs.Size();
    KG_PROCESS_ERROR(dwFileSize > 0);

    pBuf = new char[dwFileSize];
    KG_PROCESS_PTR_ERROR(pBuf);

    dwReadSize = fs.Read(pBuf, dwFileSize, dwFileSize);
    KG_PROCESS_ERROR(dwReadSize == dwFileSize);

    nRetCode = LoadFromBuff(*pdwScriptId, pszFileName, pBuf, dwFileSize);
    KG_PROCESS_ERROR(nRetCode);

Exit1:
    bResult = true;
Exit0:
    if (!bResult)
    {
        KG_ASSERT(false);
        //KGLogPrintf(KGLOG_ERR, "[Lua] Failed to load script: %s", cszFileName);
    }

    KG_DeleteArrayPtrSafely(pBuf);
    fs.Close();

    return bResult;
}

bool KG_LuaScriptV51::LoadFromBuff(DWORD dwScriptId, const char *pszScriptName, const char *pBuff, DWORD dwFileSize)
{
    bool              bResult            = false;
    int               nRetCode           = 0;
    DWORD             dwPreviousScriptId = m_dwActiveScriptId;                              // save current active script.
    KG_LuaScriptData *pScriptData        = NULL;

    KG_PROCESS_C_STR_ERROR(pszScriptName);
    KG_PROCESS_PTR_ERROR(pBuff);

    // switch active script id
    m_dwActiveScriptId = dwScriptId;

    if (0 == dwScriptId)
    { // execute some script temporarily.
        nRetCode = luaL_loadbuffer(m_pLuaState, pBuff, dwFileSize, pszScriptName);          // loads a buffer as a Lua chunk.
        KG_PROCESS_ERROR(0 == nRetCode);
        KG_PROCESS_SUCCESS(true);
    }

    // load some script in server life-time.
    pScriptData = &m_scriptDataMap[dwScriptId];
    nRetCode    = KG_Snprintf(pScriptData->m_szName, sizeof(pScriptData->m_szName), "%s", pszScriptName);
    KG_PROCESS_ERROR(nRetCode > 0 && nRetCode < (int)sizeof(pScriptData->m_szName));        // include '\0'.

    // associate this script to lua.
    nRetCode = _AssociateScriptToLua(dwScriptId);
    KG_PROCESS_ERROR(nRetCode);

    // write this hash id to file local table.
    nRetCode = AddLocalInteger(dwScriptId, KG_LUA_SCRIPT_ID, dwScriptId);
    KG_PROCESS_ERROR(nRetCode);

    // load this buff as a Lua chunk.
    nRetCode = luaL_loadbuffer(m_pLuaState, pBuff, dwFileSize, pszScriptName);
    KG_PROCESS_ERROR(0 == nRetCode);
    /* --------- */
    /* | chunk | */
    /* --------- */

    lua_rawgeti(m_pLuaState, LUA_GLOBALSINDEX, dwScriptId);
    nRetCode = lua_istable(m_pLuaState, -1);
    KG_PROCESS_ERROR(nRetCode);
    /* --------- */
    /* |   t   | */
    /* --------- */
    /* | chunk | */
    /* --------- */

    lua_setfenv(m_pLuaState, -2);
    /* --------- */
    /* | chunk | */
    /* --------- */
Exit1:
    // call a function in protected mode.
    nRetCode = lua_pcall(m_pLuaState, 0, 0, 0);
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

bool KG_LuaScriptV51::AddGlobalInteger(const char *pszVarName, int nValue)
{
    bool bResult = false;

    KG_PROCESS_C_STR_ERROR(pszVarName);

    // push key and value onto the stack.
    lua_pushstring(m_pLuaState, pszVarName);
    lua_pushnumber(m_pLuaState, nValue    );
    /* --------- */
    /* | value | */
    /* --------- */
    /* |  name | */
    /* --------- */

    // gt[name] = value
    lua_settable(m_pLuaState, LUA_GLOBALSINDEX);                        // t[k] = v, where t is the value at the given index, v is the value at the top of the stack, and k is the value just below the top.

    bResult = true;
Exit0:
    return bResult;
}

bool KG_LuaScriptV51::AddGlobalString(const char *pszVarName, const char *pszValue)
{
    bool bResult = false;

    KG_PROCESS_C_STR_ERROR(pszVarName);
    KG_PROCESS_C_STR_ERROR(pszValue  );

    // push key and value onto the stack.
    lua_pushstring(m_pLuaState, pszVarName);
    lua_pushstring(m_pLuaState, pszValue  );
    /* --------- */
    /* | value | */
    /* --------- */
    /* |  name | */
    /* --------- */

    // gt[name] = value
    lua_settable(m_pLuaState, LUA_GLOBALSINDEX);                        // t[k] = v, where t is the value at the given index, v is the value at the top of the stack, and k is the value just below the top.

    bResult = true;
Exit0:
    return bResult;
}

bool KG_LuaScriptV51::AddLocalInteger(DWORD dwScriptId, const char* pszVarName, int nValue)
{
    bool bResult      = false;
    int  nRetCode     = 0;
    int  nStackTopIdx = 0;                                              // the index of the top element in the stack.

    KG_PROCESS_C_STR_ERROR(pszVarName);

    if (0 == dwScriptId)
    { // add to global variable.
        nRetCode = AddGlobalInteger(pszVarName, nValue);
        KG_PROCESS_ERROR(nRetCode);
        KG_PROCESS_SUCCESS(true);
    }

    lua_rawgeti(m_pLuaState, LUA_GLOBALSINDEX, dwScriptId);             // pushes onto the stack the value t[n], where t is the table at the given index, n = dwScriptId.
    nStackTopIdx = lua_gettop(m_pLuaState);
    /* --------- */
    /* |   t   | */
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

    lua_settable(m_pLuaState, nStackTopIdx);                            // t[k] = v, where t is the value at the given index, v is the value at the top of the stack, and k is the value just below the top.
    /* --------- */
    /* |   t   | */
    /* --------- */

    // remove the t from the stack.
    lua_pop(m_pLuaState, 1);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

bool KG_LuaScriptV51::AddLocalString(DWORD dwScriptId, const char *pszVarName, const char *pszValue)
{
    bool bResult      = false;
    int  nRetCode     = 0;
    int  nStackTopIdx = 0;                                              // the index of the top element in the stack.

    KG_PROCESS_C_STR_ERROR(pszVarName);
    KG_PROCESS_C_STR_ERROR(pszValue  );

    if (0 == dwScriptId)
    { // add to global variable.
        nRetCode = AddGlobalString(pszVarName, pszValue);
        KG_PROCESS_ERROR(nRetCode);
        KG_PROCESS_SUCCESS(true);
    }

    lua_rawgeti(m_pLuaState, LUA_GLOBALSINDEX, dwScriptId);             // pushes onto the stack the value t[n], where t is the table at the given index, n = dwScriptId.
    nStackTopIdx = lua_gettop(m_pLuaState);
    /* --------- */
    /* |   t   | */
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

    lua_settable(m_pLuaState, nStackTopIdx);                            // t[k] = v, where t is the value at the given index, v is the value at the top of the stack, and k is the value just below the top.
    /* --------- */
    /* |   t   | */
    /* --------- */

    // remove the t from the stack.
    lua_pop(m_pLuaState, 1);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

bool KG_LuaScriptV51::IsScriptLoaded(DWORD dwScriptId) const
{
    bool                                bResult = false;
    KG_LuaScriptDataMap::const_iterator it      = m_scriptDataMap.find(dwScriptId);

    if (it != m_scriptDataMap.end())
    {
        bResult = true;
    }

    return bResult;
}

bool KG_LuaScriptV51::IsScriptIncluded(DWORD dwScriptId, DWORD dwIncludedScriptId)
{
    bool              bResult     = false;
    size_t            uIdx        = 0;
    size_t            uSize       = 0;
    KG_LuaScriptData* pScriptData = NULL;

    pScriptData = GetScriptData(dwScriptId);
    KG_PROCESS_PTR_ERROR(pScriptData);

    uSize = pScriptData->m_IncludeScripts.size();
    for (uIdx = 0; uIdx < uSize; uIdx++)
    {
        if (pScriptData->m_IncludeScripts[uIdx].m_dwScriptId == dwIncludedScriptId)
        {
            break;
        }
    }

    KG_PROCESS_ERROR_Q(uIdx != uSize);                                  // included

    bResult = true;
Exit0:
    return bResult;
}

DWORD KG_LuaScriptV51::GetActiveScriptId() const
{
    return m_dwActiveScriptId;
}

KG_LuaScriptData *KG_LuaScriptV51::GetScriptData(DWORD dwScriptId)
{
    KG_LuaScriptData *            pResult = NULL;
    KG_LuaScriptDataMap::iterator it      = m_scriptDataMap.find(dwScriptId);

    if (it != m_scriptDataMap.end())
    {
        pResult = &it->second;
    }

    return pResult;
}

lua_State *KG_LuaScriptV51::GetLuaState()
{
    return m_pLuaState;
}

void KG_LuaScriptV51::DumpStrt() const
{
    int          nRetCode                     = 0;
    stringtable *pST                          = NULL;
    lu_int32     uNuse                        = 0;
    lu_int32     uSize                        = 0;
    time_t       nTime                        = ::time(NULL);
    char         szFileName[KG_MAX_FILE_PATH] = {'\0'};
    KG_File      fs;
    struct tm    now;

    KG_PROCESS_ERROR(m_pLuaState);

    pST   = &G(m_pLuaState)->strt;
    uNuse = pST->nuse;
    uSize = pST->size;

    KG_LocalTime(&nTime, &now);
    KG_CreatePath("lua_dump");

    KG_Snprintf(
        szFileName, sizeof(szFileName),
        "lua_dump/strt_dump_%d%2.2d%2.2d-%2.2d%2.2d%2.2d.txt",
        now.tm_year + 1900, now.tm_mon + 1, now.tm_mday,
        now.tm_hour, now.tm_min, now.tm_sec
    );

    nRetCode = fs.Open("szFileName", "wb");
    KG_PROCESS_ERROR(nRetCode);

    //fprintf(pFile, "Global strt size: %u/%d\n", uNuse, uSize);

    //for (int i= 0; i < uSize; i++)
    //{
    //    GCObject *p = pST->hash[i];
    //    while (p)
    //    {  // for each node in the list
    //        GCObject *next = p->gch.next;  // save next
    //        fprintf(pFile, "%s\n", getstr(gco2ts(p)));
    //        p = next;
    //    }
    //}

Exit0:
    fs.Close();
}

// create a new table associated to the specified metatable and save it to gt[dwScriptID]
bool KG_LuaScriptV51::_AssociateScriptToLua(DWORD dwScriptId)
{
    bool bResult       = false;
    int  nStackTopIdx  = 0;                                             // the index of the top element in the stack.
    int  nMetaTableIdx = 0;                                             // the index of metatable in the stack.

    KG_PROCESS_ERROR(0 != dwScriptId);

    // create an empty table(t) on top of the stack.
    lua_newtable(m_pLuaState);                                          // creates a new empty table and pushes it onto the stack.
    nStackTopIdx  = lua_gettop(m_pLuaState);
    nMetaTableIdx = nStackTopIdx;
    /* -------------------------- */
    /* |   t   | <- nMetaTableIdx */
    /* -------------------------- */

    // push the metatable(mt) on top of the stack.
    lua_rawgeti(m_pLuaState, LUA_REGISTRYINDEX, m_nMetaTableRIdx);      // pushes onto the stack the value t[n], where t is the table at the given index, n = m_nMTRef.
    /* -------------------------- */
    /* |   mt  |                  */
    /* -------------------------  */
    /* |   t   | <- nMetaTableIdx */
    /* -------------------------- */

    // pop the metatable(mt) and set it as the new metatable of t.
    lua_setmetatable(m_pLuaState, nMetaTableIdx);                       // pops a table from the stack and sets it as the new metatable for the value at the given index.
    /* -------------- */
    /* |   t   | <= mt*/
    /* -------------- */

    // save script table to gt.
    lua_rawseti(m_pLuaState, LUA_GLOBALSINDEX, dwScriptId);             // t[i] = v, where t is the table at the given index and v is the value at the top of the stack.

    bResult = true;
Exit0:
    return bResult;
}

bool KG_LuaScriptV51::_GetVarInIncludeScripts(const KG_LuaIncludeData &includes, const char *pszValueName)
{
    bool              bResult     = false;
    int               nRetCode    = 0;
    DWORD             dwScriptId  = 0;
    KG_LuaScriptData *pScriptData = NULL;
    int               nIdx        = 0;
    size_t            uSize       = 0;

    dwScriptId  = includes.m_dwScriptId;
    pScriptData = includes.m_pScriptData;

    KG_PROCESS_ERROR(dwScriptId > 0);
    KG_PROCESS_PTR_ERROR(pScriptData);
    KG_PROCESS_C_STR_ERROR(pszValueName);

    uSize = pScriptData->m_IncludeScripts.size();
    for (nIdx = uSize - 1; nIdx >= 0; nIdx--)
    { //  Traverse all including scripts in a reverse order, to let the including file seems to be overridden.
        dwScriptId = pScriptData->m_IncludeScripts[nIdx].m_dwScriptId;
        if (dwScriptId == 0)
        {
            continue;
        }

        lua_rawgeti(m_pLuaState, LUA_GLOBALSINDEX, dwScriptId);
        nRetCode = lua_istable(m_pLuaState, -1);
        if (!nRetCode)
        { // the target script hasn't been loaded.
            continue;
        }

        lua_pushstring(m_pLuaState, pszValueName);
        /* ---------------- */
        /* | pszValueName | */
        /* ---------------  */
        /* |      t       | */
        /* ---------------- */

        lua_rawget(m_pLuaState, -2);                                    // Pushes onto the stack the value t[k], where t is the value at the given valid index and k is the value at the top of the stack.
        /* --------- */
        /* | value | */
        /* --------  */
        /* |   t   | */
        /* --------- */

        lua_remove(m_pLuaState, -2);                                    // remove t
        /* --------- */
        /* | value | */
        /* --------  */

        nRetCode = lua_isnil(m_pLuaState, -1);
        if (nRetCode)
        { // target variable is nil
            lua_remove(m_pLuaState, -1);                                // remove v

            // traverse all sub including files recursively.
            nRetCode = _GetVarInIncludeScripts(pScriptData->m_IncludeScripts[nIdx], pszValueName);
            if (!nRetCode)
            { // not found
                continue;
            }
        }

        // found here.
        break;
    }

    KG_PROCESS_ERROR_Q(nIdx >= 0);

    bResult = true;
Exit0:
    return bResult;
}

int KG_LuaScriptV51::_LuaInclude(lua_State* L)
{
    int               nResult          = 0;
    int               nRetCode         = 0;
    DWORD             dwScriptId       = 0;
    DWORD             dwActiveScriptId = 0;
    const char *      pszFileName      = NULL;
    KG_LuaScriptV51 * pThis            = NULL;
    KG_LuaScriptData *pScriptData      = NULL;
    KG_LuaIncludeData data;

    KG_PROCESS_PTR_ERROR(L);
    /* -------------------------  */
    /* |      pszFileName       | */
    /* -------------------------- */

    nRetCode = lua_gettop(L);
    KG_PROCESS_ERROR(1 == nRetCode);                                    // "Include" got only one parameter.

    pszFileName = lua_tostring(L, 1);
    KG_PROCESS_C_STR_ERROR(pszFileName);

    dwScriptId = KG_KSGFileNameHash(pszFileName);
    KG_PROCESS_ERROR(dwScriptId > 0);

    // get KG_LuaScriptV51 instance from lua.
    lua_pushstring(L, KG_LUA_SCRIPT_THIS);
    lua_gettable(L, LUA_GLOBALSINDEX);                                  // Pushes onto the stack the value t[k], where t is the value at the given valid index and k is the value at the top of the stack.
    /* -------------------------- */
    /* | gt["KG_LuaScriptThis"] | */
    /* -------------------------  */
    /* |      pszFileName       | */
    /* -------------------------- */

    pThis = (KG_LuaScriptV51 *)lua_touserdata(L, 2);
    KG_PROCESS_PTR_ERROR(pThis);

    dwActiveScriptId = pThis->GetActiveScriptId();                      // get script id of current script
    KG_PROCESS_ERROR(dwActiveScriptId > 0);

    pScriptData = pThis->GetScriptData(dwActiveScriptId);               // get script data of current script
    KG_PROCESS_PTR_ERROR(pScriptData);

    nRetCode = pThis->IsScriptLoaded(dwScriptId);
    if (!nRetCode)
    { // If included file not loaded
        nRetCode = pThis->LoadFromFile(pszFileName, &dwScriptId);       // load including script
        KG_PROCESS_ERROR(nRetCode);
    }

    nRetCode = pThis->IsScriptIncluded(dwActiveScriptId, dwScriptId);
    KG_PROCESS_ERROR(!nRetCode);                                        // the target script can't be included.

    // add to included list
    data.m_dwScriptId  = dwScriptId;
    data.m_pScriptData = pThis->GetScriptData(dwScriptId);
    pScriptData->m_IncludeScripts.push_back(data);

Exit0:
    return nResult;
}

int KG_LuaScriptV51::_LuaIndex(lua_State* L)
{
    int               nResult     = 0;
    int               nRetCode    = 0;
    DWORD             dwScriptID  = 0;
    KG_LuaScriptV51 * pThis       = NULL;
    const char *      pszKey      = lua_tostring(L, 2);
    KG_LuaIncludeData include;

    KG_PROCESS_C_STR_ERROR(pszKey);

    lua_pushstring(L, KG_LUA_SCRIPT_THIS);
    /* ---------------------- */
    /* | "KG_LuaScriptThis" | */
    /* ---------------------- */
    /* |        key         | */
    /* ---------------------  */
    /* |         t          | */
    /* ---------------------- */

    lua_gettable(L, LUA_GLOBALSINDEX);
    /* ---------------------- */
    /* |       this         | */
    /* ---------------------- */
    /* |        key         | */
    /* ---------------------  */
    /* |         t          | */
    /* ---------------------- */

    pThis = (KG_LuaScriptV51 *)lua_touserdata(L, -1);
    KG_PROCESS_PTR_ERROR(pThis);

    include.m_dwScriptId  = pThis->GetActiveScriptId();
    include.m_pScriptData = pThis->GetScriptData(include.m_dwScriptId);

    nRetCode = pThis->_GetVarInIncludeScripts(include, pszKey);
    if (!nRetCode)
    { // try to get global variable.
        lua_getglobal(L, pszKey);
    }

    nResult = 1;
Exit0:
    return nResult;
}

KG_NAMESPACE_END
