#include "file.h"
#include "debug.h"

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
    m_lFileBuffOffset = 0;
    m_pLatestSection  = NULL;
}

KG_NAMESPACE_END
