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

KG_NAMESPACE_END
