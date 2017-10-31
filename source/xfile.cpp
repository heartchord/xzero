#include "str.h"
#include "hash.h"
#include "file.h"
#include "debug.h"
#include "pointer.h"

# pragma warning(disable: 4996)

KG_NAMESPACE_BEGIN(xzero)

int KG_OpenFileSafely(FILE *&fp, const char * const cszFile, const char * const cszMode)
{
    int nResult = false;

    KG_PROCESS_C_STR_ERROR(cszFile);
    KG_PROCESS_C_STR_ERROR(cszMode);

    KG_PROCESS_ERROR(NULL == fp && "[ERROR] File handle has been used!");
    fp = ::fopen(cszFile, cszMode);
    KG_PROCESS_ERROR(NULL != fp && "[ERROR] Open file failed!");

    nResult = true;
Exit0:
    return nResult;
}

int KG_CloseFileSafely(FILE *&fp)
{
    int nResult  = false;
    int nRetCode = 0;

    KG_PROCESS_SUCCESS(NULL == fp);

    nRetCode = ::fclose(fp);
    KG_PROCESS_ERROR(0 == nRetCode);
    fp = NULL;

Exit1:
    nResult = true;
Exit0:
    return nResult;
}

KG_File::KG_File() : m_hFile(NULL), m_bBinaryFile(false)
{
}

KG_File::~KG_File()
{
    Close();
}

int KG_File::Open(const char * const cszFile, const char * const cszMode)
{
    int nResult  = false;
    int nRetCode = false;

    m_bBinaryFile = false;

    nRetCode = KG_OpenFileSafely(m_hFile, cszFile, cszMode);
    KG_PROCESS_ERROR(nRetCode);

    if (::strchr(cszMode, 'b'))
    {
        m_bBinaryFile = true;
    }

    nResult = true;
Exit0:
    return nResult;
}

int KG_File::Flush()
{
    int nResult  = false;
    int nRetCode = 0;

    KG_PROCESS_PTR_ERROR(m_hFile);
    nRetCode = ::fflush(m_hFile);
    KG_PROCESS_ERROR(0 == nRetCode);

    nResult = true;
Exit0:
    return nResult;
}

int KG_File::Close()
{
    int nResult  = false;
    int nRetCode = false;

    KG_PROCESS_SUCCESS(NULL == m_hFile);

    nRetCode = KG_CloseFileSafely(m_hFile);
    KG_PROCESS_ERROR(nRetCode);

Exit1:
    nResult = true;
Exit0:
    return nResult;
}

LONG KG_File::Size() const
{
    LONG lResult  = -1;
    int  nRetCode = 0;
    LONG lCurPos  = -1;
    LONG lOriPos  = -1;

    KG_PROCESS_PTR_ERROR(m_hFile);

    lOriPos = ::ftell(m_hFile);
    KG_PROCESS_ERROR(-1 != lOriPos);

    nRetCode = ::fseek(m_hFile, 0, SEEK_END);
    KG_PROCESS_ERROR(0 == nRetCode);

    lCurPos = ::ftell(m_hFile);
    KG_PROCESS_ERROR(-1 != lCurPos);

    nRetCode = ::fseek(m_hFile, lOriPos, SEEK_SET);
    KG_PROCESS_ERROR(0 == nRetCode);

    lResult = lCurPos;
Exit0:
    return lResult;
}

LONG KG_File::Tell() const
{
    LONG lResult = -1;
    LONG lCurPos = -1;

    KG_PROCESS_PTR_ERROR(m_hFile);

    lCurPos = ::ftell(m_hFile);
    KG_PROCESS_ERROR(-1 != lCurPos);

    lResult = lCurPos;
Exit0:
    return lResult;
}

int KG_File::IsEof() const
{
    int nResult  = false;
    int nRetCode = 0;

    KG_PROCESS_PTR_ERROR(m_hFile);

    nRetCode = ::feof(m_hFile);
    KG_PROCESS_ERROR(0 != nRetCode);

    nResult = true;
Exit0:
    return nResult;
}

