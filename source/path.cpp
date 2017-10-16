#include "path.h"

KG_NAMESPACE_BEGIN(xzero)

bool KG_IsPathSeparator(int c)
{
    return '/' == c || '\\' == c;
}

bool KG_CreatePath(const char * pszPath)
{
    bool bResult = false;
    int   nRetCode                           = 0;
    char  pFullDirPath[KG_MAX_FILE_PATH_LEN] = {'\0'};                  // ȫ·��
    char *pIter                              = pFullDirPath;
    char  c                                  = 0;

    KG_CHECK_C_STR_RET_BOOL(cszDirPath);
    ::strcpy(pFullDirPath, cszDirPath);                                 // ��������ֹcszDirPathΪ�����ַ���

    while (*pIter)
    {
        c = *pIter;                                                     // ���浱ǰ�ַ�('/' or '\\')
        if (KG_IsPathSeparator(c) && *(pIter + 1))
        { // ������һ���ַ���·���ָ����򲻽��д���
          // ������㷨�����ҵ�·���ָ��������ָ������'\0'�ض��ַ�������Ŀ¼��Ȼ�󽫷ָ�����ԭ������������������C�����ַ��������ԡ�
            *pIter   = '\0';
            nRetCode = _KG_CreateDir(pFullDirPath);
            *pIter   = c;
            KG_CHECK_RET_BOOL(nRetCode);
        }

        pIter++;
    }

    nRetCode = _KG_CreateDir(pFullDirPath);                             // �������һ��Ŀ¼
    KG_CHECK_RET_BOOL(nRetCode);

    return true;
}

KG_NAMESPACE_END
