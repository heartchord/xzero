#pragma once

#include "xpublic.h"

#undef  KG_MAX_INI_SEC_LEN
#define KG_MAX_INI_SEC_LEN  64                                          // Include '\0' character

#undef  KG_MAX_INI_KEY_LEN
#define KG_MAX_INI_KEY_LEN  64                                          // Include '\0' character

KG_NAMESPACE_BEGIN(xzero)

bool KG_OpenFileSafely (FILE *&fp, const char *pszFile, const char *pszMode);
bool KG_CloseFileSafely(FILE *&fp);

class KG_File : private KG_UnCopyable
{
private:
    FILE *m_hFile;
    bool  m_bBinaryFile;

public:
    KG_File();
    ~KG_File();

public:
    bool  Open(const char *pszFile, const char *pszMode);
    bool  Flush();
    bool  Close();

    LONG  Size() const;
    LONG  Tell() const;
    bool  IsEof() const;
    bool  IsOpen() const;

    bool  SeekEnd    (LONG lOffset);
    bool  SeekBegin  (LONG lOffset);
    bool  SeekCurrent(LONG lOffset);

    LONG  Read        (char *pBuff, LONG lBuffSize, LONG lReadSize);
    LONG  ReadTextLine(char *pBuff, LONG lBuffSize, LONG lReadSize);

    LONG  Write        (const char *pBuff,   LONG lBuffSize, LONG lWriteSize);
    LONG  WriteLine    (const char *pBuff,   LONG lBuffSize, LONG lWriteSize);
    LONG  WriteTextLine(const char *pszText, LONG lTextSize, LONG lWriteSize);
    LONG  WriteFormat  (const char *pszFormat, ...);
    
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
    bool Save(KG_File *pFile);

    bool Load(const char *pszFilePath);
    bool Save(const char *pszFilePath);

    void Release();

    bool IsSecExisted(const char *pszSecName);
    bool IsKeyExisted(const char *pszSecName, const char *pszKeyName);

    bool GetNextSec(const char *pszSecName, char *pNextSecBuff, int nBuffLen);
    bool GetNextKey(const char *pszSecName, const char *pszKeyName, char *pNextKeyBuff, int nBuffLen);

    int  GetSecCount();
    int  GetKeyCount(const char *pszSecName);

    bool EraseSec(const char *pszSecName);
    bool EraseKey(const char *pszSecName, const char *pszKeyName);

    bool GetStr   (const char *pszSecName, const char *pszKeyName, const char *pszDefault, char * pKeyValueBuff, int nBuffLen);
    bool GetInt   (const char *pszSecName, const char *pszKeyName, int         nDefault,   int *  pnKeyValue );
    bool GetBool  (const char *pszSecName, const char *pszKeyName, bool        bDefault,   bool * pbKeyValue );
    bool GetLong  (const char *pszSecName, const char *pszKeyName, long        lDefault,   long * plKeyValue );
    bool GetFloat (const char *pszSecName, const char *pszKeyName, float       fDefault,   float *pfKeyValue );
    bool GetStruct(const char *pszSecName, const char *pszKeyName, void *      pvStruct,   int    nStructSize);

    bool SetStr   (const char *pszSecName, const char *pszKeyName, const char *pszKeyValue);
    bool SetInt   (const char *pszSecName, const char *pszKeyName, int         nKeyValue  );
    bool SetBool  (const char *pszSecName, const char *pszKeyName, bool        bKeyValue  );
    bool SetLong  (const char *pszSecName, const char *pszKeyName, long        lKeyValue  );
    bool SetFloat (const char *pszSecName, const char *pszKeyName, float       fKeyValue  );
    bool SetStruct(const char *pszSecName, const char *pszKeyName, void *      pvStruct, int nStructSize);

    bool GetMultiInt  (const char *pszSecName, const char *pszKeyName, int   pnKeyValueArray[], int nArraySize, int *pnKeyValueCount);
    bool GetMultiLong (const char *pszSecName, const char *pszKeyName, long  plKeyValueArray[], int nArraySize, int *pnKeyValueCount);
    bool GetMultiFloat(const char *pszSecName, const char *pszKeyName, float pfKeyValueArray[], int nArraySize, int *pnKeyValueCount);

    bool SetMultiInt  (const char *pszSecName, const char *pszKeyName, int   pnKeyValueArray[], int nKeyValueCount);
    bool SetMultiLong (const char *pszSecName, const char *pszKeyName, long  plKeyValueArray[], int nKeyValueCount);
    bool SetMultiFloat(const char *pszSecName, const char *pszKeyName, float pfKeyValueArray[], int nKeyValueCount);

private:
    bool  IsKeyNameChar (char c) const;
    char *SplitKeyValue (char *szLine) const;
    bool  CreateIniLink (char *pBuff, int nBuffSize);
    bool  GetLineOfBuff (char *pBuff, int nBuffSize);
    bool  FormatSecName (char *pBuff, int nBuffSize, const char *pszSecName) const;
    bool  ReverseSecName(char *pBuff, int nBuffSize, const char *pszSecName) const;

    bool GetKeyValue(const char *pszSecName, const char *pszKeyName, char *pKeyValueBuff, int nBuffLen);
    bool SetKeyValue(const char *pszSecName, const char *pszKeyName, const char *pszKeyValue);
private:
    KG_IniFileSecNode  m_RootSection;                                   // section list
    LONG               m_lFileBuffOffset;                               // file buffer offset
    PKG_IniFileSecNode m_pLatestSection;                                // the latest accessed section
};

KG_NAMESPACE_END