int KG_File::IsOpen() const
{
    return NULL != m_hFile;
}

int KG_File::SeekEnd(LONG lOffset)
{
    int  nResult  = false;
    int  nRetCode = 0;
    LONG lSize    = -1;

    KG_PROCESS_PTR_ERROR(m_hFile);
    KG_PROCESS_ERROR(lOffset >= 0);

    lSize = Size();
    KG_PROCESS_ERROR(lOffset <= lSize);

    nRetCode = ::fseek(m_hFile, -lOffset, SEEK_END);
    KG_PROCESS_ERROR(0 == nRetCode);

    nResult = true;
Exit0:
    return nResult;
}

int KG_File::SeekBegin(LONG lOffset)
{
    int  nResult  = false;
    int  nRetCode = 0;
    LONG lSize    = -1;

    KG_PROCESS_PTR_ERROR(m_hFile);
    KG_PROCESS_ERROR(lOffset >= 0);

    lSize = Size();
    KG_PROCESS_ERROR(lOffset <= lSize);

    nRetCode = ::fseek(m_hFile, lOffset, SEEK_SET);
    KG_PROCESS_ERROR(0 == nRetCode);

    nResult = true;
Exit0:
    return nResult;
}

int KG_File::SeekCurrent(LONG lOffset)
{
    int  nResult  = false;
    int  nRetCode = 0;
    LONG lSize    = -1;
    LONG lCurPos  = -1;

    KG_PROCESS_PTR_ERROR(m_hFile);
    KG_PROCESS_ERROR(lOffset >= 0);

    lSize = Size();
    KG_PROCESS_ERROR(-1 != lSize);

    lCurPos = Tell();
    KG_PROCESS_ERROR(-1 != lCurPos);
    KG_PROCESS_ERROR(lOffset <= lSize - lCurPos);

    nRetCode = ::fseek(m_hFile, lOffset, SEEK_CUR);
    KG_PROCESS_ERROR(0 == nRetCode);

    nResult = true;
Exit0:
    return nResult;
}

LONG KG_File::Read(char * const cpBuff, LONG lBuffSize, LONG lReadSize)
{
    LONG lResult    = -1;
    int  nRetCode   = 0;
    LONG lReadBytes = -1;

    KG_PROCESS_PTR_ERROR(cpBuff);
    KG_PROCESS_PTR_ERROR(m_hFile);
    KG_PROCESS_ERROR(lBuffSize > 0 && lReadSize > 0 && lReadSize <= lBuffSize);

    lReadBytes = ::fread(cpBuff, 1, lReadSize, m_hFile);
    nRetCode   = ::ferror(m_hFile);

    KG_PROCESS_ERROR(0 == nRetCode);
    KG_PROCESS_ERROR(lReadBytes <= lReadSize);

    lResult = lReadBytes;
Exit0:
    return lResult;
}

LONG KG_File::ReadTextLine(char * const cpBuff, LONG lBuffSize, LONG lReadSize)
{
    LONG   lResult    = -1;
    int    nRetCode   = 0;
    int    nCurPos    = 0;
    LONG   lReadBytes = -1;
    char * pTextLine  = NULL;
    char * pNewLine   = NULL;

    KG_PROCESS_PTR_ERROR(cpBuff);
    KG_PROCESS_PTR_ERROR(m_hFile);
    KG_PROCESS_ERROR(!m_bBinaryFile);
    KG_PROCESS_ERROR(lBuffSize > 0 && lReadSize > 0 && lReadSize <= lBuffSize);

    pTextLine = ::fgets(cpBuff, lReadSize, m_hFile);
    nRetCode  = ::ferror(m_hFile);

    KG_PROCESS_ERROR(0 == nRetCode);
    if (NULL == pTextLine || '\0' == pTextLine[0])
    { // empty string
        lReadBytes = 0;
        KG_PROCESS_SUCCESS(true);
    }

    // process '\r' and '\n' :  mac - '\r', unix - '\n', windows - '\r\n'
    lReadBytes = ::strnlen(pTextLine, lReadSize);
    nCurPos    = lReadBytes - 1;

    for (; nCurPos >= 0; nCurPos--)
    {
        if ('\r' != cpBuff[nCurPos] && '\n' != cpBuff[nCurPos])
        {
            break;
        }

        cpBuff[nCurPos] = '\0';
        lReadBytes--;
    }

Exit1:
    lResult = lReadBytes;
Exit0:
    return lResult;
}

