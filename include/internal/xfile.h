#pragma once

#include "public.h"

#undef  KG_MAX_INI_SEC_LEN
#define KG_MAX_INI_SEC_LEN  64                                          // Include '\0' character

#undef  KG_MAX_INI_KEY_LEN
#define KG_MAX_INI_KEY_LEN  64                                          // Include '\0' character

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
    bool Save(KG_File *pFile);

    bool Load(const char *pszFilePath);
    bool Save(const char *pszFilePath);

    void Release();

    bool IsSecExisted(const char *pszSecName) const;
    bool IsKeyExisted(const char *pszSecName, const char *pszKeyName) const;

    bool GetNextSec(const char *pszSecName, char *pNextSecBuff, int nBuffLen) const;
    bool GetNextKey(const char *pszSecName, const char *pszKeyName, char *pNextKeyBuff, int nBuffLen) const;

    int  GetSecCount() const;
    int  GetKeyCount(const char *pszSecName) const;

    bool EraseSec(const char *pszSecName);
    bool EraseKey(const char *pszSecName, const char *pszKeyName);

    bool GetStr   (const char *pszSecName, const char *pszKeyName, const char *pszDefault, char * pKeyValueBuff, int nBuffLen) const;
    bool GetInt   (const char *pszSecName, const char *pszKeyName, int         nDefault,   int *  pnKeyValue ) const;
    bool GetBool  (const char *pszSecName, const char *pszKeyName, bool        bDefault,   bool * pbKeyValue ) const;
    bool GetLong  (const char *pszSecName, const char *pszKeyName, long        lDefault,   long * plKeyValue ) const;
    bool GetFloat (const char *pszSecName, const char *pszKeyName, float       fDefault,   float *pfKeyValue ) const;
    bool GetStruct(const char *pszSecName, const char *pszKeyName, void *      pvStruct,   int    nStructSize) const;

    bool SetStr   (const char *pszSecName, const char *pszKeyName, const char *pszKeyValue);
    bool SetInt   (const char *pszSecName, const char *pszKeyName, int         nKeyValue  );
    bool SetBool  (const char *pszSecName, const char *pszKeyName, bool        bKeyValue  );
    bool SetLong  (const char *pszSecName, const char *pszKeyName, long        lKeyValue  );
    bool SetFloat (const char *pszSecName, const char *pszKeyName, float       fKeyValue  );
    bool SetStruct(const char *pszSecName, const char *pszKeyName, void *      pvStruct, int nStructSize);

    bool GetMultiInt  (const char *pszSecName, const char *pszKeyName, int   pnKeyValueArray[], int nArraySize, int *pnKeyValueCount) const;
    bool GetMultiLong (const char *pszSecName, const char *pszKeyName, long  plKeyValueArray[], int nArraySize, int *pnKeyValueCount) const;
    bool GetMultiFloat(const char *pszSecName, const char *pszKeyName, float pfKeyValueArray[], int nArraySize, int *pnKeyValueCount) const;

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
    int                m_nFileBuffOffset;                               // file buffer offset
    PKG_IniFileSecNode m_pLatestSection;                                // the latest accessed section
};

KG_NAMESPACE_END
