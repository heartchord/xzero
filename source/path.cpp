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
    char  pFullDirPath[KG_MAX_FILE_PATH_LEN] = {'\0'};                  // 全路径
    char *pIter                              = pFullDirPath;
    char  c                                  = 0;

    KG_CHECK_C_STR_RET_BOOL(cszDirPath);
    ::strcpy(pFullDirPath, cszDirPath);                                 // 拷贝，防止cszDirPath为常量字符串

    while (*pIter)
    {
        c = *pIter;                                                     // 缓存当前字符('/' or '\\')
        if (KG_IsPathSeparator(c) && *(pIter + 1))
        { // 如果最后一个字符是路径分隔符则不进行处理。
          // 巧妙的算法：查找到路径分隔符，将分隔符变成'\0'截断字符串创建目录，然后将分隔符还原回来，这里充分利用了C类型字符串的特性。
            *pIter   = '\0';
            nRetCode = _KG_CreateDir(pFullDirPath);
            *pIter   = c;
            KG_CHECK_RET_BOOL(nRetCode);
        }

        pIter++;
    }

    nRetCode = _KG_CreateDir(pFullDirPath);                             // 创建最后一级目录
    KG_CHECK_RET_BOOL(nRetCode);

    return true;
}

KG_NAMESPACE_END