LONG KG_File::Write(const char * const cpcBuff, LONG lBuffSize, LONG lWriteSize)
{
    LONG lResult     = -1;
    LONG lWriteBytes = -1;

    KG_PROCESS_PTR_ERROR(m_hFile);
    KG_PROCESS_PTR_ERROR(cpcBuff);
    KG_PROCESS_ERROR(lBuffSize > 0 && lWriteSize > 0 && lWriteSize <= lBuffSize);

    lWriteBytes = ::fwrite(cpcBuff, 1, lWriteSize, m_hFile);
    KG_PROCESS_ERROR(lWriteBytes <= lWriteSize);

    lResult = lWriteBytes;
Exit0:
    return lResult;
}

LONG KG_File::WriteLine(const char * const cpcBuff, LONG lBuffSize, LONG lWriteSize)
{
    LONG lResult     = -1;
    int  nRetCode    = 0;
    LONG lWriteBytes = -1;

    lWriteBytes = Write(cpcBuff, lBuffSize, lWriteSize);
    KG_PROCESS_ERROR(-1 != lWriteBytes);

    nRetCode = ::fprintf(m_hFile, "\r\n");
    KG_PROCESS_ERROR(nRetCode > 0);

    lResult = lWriteBytes;
Exit0:
    return lResult;
}

LONG KG_File::WriteTextLine(const char * const cszText, LONG lTextSize, LONG lWriteSize)
{
    LONG lResult     = -1;
    int  nRetCode    = 0;
    LONG lWriteBytes = -1;

    KG_PROCESS_PTR_ERROR(m_hFile);
    KG_PROCESS_PTR_ERROR(cszText);
    KG_PROCESS_ERROR(!m_bBinaryFile);
    KG_PROCESS_ERROR(lTextSize > 0 && lWriteSize > 0 && lWriteSize <= lTextSize);

    lWriteBytes = ::fwrite(cszText, 1, lWriteSize, m_hFile);
    KG_PROCESS_ERROR(lWriteBytes <= lWriteSize);

    nRetCode = ::fprintf(m_hFile, "\r\n");
    KG_PROCESS_ERROR(nRetCode > 0);

    lResult = lWriteBytes;
Exit0:
    return lResult;
}

KG_IniFile::KG_IniFile()
{
    m_nFileBuffOffset = 0;
    m_pLatestSection  = NULL;
}

bool KG_IniFile::IsKeyNameChar(char c) const
{
    if ( (c >= 'A' && c <= 'Z') ||
         (c >= 'a' && c <= 'z') ||
         (c >= '0' && c <= '9') ||
         (c == '$')             ||
         (c == '.')             ||
         (c == '_')             ||
         (c == '-'))
    {
        return true;
    }
    return false;
}

char *KG_IniFile::SplitKeyValue(char *szLine) const
{
    char *pResult = NULL;
    char *pIter   = szLine;

    while (*pIter)
    {
        if ('=' == *pIter)
        {
            break;
        }

        pIter++;
    }

    KG_PROCESS_ERROR(*pIter && "[Error] It seems 'key=value' line lacks of '=' character!");

    *pIter  = '\0';                                                     // '=' -> '\0'
    pResult = pIter + 1;
Exit0:
    return pResult;
}

