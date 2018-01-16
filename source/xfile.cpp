#include "xhash.h"
#include "xfile.h"
#include "xdebug.h"
#include "xstring.h"
#include "xpointer.h"

# pragma warning(disable: 4996)

#ifdef KG_PLATFORM_WINDOWS                                              // windows platform

const char l_pszFileNewLine[3] = "\r\n";
const int  l_nFileNewLineLen   = 2;

#else                                                                   // linux platform

const char l_pszFileNewLine[2] = "\n";
const int  l_nFileNewLineLen = 1;

#endif // KG_PLATFORM_WINDOWS

KG_NAMESPACE_BEGIN(xzero)

bool KG_OpenFileSafely(FILE *&fp, const char *pszFile, const char *pszMode)
{
    bool bResult = false;

    KG_PROCESS_C_STR_ERROR(pszFile);
    KG_PROCESS_C_STR_ERROR(pszMode);

    KG_PROCESS_ERROR(NULL == fp && "[ERROR] File handle has been used!");
    fp = ::fopen(pszFile, pszMode);
    KG_PROCESS_ERROR(NULL != fp && "[ERROR] Open file failed!");

    bResult = true;
Exit0:
    return bResult;
}

bool KG_CloseFileSafely(FILE *&fp)
{
    bool bResult  = false;
    int  nRetCode = 0;

    KG_PROCESS_SUCCESS(NULL == fp);

    nRetCode = ::fclose(fp);
    KG_PROCESS_ERROR(0 == nRetCode);
    fp = NULL;

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

KG_File::KG_File() : m_hFile(NULL), m_bBinaryFile(false)
{
}

KG_File::~KG_File()
{
    Close();
}

bool KG_File::Open(const char *pszFile, const char *pszMode)
{
    bool bResult  = false;
    int  nRetCode = false;

    m_bBinaryFile = false;

    nRetCode = KG_OpenFileSafely(m_hFile, pszFile, pszMode);
    KG_PROCESS_ERROR(nRetCode);

    if (::strchr(pszMode, 'b'))
    {
        m_bBinaryFile = true;
    }

    bResult = true;
Exit0:
    return bResult;
}

bool KG_File::Flush()
{
    bool bResult  = false;
    int  nRetCode = 0;

    KG_PROCESS_PTR_ERROR(m_hFile);
    nRetCode = ::fflush(m_hFile);
    KG_PROCESS_ERROR(0 == nRetCode);

    bResult = true;
Exit0:
    return bResult;
}

bool KG_File::Close()
{
    bool bResult  = false;
    int  nRetCode = false;

    KG_PROCESS_SUCCESS(NULL == m_hFile);

    nRetCode = KG_CloseFileSafely(m_hFile);
    KG_PROCESS_ERROR(nRetCode);

Exit1:
    bResult = true;
Exit0:
    return bResult;
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

bool KG_File::IsEof() const
{
    bool bResult  = false;
    int  nRetCode = 0;

    KG_PROCESS_PTR_ERROR(m_hFile);

    nRetCode = ::feof(m_hFile);
    KG_PROCESS_ERROR(0 != nRetCode);

    bResult = true;
Exit0:
    return bResult;
}

bool KG_File::IsOpen() const
{
    return NULL != m_hFile;
}

bool KG_File::SeekEnd(LONG lOffset)
{
    bool bResult  = false;
    int  nRetCode = 0;
    LONG lSize    = -1;

    KG_PROCESS_PTR_ERROR(m_hFile);
    KG_PROCESS_ERROR(lOffset >= 0);

    lSize = Size();
    KG_PROCESS_ERROR(lOffset <= lSize);

    nRetCode = ::fseek(m_hFile, -lOffset, SEEK_END);
    KG_PROCESS_ERROR(0 == nRetCode);

    bResult = true;
Exit0:
    return bResult;
}

bool KG_File::SeekBegin(LONG lOffset)
{
    bool bResult  = false;
    int  nRetCode = 0;
    LONG lSize    = -1;

    KG_PROCESS_PTR_ERROR(m_hFile);
    KG_PROCESS_ERROR(lOffset >= 0);

    lSize = Size();
    KG_PROCESS_ERROR(lOffset <= lSize);

    nRetCode = ::fseek(m_hFile, lOffset, SEEK_SET);
    KG_PROCESS_ERROR(0 == nRetCode);

    bResult = true;
Exit0:
    return bResult;
}

bool KG_File::SeekCurrent(LONG lOffset)
{
    bool bResult  = false;
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

    bResult = true;
Exit0:
    return bResult;
}

LONG KG_File::Read(char *pBuff, LONG lBuffSize, LONG lReadSize)
{
    LONG lResult    = -1;
    int  nRetCode   = 0;
    LONG lReadBytes = -1;

    KG_PROCESS_PTR_ERROR(pBuff);
    KG_PROCESS_PTR_ERROR(m_hFile);
    KG_PROCESS_ERROR(lBuffSize > 0 && lReadSize > 0 && lReadSize <= lBuffSize);

    lReadBytes = (LONG)::fread(pBuff, 1, lReadSize, m_hFile);
    nRetCode   = ::ferror(m_hFile);

    KG_PROCESS_ERROR(0 == nRetCode);
    KG_PROCESS_ERROR(lReadBytes <= lReadSize);

    lResult = lReadBytes;
Exit0:
    return lResult;
}

LONG KG_File::ReadTextLine(char *pBuff, LONG lBuffSize, LONG lReadSize)
{
    LONG   lResult    = -1;
    int    nRetCode   = 0;
    int    nCurPos    = 0;
    LONG   lReadBytes = -1;
    char * pTextLine  = NULL;
    char * pNewLine   = NULL;

    KG_PROCESS_PTR_ERROR(pBuff);
    KG_PROCESS_PTR_ERROR(m_hFile);
    KG_PROCESS_ERROR(!m_bBinaryFile);
    KG_PROCESS_ERROR(lBuffSize > 0 && lReadSize > 0 && lReadSize <= lBuffSize);

    pTextLine = ::fgets(pBuff, lReadSize, m_hFile);
    nRetCode  = ::ferror(m_hFile);

    KG_PROCESS_ERROR(0 == nRetCode);
    if (NULL == pTextLine || '\0' == pTextLine[0])
    { // empty string
        lReadBytes = 0;
        KG_PROCESS_SUCCESS(true);
    }

    // process '\r' and '\n' :  mac - '\r', unix - '\n', windows - '\r\n'
    lReadBytes = (LONG)::strnlen(pTextLine, lReadSize);
    nCurPos    = lReadBytes - 1;

    for (; nCurPos >= 0; nCurPos--)
    {
        if ('\r' != pBuff[nCurPos] && '\n' != pBuff[nCurPos])
        {
            break;
        }

        pBuff[nCurPos] = '\0';
        lReadBytes--;
    }

Exit1:
    lResult = lReadBytes;
Exit0:
    return lResult;
}

LONG KG_File::Write(const char *pBuff, LONG lBuffSize, LONG lWriteSize)
{
    LONG lResult     = -1;
    LONG lWriteBytes = -1;

    KG_PROCESS_PTR_ERROR(m_hFile);
    KG_PROCESS_PTR_ERROR(pBuff);
    KG_PROCESS_ERROR(lBuffSize > 0 && lWriteSize > 0 && lWriteSize <= lBuffSize);

    lWriteBytes = (LONG)::fwrite(pBuff, 1, lWriteSize, m_hFile);
    KG_PROCESS_ERROR(lWriteBytes <= lWriteSize);

    lResult = lWriteBytes;
Exit0:
    return lResult;
}

LONG KG_File::WriteLine(const char *pBuff, LONG lBuffSize, LONG lWriteSize)
{
    LONG lResult     = -1;
    int  nRetCode    = 0;
    LONG lWriteBytes = -1;

    lWriteBytes = Write(pBuff, lBuffSize, lWriteSize);
    KG_PROCESS_ERROR(-1 != lWriteBytes);

    nRetCode = ::fprintf(m_hFile, "\r\n");
    KG_PROCESS_ERROR(nRetCode > 0);

    lResult = lWriteBytes;
Exit0:
    return lResult;
}

LONG KG_File::WriteTextLine(const char *pszText, LONG lTextSize, LONG lWriteSize)
{
    LONG lResult     = -1;
    int  nRetCode    = 0;
    LONG lWriteBytes = -1;

    KG_PROCESS_PTR_ERROR(m_hFile);
    KG_PROCESS_PTR_ERROR(pszText);
    KG_PROCESS_ERROR(!m_bBinaryFile);
    KG_PROCESS_ERROR(lTextSize > 0 && lWriteSize > 0 && lWriteSize <= lTextSize);

    lWriteBytes = (LONG)::fwrite(pszText, 1, lWriteSize, m_hFile);
    KG_PROCESS_ERROR(lWriteBytes <= lWriteSize);

    nRetCode = ::fprintf(m_hFile, "\r\n");
    KG_PROCESS_ERROR(nRetCode > 0);

    lResult = lWriteBytes;
Exit0:
    return lResult;
}

LONG KG_File::WriteFormat(const char *pszFormat, ...)
{
    LONG lResult     = -1;
    LONG lWriteBytes = -1;

    KG_PROCESS_PTR_ERROR(m_hFile);

    va_list va;
    va_start(va, pszFormat);
    lWriteBytes = ::vfprintf(m_hFile, pszFormat, va);
    va_end(va);
    KG_PROCESS_ERROR(lWriteBytes >= 0);

    lResult = lWriteBytes;
Exit0:
    return lResult;
}

KG_IniFile::KG_IniFile()
{
    m_lFileBuffOffset = 0;
    m_pLatestSection  = NULL;
}

KG_IniFile::~KG_IniFile()
{
    Release();
}

bool KG_IniFile::Load(KG_File *pFile)
{
    bool  bResult   = false;
    int   nRetCode  = false;
    LONG  lFileSize = 0;
    LONG  lReadSize = 0;
    char *pBuf      = NULL;

    KG_PROCESS_PTR_ERROR(pFile);

    nRetCode = pFile->IsOpen();
    KG_PROCESS_ERROR(nRetCode);

    lFileSize = pFile->Size();
    KG_PROCESS_ERROR(lFileSize > 0);

    pBuf = ::new char[lFileSize + 4];
    KG_PROCESS_PTR_ERROR(pBuf);

    lReadSize = pFile->Read(pBuf, lFileSize + 4, lFileSize);
    KG_PROCESS_ERROR(lReadSize == lFileSize);

    Release();
    CreateIniLink(pBuf, lFileSize);

    bResult = true;
Exit0:
    KG_DeleteArrayPtrSafely(pBuf);
    return bResult;
}

bool KG_IniFile::Save(KG_File *pFile)
{
    bool               bResult    = false;
    int                nRetCode   = false;
    LONG               lWriteSize = 0;
    int                nLen       = 0;
    PKG_IniFileSecNode pSecNode   = m_RootSection.m_pNext;
    PKG_IniFileKeyNode pKeyNode   = NULL;

    KG_PROCESS_PTR_ERROR(pFile);

    nRetCode = pFile->IsOpen();
    KG_PROCESS_ERROR(nRetCode);

    while (NULL != pSecNode)
    {
        nLen       = (int)::strlen(pSecNode->m_pName);
        lWriteSize = pFile->Write(pSecNode->m_pName, nLen, nLen);
        KG_PROCESS_ERROR(lWriteSize == nLen);

        lWriteSize = pFile->Write(l_pszFileNewLine, l_nFileNewLineLen, l_nFileNewLineLen);
        KG_PROCESS_ERROR(lWriteSize == l_nFileNewLineLen);

        pKeyNode = pSecNode->m_RootKey.m_pNext;
        while (NULL != pKeyNode)
        {
            nLen       = (int)::strlen(pKeyNode->m_pName);
            lWriteSize = pFile->Write(pKeyNode->m_pName, nLen, nLen);
            KG_PROCESS_ERROR(lWriteSize == nLen);

            lWriteSize = pFile->Write("=", 1, 1);
            KG_PROCESS_ERROR(lWriteSize == 1);

            nLen       = (int)::strlen(pKeyNode->m_pValue);
            lWriteSize = pFile->Write(pKeyNode->m_pValue, nLen, nLen);
            KG_PROCESS_ERROR(lWriteSize == nLen);

            lWriteSize = pFile->Write(l_pszFileNewLine, l_nFileNewLineLen, l_nFileNewLineLen);
            KG_PROCESS_ERROR(lWriteSize == l_nFileNewLineLen);

            pKeyNode = pKeyNode->m_pNext;
        }

        lWriteSize = pFile->Write(l_pszFileNewLine, l_nFileNewLineLen, l_nFileNewLineLen);
        KG_PROCESS_ERROR(lWriteSize == l_nFileNewLineLen);

        pSecNode = pSecNode->m_pNext;
    }

    bResult = true;
Exit0:
    return bResult;
}

bool KG_IniFile::Load(const char *pszFilePath)
{
    bool    bResult  = 0;
    int     nRetCode = false;
    KG_File sFile;

    KG_PROCESS_C_STR_ERROR(pszFilePath);

    nRetCode = sFile.Open(pszFilePath, "rb");
    KG_PROCESS_ERROR(nRetCode);

    nRetCode = Load(&sFile);
    KG_PROCESS_ERROR(nRetCode);

    bResult = true;
Exit0:
    nRetCode = sFile.Close();
    KG_ASSERT(nRetCode);

    return bResult;
}

bool KG_IniFile::Save(const char *pszFilePath)
{
    bool    bResult  = false;
    int     nRetCode = false;
    KG_File sFile;

    KG_PROCESS_C_STR_ERROR(pszFilePath);

    nRetCode = sFile.Open(pszFilePath, "wb");
    KG_PROCESS_ERROR(nRetCode);

    nRetCode = Save(&sFile);
    KG_PROCESS_ERROR(nRetCode);

    bResult = true;
Exit0:
    nRetCode = sFile.Close();
    KG_ASSERT(nRetCode);

    return bResult;
}

void KG_IniFile::Release()
{
    PKG_IniFileSecNode pNextSec = &m_RootSection;
    PKG_IniFileSecNode pThisSec = pNextSec->m_pNext;
    PKG_IniFileKeyNode pNextKey = NULL;
    PKG_IniFileKeyNode pThisKey = NULL;

    while (NULL != pThisSec)
    {
        pNextSec = pThisSec->m_pNext;
        pNextKey = &pThisSec->m_RootKey;
        pThisKey = pNextKey->m_pNext;

        while (NULL != pThisKey)
        {
            pNextKey = pThisKey->m_pNext;
            KG_DeleteArrayPtrSafely(pThisKey->m_pName);
            KG_DeleteArrayPtrSafely(pThisKey->m_pValue);
            KG_DeletePtrSafely(pThisKey);
            pThisKey = pNextKey;
        }

        KG_DeleteArrayPtrSafely(pThisSec->m_pName);
        KG_DeletePtrSafely(pThisSec);
        pThisSec = pNextSec;
    }

    m_RootSection.m_pNext = NULL;
    m_pLatestSection      = NULL;
}

bool KG_IniFile::IsSecExisted(const char *pszSecName)
{
    bool               bResult         = false;
    int                nRetCode        = 0;
    DWORD              dwId            = KG_INVALID_STR2ID;
    PKG_IniFileSecNode pSecNode        = m_RootSection.m_pNext;
    char szSection[KG_MAX_INI_SEC_LEN] = { '\0' };

    KG_PROCESS_C_STR_ERROR(pszSecName);

    nRetCode = FormatSecName(szSection, sizeof(szSection), pszSecName);
    KG_PROCESS_ERROR(nRetCode);

    dwId = KG_KSGStringHash(szSection);
    KG_PROCESS_SUCCESS(NULL != m_pLatestSection && dwId == m_pLatestSection->m_dwId);

    while (NULL != pSecNode)
    {
        if (dwId == pSecNode->m_dwId)
        {
            m_pLatestSection = pSecNode;
            break;
        }

        pSecNode = pSecNode->m_pNext;
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

bool KG_IniFile::IsKeyExisted(const char *pszSecName, const char *pszKeyName)
{
    bool               bResult         = false;
    int                nRetCode        = 0;
    DWORD              dwId            = KG_INVALID_STR2ID;
    PKG_IniFileSecNode pSecNode        = NULL;
    PKG_IniFileKeyNode pKeyNode        = NULL;
    char szSection[KG_MAX_INI_SEC_LEN] = { '\0' };

    KG_PROCESS_C_STR_ERROR(pszSecName);
    KG_PROCESS_C_STR_ERROR(pszKeyName);

    nRetCode = FormatSecName(szSection, sizeof(szSection), pszSecName);
    KG_PROCESS_ERROR(nRetCode);

    // get proper section
    dwId = KG_KSGStringHash(szSection);
    if (NULL != m_pLatestSection && dwId == m_pLatestSection->m_dwId)
    {
        pSecNode = m_pLatestSection;
        goto Exit2;
    }

    pSecNode = m_RootSection.m_pNext;
    while (NULL != pSecNode)
    {
        if (dwId == pSecNode->m_dwId)
        {
            m_pLatestSection = pSecNode;
            break;
        }

        pSecNode = pSecNode->m_pNext;
    }

Exit2:
    KG_PROCESS_PTR_ERROR(pSecNode);

    dwId     = KG_KSGStringHash(pszKeyName);
    pKeyNode = pSecNode->m_RootKey.m_pNext;
    while (NULL != pKeyNode)
    {
        if (dwId == pKeyNode->m_dwId)
        {
            break;
        }

        pKeyNode = pKeyNode->m_pNext;
    }

    bResult = true;
Exit0:
    return bResult;
}

bool KG_IniFile::GetNextSec(const char *pszSecName, char *pNextSecBuff, int nBuffLen)
{
    bool               bResult         = false;
    int                nRetCode        = 0;
    DWORD              dwId            = KG_INVALID_STR2ID;
    PKG_IniFileSecNode pSecNode        = NULL;
    char szSection[KG_MAX_INI_SEC_LEN] = { '\0' };

    KG_PROCESS_PTR_ERROR(pNextSecBuff);
    KG_PROCESS_ERROR(nBuffLen >= KG_MAX_INI_SEC_LEN);

    pNextSecBuff[0] = '\0';

    if (NULL == pszSecName || '\0' == pszSecName[0])
    { // get first section name
        pSecNode = m_RootSection.m_pNext;
        KG_PROCESS_PTR_ERROR(pSecNode);

        nRetCode = ReverseSecName(pNextSecBuff, nBuffLen, pSecNode->m_pName);
        KG_PROCESS_ERROR(nRetCode);
        KG_PROCESS_SUCCESS(true);
    }

    nRetCode = FormatSecName(szSection, sizeof(szSection), pszSecName);
    KG_PROCESS_ERROR(nRetCode);

    dwId = KG_KSGStringHash(szSection);
    if (NULL != m_pLatestSection && dwId == m_pLatestSection->m_dwId)
    {
        KG_PROCESS_PTR_ERROR_Q(m_pLatestSection->m_pNext);
        nRetCode = ReverseSecName(pNextSecBuff, nBuffLen, m_pLatestSection->m_pNext->m_pName);
        KG_PROCESS_ERROR(nRetCode);
        KG_PROCESS_SUCCESS(true);
    }

    pSecNode = m_RootSection.m_pNext;
    while (NULL != pSecNode)
    {
        if (dwId == pSecNode->m_dwId)
        {
            m_pLatestSection = pSecNode;
            break;
        }

        pSecNode = pSecNode->m_pNext;
    }

    KG_PROCESS_PTR_ERROR_Q(pSecNode);
    KG_PROCESS_PTR_ERROR_Q(pSecNode->m_pNext);

    nRetCode = ReverseSecName(pNextSecBuff, nBuffLen, pSecNode->m_pNext->m_pName);
    KG_PROCESS_ERROR(nRetCode);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

bool KG_IniFile::GetNextKey(const char *pszSecName, const char *pszKeyName, char *pNextKeyBuff, int nBuffLen)
{
    bool               bResult         = false;
    int                nRetCode        = 0;
    DWORD              dwId            = KG_INVALID_STR2ID;
    PKG_IniFileSecNode pSecNode        = NULL;
    PKG_IniFileKeyNode pKeyNode        = NULL;
    char szSection[KG_MAX_INI_SEC_LEN] = { '\0' };

    KG_PROCESS_PTR_ERROR(pNextKeyBuff);
    KG_PROCESS_ERROR(nBuffLen >= KG_MAX_INI_SEC_LEN);

    pNextKeyBuff[0] = '\0';

    // get proper section
    if (NULL == pszSecName || '\0' == pszSecName[0])
    {
        pSecNode = m_RootSection.m_pNext;
        goto Exit2;
    }

    nRetCode = FormatSecName(szSection, sizeof(szSection), pszSecName);
    KG_PROCESS_ERROR(nRetCode);
    dwId     = KG_KSGStringHash(szSection);

    if (NULL != m_pLatestSection && dwId == m_pLatestSection->m_dwId)
    {
        pSecNode = m_pLatestSection;
        goto Exit2;
    }

    pSecNode = m_RootSection.m_pNext;
    while (NULL != pSecNode)
    {
        if (dwId == pSecNode->m_dwId)
        {
            m_pLatestSection = pSecNode;
            break;
        }

        pSecNode = pSecNode->m_pNext;
    }


Exit2:
    KG_PROCESS_PTR_ERROR_Q(pSecNode);

    if (NULL == pszKeyName || '\0' == pszKeyName[0])
    { // get first key name
        pKeyNode = pSecNode->m_RootKey.m_pNext;
        KG_PROCESS_PTR_ERROR(pKeyNode);
        KG_PROCESS_C_STR_ERROR(pKeyNode->m_pName);
        ::strcpy(pNextKeyBuff, pKeyNode->m_pName);
        KG_PROCESS_SUCCESS(true);
    }

    pKeyNode = pSecNode->m_RootKey.m_pNext;
    dwId     = KG_KSGStringHash(pszKeyName);

    while (NULL != pKeyNode)
    {
        if (dwId == pKeyNode->m_dwId)
        {
            break;
        }

        pKeyNode = pKeyNode->m_pNext;
    }
    KG_PROCESS_PTR_ERROR_Q(pKeyNode);
    KG_PROCESS_PTR_ERROR_Q(pKeyNode->m_pNext);

    pKeyNode = pKeyNode->m_pNext;
    KG_PROCESS_C_STR_ERROR(pKeyNode->m_pName);
    ::strcpy(pNextKeyBuff, pKeyNode->m_pName);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

int KG_IniFile::GetSecCount()
{
    int nResult = 0;
    PKG_IniFileSecNode pSecNode = m_RootSection.m_pNext;

    while (NULL != pSecNode)
    {
        nResult++;
        pSecNode = pSecNode->m_pNext;
    }

    return nResult;
}

int KG_IniFile::GetKeyCount(const char *pszSecName)
{
    int                nResult         = 0;
    int                nRetCode        = 0;
    DWORD              dwId            = KG_INVALID_STR2ID;
    PKG_IniFileSecNode pSecNode        = NULL;
    PKG_IniFileKeyNode pKeyNode        = NULL;
    char szSection[KG_MAX_INI_SEC_LEN] = { '\0' };

    // get proper section
    if (NULL == pszSecName || '\0' == pszSecName[0])
    { // get first section
        pSecNode = m_RootSection.m_pNext;
        goto Exit2;
    }

    nRetCode = FormatSecName(szSection, sizeof(szSection), pszSecName);
    KG_PROCESS_ERROR(nRetCode);
    dwId     = KG_KSGStringHash(szSection);

    if (NULL != m_pLatestSection && dwId == m_pLatestSection->m_dwId)
    {
        pSecNode = m_pLatestSection;
        goto Exit2;
    }

    pSecNode = m_RootSection.m_pNext;
    while (NULL != pSecNode)
    {
        if (dwId == pSecNode->m_dwId)
        {
            m_pLatestSection = pSecNode;
            break;
        }

        pSecNode = pSecNode->m_pNext;
    }
Exit2:
    KG_PROCESS_PTR_ERROR_Q(pSecNode);

    pKeyNode = pSecNode->m_RootKey.m_pNext;
    while (NULL != pKeyNode)
    {
        nResult++;
        pKeyNode = pKeyNode->m_pNext;
    }

Exit0:
    return nResult;
}

bool KG_IniFile::GetStr(const char *pszSecName, const char *pszKeyName, const char *pszDefault, char * pKeyValueBuff, int nBuffLen)
{
    bool bResult  = false;
    int  nRetCode = 0;
    int  nLen     = 0;

    KG_PROCESS_PTR_ERROR(pszDefault);

    nRetCode = GetKeyValue(pszSecName, pszKeyName, pKeyValueBuff, nBuffLen);
    if (!nRetCode)
    {
        nLen = (int)::strlen(pszDefault);
        KG_PROCESS_ERROR(nLen < nBuffLen);
        KG_Strncpy(pKeyValueBuff, pszDefault, nLen);
        KG_PROCESS_ERROR_Q(false);
    }

    bResult = true;
Exit0:
    return bResult;
}

bool KG_IniFile::GetInt(const char *pszSecName, const char *pszKeyName, int nDefault, int *pnKeyValue)
{
    bool bResult    = false;
    int  nRetCode   = 0;
    char szBuff[64] = { '\0' };

    KG_PROCESS_PTR_ERROR(pnKeyValue);

    nRetCode = GetKeyValue(pszSecName, pszKeyName, szBuff, sizeof(szBuff));
    if (!nRetCode)
    {
        *pnKeyValue = nDefault;
        KG_PROCESS_ERROR_Q(false);
    }

    *pnKeyValue = ::atoi(szBuff);

    bResult = true;
Exit0:
    return bResult;
}

bool KG_IniFile::GetBool(const char *pszSecName, const char *pszKeyName, bool bDefault, bool *pbKeyValue)
{
    bool bResult      = false;
    int  nRetCode     = false;
    char szBuff[16] = { '\0' };

    KG_PROCESS_PTR_ERROR(pbKeyValue);

    nRetCode = GetKeyValue(pszSecName, pszKeyName, szBuff, sizeof(szBuff));
    if (!nRetCode)
    {
        *pbKeyValue = bDefault;
        KG_PROCESS_ERROR_Q(false);
    }

    *pbKeyValue = false;
    for (int i = 0; i < sizeof(g_pszTrueStrList) / sizeof(const char *); i++)
    {
        nRetCode = KG_Strcasecmp(szBuff, g_pszTrueStrList[i]);
        if (0 == nRetCode)
        {
            *pbKeyValue = true;
            break;
        }
    }

    bResult = true;
Exit0:
    return bResult;
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

    m_lFileBuffOffset = 0;
    while (m_lFileBuffOffset < nBuffSize)
    {
        pStrLine = &pBuff[m_lFileBuffOffset];

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
    KG_PROCESS_ERROR(m_lFileBuffOffset < nBuffSize);

    while (0x0D != pBuff[m_lFileBuffOffset] && 0x0A != pBuff[m_lFileBuffOffset])
    { // '\r' = 0x0D '\n' = 0x0A
        m_lFileBuffOffset++;
        if (m_lFileBuffOffset >= nBuffSize)
        {
            break;
        }
    }
    nCurOffset = m_lFileBuffOffset;

    if (0x0D == pBuff[m_lFileBuffOffset] && 0x0A == pBuff[m_lFileBuffOffset + 1])
    { // "\r\n"
        m_lFileBuffOffset += 2;
    }
    else
    { // "\n"
        m_lFileBuffOffset += 1;
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

        nLen = (int)::strlen(pSecName);
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

        nLen = (int)::strlen(pszKeyName);
        pNextKeyNode->m_pName = ::new char[nLen + 1];                   // Include '\0'
        KG_Strncpy(pNextKeyNode->m_pName, pszKeyName, nLen);
        pNextKeyNode->m_pValue = NULL;
        pNextKeyNode->m_pNext  = NULL;
        pThisKeyNode->m_pNext  = pNextKeyNode;                          // into link
    }

    // replace the old value with new
    KG_DeleteArrayPtrSafely(pNextKeyNode->m_pValue);
    nLen = (int)::strlen(pszKeyValue);
    pNextKeyNode->m_pValue = ::new char[nLen + 1];                     // Include '\0'
    KG_Strncpy(pNextKeyNode->m_pValue, pszKeyValue, nLen);

    bResult = true;
Exit0:
    return bResult;
}

KG_NAMESPACE_END
