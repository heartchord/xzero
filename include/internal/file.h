#pragma once

#include "public.h"

KG_NAMESPACE_BEGIN(xzero)

int KG_OpenFileSafely (FILE *&fp, const char * const cszFile, const char * const cszMode);
int KG_CloseFileSafely(FILE *&fp);

class KG_File : private KG_UnCopyable
{
private:
    FILE *m_hFile;
    bool  m_bBinaryFile;

public:
    KG_File();
    ~KG_File();

public:
    int  Open(const char * const cszFile, const char * const cszMode);
    int  Flush();
    int  Close();

    LONG Size()   const;
    LONG Tell()   const;
    int  IsEof()  const;
    int  IsOpen() const;

    int  SeekEnd    (LONG lOffset);
    int  SeekBegin  (LONG lOffset);
    int  SeekCurrent(LONG lOffset);

    LONG Read         (char * const cpBuff, LONG lBuffSize, LONG lReadSize);
    LONG ReadTextLine (char * const cpBuff, LONG lBuffSize, LONG lReadSize);
    LONG Write        (const char * const cpcBuff, LONG lBuffSize, LONG lWriteSize);
    LONG WriteLine    (const char * const cpcBuff, LONG lBuffSize, LONG lWriteSize);
    LONG WriteTextLine(const char * const cszText, LONG lTextSize, LONG lWriteSize);
};

struct KG_IniFileKeyNode
{
    DWORD              m_dwId;                                          // key Id
    char *             m_pName;                                         // key name
    char *             m_pValue;                                        // key value
    KG_IniFileKeyNode *m_pNext;                                         // next key

    KG_IniFileKeyNode()
    {
        m_dwId   = KG_INVALID_STR2ID;
        m_pName  = NULL;
        m_pValue = NULL;
        m_pNext  = NULL;
    }
};
typedef KG_IniFileKeyNode *PKG_IniFileKeyNode;

struct KG_IniFileSecNode
{
    DWORD              m_dwId;                                          // section Id
    char *             m_pName;                                         // section name
    KG_IniFileKeyNode  m_RootKey;                                       // key list
    KG_IniFileSecNode* m_pNext;                                         // next section

    KG_IniFileSecNode()
    {
        m_dwId  = KG_INVALID_STR2ID;
        m_pName = NULL;
        m_pNext = NULL;
    }
};
typedef KG_IniFileSecNode *PKG_IniFileSecNode;

class KG_IniFile : private KG_UnCopyable
{
public:
    KG_IniFile();
    ~KG_IniFile();

public:
    bool Load(KG_File *pFile);
    bool Load(const char * pszFilePath);

private:
    KG_IniFileSecNode  m_RootSection;                                   // section list
    long               m_lFileBuffOffset;                               // file buffer offset
    PKG_IniFileSecNode m_pLatestSection;                                // the latest accessed section
};

KG_NAMESPACE_END