bool KG_IniFile::CreateIniLink(char *pBuff, int nBuffSize)
{ // lBufferSize = lFileSize
    bool  bResult                      = false;
    int   nRetCode                     = 0;
    char *pStrLine                     = NULL;
    char *pKeyName                     = NULL;
    char *pKeyValue                    = NULL;
    char  pSecName[KG_MAX_INI_SEC_LEN] = "[MAIN]";

    KG_PROCESS_PTR_ERROR(pBuff);

    m_nFileBuffOffset = 0;
    while (m_nFileBuffOffset < nBuffSize)
    {
        pStrLine = &pBuff[m_nFileBuffOffset];

        nRetCode = GetLineOfBuff(pBuff, nBuffSize);
        if (!nRetCode)
        {
            break;
        }

        //nRetCode = KG_StrTrimBlank(szStrLine, 1);                       // 去掉行头的空白符，防止是空白符影响下面的判断
        //KG_ASSERT(nRetCode);

        if (IsKeyNameChar(pStrLine[0]))
        { // key line
            pKeyName  = pStrLine;
            pKeyValue = SplitKeyValue(pStrLine);

            //nRetCode = KG_StrTrimBlank(szKeyName, 2);                   // 去掉键名的尾部空白符
            //KG_ASSERT(nRetCode);

            SetKeyValue(pSecName, pKeyName, pKeyValue);
        }

        if ('[' == pStrLine[0])
        { // section line
            KG_Strncpy(pSecName, pStrLine, ::strlen(pStrLine));
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

bool KG_IniFile::GetLineOfBuff(char *pBuff, int nBuffSize)
{ // nBuffSize = nFileSize
    bool bResult    = false;
    int  nCurOffset = 0;

    KG_PROCESS_PTR_ERROR(pBuff);
    KG_PROCESS_ERROR(nBuffSize > 0);
    KG_PROCESS_ERROR(m_nFileBuffOffset < nBuffSize);

    while (0x0D != pBuff[m_nFileBuffOffset] && 0x0A != pBuff[m_nFileBuffOffset])
    { // '\r' = 0x0D '\n' = 0x0A
        m_nFileBuffOffset++;
        if (m_nFileBuffOffset >= nBuffSize)
        {
            break;
        }
    }
    nCurOffset = m_nFileBuffOffset;

    if (0x0D == pBuff[m_nFileBuffOffset] && 0x0A == pBuff[m_nFileBuffOffset + 1])
    { // "\r\n"
        m_nFileBuffOffset += 2;
    }
    else
    { // "\n"
        m_nFileBuffOffset += 1;
    }

    pBuff[nCurOffset] = '\0';

    bResult = true;
Exit0:
    return bResult;
}

bool KG_IniFile::FormatSecName(char *pBuff, int nBuffSize, const char *pszSecName) const
{
    bool bResult = false;
    int  nSrcLen = 0;
    int  nDstLen = 0;

    KG_PROCESS_PTR_ERROR(pBuff);
    KG_PROCESS_C_STR_ERROR(pszSecName);
    KG_PROCESS_ERROR(nBuffSize >= KG_MAX_INI_SEC_LEN);

    pBuff[0] = '\0';
    nSrcLen  = (int)::strlen(pszSecName);

    if ('[' != pszSecName[0])
    {
        pBuff[nDstLen++] = '[';
        pBuff[nDstLen]   = '\0';
    }

    KG_PROCESS_ERROR(nDstLen + nSrcLen < nBuffSize);

    ::strcat(pBuff, pszSecName);
    nDstLen += nSrcLen;

    if (']' != pBuff[nDstLen - 1])
    {
        KG_PROCESS_ERROR(nDstLen + 1 < nBuffSize);
        pBuff[nDstLen++] = ']';
        pBuff[nDstLen]   = '\0';
    }

    bResult = true;
Exit0:
    return bResult;
}

bool KG_IniFile::ReverseSecName(char *pBuff, int nBuffSize, const char *pszSecName) const
{
    bool        bResult = false;
    int         nSrcLen = 0;
    const char *pIter   = pszSecName;

    KG_PROCESS_PTR_ERROR(pBuff);
    KG_PROCESS_C_STR_ERROR(pszSecName);
    KG_PROCESS_ERROR(nBuffSize >= KG_MAX_INI_SEC_LEN);

    pBuff[0] = '\0';
    nSrcLen  = (int)::strlen(pszSecName);

    if ('[' == pszSecName[0])
    {
        pIter++;
        nSrcLen--;
    }

    KG_PROCESS_ERROR(nSrcLen < nBuffSize);
    ::strcat(pBuff, pIter);

    if (nSrcLen > 0 && ']' == pBuff[nSrcLen - 1])
    {
        pBuff[nSrcLen - 1] = '\0';
    }

    bResult = true;
Exit0:
    return bResult;
}

bool KG_IniFile::GetKeyValue(const char *pszSecName, const char *pszKeyName, char *pKeyValueBuff, int nBuffLen)
{
    bool               bResult        = false;
    int                nRetCode       = 0;
    int                nLen           = 0;
    DWORD              dwId           = KG_INVALID_STR2ID;
    PKG_IniFileSecNode pSecNode       = NULL;
    PKG_IniFileKeyNode pKeyNode       = NULL;
    char pSecName[KG_MAX_INI_SEC_LEN] = {'\0'};

    KG_PROCESS_C_STR_ERROR(pszSecName);
    KG_PROCESS_C_STR_ERROR(pszKeyName);
    KG_PROCESS_PTR_ERROR(pKeyValueBuff);
    KG_PROCESS_ERROR(nBuffLen > 0);

    // format section name
    nRetCode = FormatSecName(pSecName, sizeof(pSecName), pszSecName);
    KG_PROCESS_ERROR(nRetCode);

    dwId = KG_KSGStringHash(pSecName);                                  // section id => uId

    // search for the matched section
    pSecNode = m_RootSection.m_pNext;
    if (NULL != m_pLatestSection && dwId == m_pLatestSection->m_dwId)
    {
        pSecNode = m_pLatestSection;
    }
    else
    {
        while (NULL != pSecNode)
        {
            if (dwId == pSecNode->m_dwId)
            {
                m_pLatestSection = pSecNode;
                break;
            }

            pSecNode = pSecNode->m_pNext;
        }
    }
    KG_PROCESS_PTR_ERROR(pSecNode);                                     // if no such section founded

    // search for the same key
    dwId      = KG_KSGStringHash(pszKeyName);                           // key id => uId
    pKeyNode = pSecNode->m_RootKey.m_pNext;

    while (NULL != pKeyNode)
    {
        if (dwId == pKeyNode->m_dwId)
        {
            break;
        }
        pKeyNode = pKeyNode->m_pNext;
    }

    KG_PROCESS_PTR_ERROR(pKeyNode);                                     // if no such key found

    // copy the value of the key
    KG_PROCESS_PTR_ERROR(pKeyNode->m_pValue);

    nLen = (int)::strlen(pKeyNode->m_pValue);
    KG_PROCESS_ERROR(nLen < nBuffLen);                                  // Include '\0'
    KG_Strncpy(pKeyValueBuff, pKeyNode->m_pValue, nLen);

    bResult = true;
Exit0:
    return true;
}

bool KG_IniFile::SetKeyValue(const char *pszSecName, const char *pszKeyName, const char *pszKeyValue)
{
    bool               bResult        = false;
    int                nRetCode       = 0;
    int                nLen           = 0;
    DWORD              dwId           = KG_INVALID_STR2ID;
    PKG_IniFileSecNode pThisSecNode   = NULL;
    PKG_IniFileSecNode pNextSecNode   = NULL;
    PKG_IniFileKeyNode pThisKeyNode   = NULL;
    PKG_IniFileKeyNode pNextKeyNode   = NULL;
    char pSecName[KG_MAX_INI_SEC_LEN] = {'\0'};

    KG_PROCESS_PTR_ERROR(pszKeyValue);                                  // key value can be empty string.
    KG_PROCESS_C_STR_ERROR(pszSecName);
    KG_PROCESS_C_STR_ERROR(pszKeyName);

    // format section name
    nRetCode = FormatSecName(pSecName, sizeof(pSecName), pszSecName);
    KG_PROCESS_ERROR(nRetCode);

    dwId = KG_KSGStringHash(pSecName);                                  // section id => uId

    // search for the matched section
    if (NULL != m_pLatestSection && dwId == m_pLatestSection->m_dwId)
    {
        pThisSecNode = NULL;                                            //unuse
        pNextSecNode = m_pLatestSection;
    }
    else
    {
        pThisSecNode = &m_RootSection;
        pNextSecNode = pThisSecNode->m_pNext;

        while (NULL != pNextSecNode)
        {
            if (dwId == pNextSecNode->m_dwId)
            {
                m_pLatestSection = pNextSecNode;
                break;
            }

            pThisSecNode = pNextSecNode;                                // next => this
            pNextSecNode = pThisSecNode->m_pNext;                       // next->next => next
        }
    }

    // if no such section found, create a new section
    if (NULL == pNextSecNode)
    {
        pNextSecNode = new KG_IniFileSecNode;
        KG_PROCESS_PTR_ERROR(pNextSecNode);

        pNextSecNode->m_dwId = dwId;

        nLen = ::strlen(pSecName);
        pNextSecNode->m_pName = ::new char[nLen + 1];                   // Include '\0'
        KG_Strncpy(pNextSecNode->m_pName, pSecName, nLen);

        pNextSecNode->m_RootKey.m_pNext = NULL;                         // m_RootKey
        pNextSecNode->m_pNext           = NULL;                         // m_pNext
        pThisSecNode->m_pNext           = pNextSecNode;                 // into link
        m_pLatestSection                = pNextSecNode;                 // change m_pLatestSection
    }

    // search for the same key
    pThisKeyNode = &pNextSecNode->m_RootKey;
    pNextKeyNode = pThisKeyNode->m_pNext;
    dwId = KG_KSGStringHash(pszKeyName);                                // key id => uId

    while (NULL != pNextKeyNode)
    {
        if (dwId == pNextKeyNode->m_dwId)
        {
            break;
        }

        pThisKeyNode = pNextKeyNode;
        pNextKeyNode = pThisKeyNode->m_pNext;
    }

    // if no such key found create a new key
    if (NULL == pNextKeyNode)
    {
        pNextKeyNode = new KG_IniFileKeyNode;
        KG_PROCESS_PTR_ERROR(pNextKeyNode);

        pNextKeyNode->m_dwId = dwId;                                    // m_uId

        nLen = ::strlen(pszKeyName);
        pNextKeyNode->m_pName = ::new char[nLen + 1];                   // Include '\0'
        KG_Strncpy(pNextKeyNode->m_pName, pszKeyName, nLen);
        pNextKeyNode->m_pValue = NULL;
        pNextKeyNode->m_pNext  = NULL;
        pThisKeyNode->m_pNext  = pNextKeyNode;                          // into link
    }

    // replace the old value with new
    KG_DeleteArrayPtrSafely(pNextKeyNode->m_pValue);
    nLen = ::strlen(pszKeyValue);
    pNextKeyNode->m_pValue = ::new char[nLen + 1];                     // Include '\0'
    KG_Strncpy(pNextKeyNode->m_pValue, pszKeyValue, nLen);

    bResult = true;
Exit0:
    return bResult;
}

KG_NAMESPACE_END
