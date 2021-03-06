#pragma once

#ifdef _WIN32                                                            // windows platform
    #define KG_PLATFORM_NAME "windows"
    #define KG_PLATFORM_WINDOWS

    #ifdef _WIN64
        #define KG_IN_64BIT_MODE
    #endif // _WIN64
#else                                                                   // linux platform
    #define KG_PLATFORM_NAME "linux"
    #define KG_PLATFORM_LINUX

    #ifdef __LP64__
        #define KG_IN_64BIT_MODE
    #endif // __LP64__
#endif // WIN32

#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
    // define WIN32_LEAN_AND_MEAN to exclude APIs such as Cryptography, DDE, RPC, Shell and Windows Sockets.
    #undef  WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    /*------------------------------------------------------------------------------*/
    /* [ windows 8, windows 8.1                            ] _WIN32_WINNT >= 0x0602 */
    /* [ windows 7                                         ] _WIN32_WINNT >= 0x0601 */
    /* [ windows vista, windows server 2008                ] _WIN32_WINNT >= 0x0600 */
    /* [ windows server 2003 with sp1, windows xp with sp2 ] _WIN32_WINNT >= 0x0502 */
    /* [ windows server 2003, windows xp                   ] _WIN32_WINNT >= 0x0501 */
    /* [ windows 2000                                      ] _WIN32_WINNT >= 0x0500 */
    /* [ windows nt 4.0     ] _WIN32_WINNT >= 0x0400                                */
    /*------------------------------------------------------------------------------*/
    #undef  _WIN32_WINNT
    #define _WIN32_WINNT 0x0500
    #include <windows.h>

    #include <io.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <assert.h>
#else                                                                   // linux platform
#endif // KG_PLATFORM_WINDOWS

/*---------------------------------------------------------------------------------------*/
/* <Type>       <Windows-32bits>    <Windows-64bits>    <Linux-32bits>    <Linux-64bits> */
/* char                 1                    1                  1                1       */
/* short                2                    2                  2                2       */
/* int                  4                    4                  4                4       */
/* long                 4                    4                  4                8       */
/* long long            8                    8                  8                8       */
/* float                4                    4                  4                4       */
/* double               8                    8                  8                8       */
/* long double          12                   16                 12               16      */
/* pointer              4                    8                  4                8       */
/* size_t               4                    8                  4                8       */
/*---------------------------------------------------------------------------------------*/
#ifdef KG_PLATFORM_WINDOWS                                              // windows platform

#else                                                                   // linux platform
    typedef unsigned char       BYTE;                                   // 1 bytes
    typedef short               SHORT;                                  // 2 bytes
    typedef unsigned short      USHORT;                                 // 2 bytes
    typedef USHORT              WORD;                                   // 2 bytes
    typedef int                 INT32;                                  // 4 bytes
    typedef unsigned int        UINT32;                                 // 4 bytes
    typedef UINT32              DWORD;                                  // 4 bytes
    typedef int                 LONG;                                   // 4 bytes
    typedef UINT32              ULONG;                                  // 4 bytes
    typedef int                 BOOL;                                   // 4 bytes
    typedef signed long long    INT64;                                  // 8 bytes
    typedef unsigned long long  UINT64;                                 // 8 bytes
    typedef UINT64              ULONGLONG;                              // 8 bytes
#endif // KG_PLATFORM_WINDOWS

#undef  KG_NAMESPACE_BEGIN
#define KG_NAMESPACE_BEGIN(ns) namespace ns {

#undef  KG_NAMESPACE_END
#define KG_NAMESPACE_END }

#undef  KG_UNREFERENCED_PARAMETER
#ifdef  KG_PLATFORM_WINDOWS                                             // windows platform
#define KG_UNREFERENCED_PARAMETER(p) UNREFERENCED_PARAMETER(p)
#else                                                                   // linux   platform
#define KG_UNREFERENCED_PARAMETER(p) (void)(p)
#endif  // KG_UNREFERENCED_PARAMETER

// Macro : KG_DISABLE_WARNING
#undef  KG_DISABLE_WARNING
#ifdef _MSC_VER                                                         // ms vc++
#define KG_DISABLE_WARNING(code, expression)            \
        __pragma(warning(push))                         \
        __pragma(warning(disable:code)) expression      \
        __pragma(warning(pop))
#else                                                                   // gcc
#define KG_DISABLE_WARNING(code, expression)            \
        expression
#endif // _MSC_VER

// Macro : KG_WHILE_FALSE_NO_WARNING
#undef  KG_WHILE_FALSE_NO_WARNING
#define KG_WHILE_FALSE_NO_WARNING KG_DISABLE_WARNING(4127, while(false))

// Macro : KG_PROCESS_ERROR_Q
#undef  KG_PROCESS_ERROR_Q
#define KG_PROCESS_ERROR_Q(condition)                   \
    do                                                  \
    {                                                   \
        if (!(condition))                               \
        {                                               \
            goto Exit0;                                 \
        }                                               \
    } KG_WHILE_FALSE_NO_WARNING

// Macro : KG_PROCESS_ERROR
#undef  KG_PROCESS_ERROR
#define KG_PROCESS_ERROR(condition)                     \
    do                                                  \
    {                                                   \
        if (!(condition))                               \
        {                                               \
            KG_ASSERT(false);                           \
            goto Exit0;                                 \
        }                                               \
    } KG_WHILE_FALSE_NO_WARNING

#undef  KG_PROCESS_ERROR_RET_CODE_Q
#define KG_PROCESS_ERROR_RET_CODE_Q(condition, ret)     \
    do                                                  \
    {                                                   \
        if (!(condition))                               \
        {                                               \
            nResult = ret;                              \
            goto Exit0;                                 \
        }                                               \
    } KG_WHILE_FALSE_NO_WARNING

#undef  KG_PROCESS_ERROR_RET_CODE
#define KG_PROCESS_ERROR_RET_CODE(condition, ret)       \
    do                                                  \
    {                                                   \
        if (!(condition))                               \
        {                                               \
            KG_ASSERT(false);                           \
            nResult = ret;                              \
            goto Exit0;                                 \
        }                                               \
    } KG_WHILE_FALSE_NO_WARNING

#undef  KG_PROCESS_SUCCESS_RET_CODE
#define KG_PROCESS_SUCCESS_RET_CODE(condition, ret)     \
    do                                                  \
    {                                                   \
        if (condition)                                  \
        {                                               \
            nResult = ret;                              \
            goto Exit0;                                 \
        }                                               \
    } KG_WHILE_FALSE_NO_WARNING

#undef  KG_PROCESS_SUCCESS
#define KG_PROCESS_SUCCESS(condition)                   \
    do                                                  \
    {                                                   \
        if (condition)                                  \
        {                                               \
            goto Exit1;                                 \
        }                                               \
    } KG_WHILE_FALSE_NO_WARNING

#undef  KG_PROCESS_C_STR_ERROR_Q
#define KG_PROCESS_C_STR_ERROR_Q(s)                     \
    KG_PROCESS_ERROR_Q(NULL != s && '\0' != s[0])

#undef  KG_PROCESS_C_STR_ERROR
#define KG_PROCESS_C_STR_ERROR(s)                       \
    KG_PROCESS_ERROR(NULL != s && '\0' != s[0])

#undef  KG_PROCESS_PTR_ERROR_Q
#define KG_PROCESS_PTR_ERROR_Q(p)                       \
    KG_PROCESS_ERROR_Q(NULL != p)

#undef  KG_PROCESS_PTR_ERROR
#define KG_PROCESS_PTR_ERROR(p)                         \
    KG_PROCESS_ERROR(NULL != p)

#undef  KG_CHECK_RET_CODE_Q
#define KG_CHECK_RET_CODE_Q(condition, ret)             \
    do                                                  \
    {                                                   \
        if (!(condition))                               \
        {                                               \
            return ret;                                 \
        }                                               \
    } KG_WHILE_FALSE_NO_WARNING

// Macro : KG_CHECK_RET_CODE
#undef  KG_CHECK_RET_CODE
#define KG_CHECK_RET_CODE(condition, ret)               \
    do                                                  \
    {                                                   \
        if (!(condition))                               \
        {                                               \
            KG_ASSERT(false);                           \
            return ret;                                 \
        }                                               \
    } KG_WHILE_FALSE_NO_WARNING

// Macro : KG_CHECK_RET_VOID_Q
#undef  KG_CHECK_RET_VOID_Q
#define KG_CHECK_RET_VOID_Q(condition)                  \
    do                                                  \
    {                                                   \
        if (!(condition))                               \
        {                                               \
            return ;                                    \
        }                                               \
    } KG_WHILE_FALSE_NO_WARNING

// Macro : KG_CHECK_RET_VOID
#undef  KG_CHECK_RET_VOID
#define KG_CHECK_RET_VOID(condition)                    \
    do                                                  \
    {                                                   \
        if (!(condition))                               \
        {                                               \
            KG_ASSERT(false);                           \
            return ;                                    \
        }                                               \
    } KG_WHILE_FALSE_NO_WARNING

// Macro       : KG_FetchFieldSize
// Description : fetch the size of the field in a struct or class.
// Param       : type  - class or struct name
//               field - member variable name
#undef  KG_FetchFieldSize
#define KG_FetchFieldSize(type, field) sizeof(((type *)0)->field)

// Macro       : KG_FetchFieldOffset
// Description : fetch the offset of the field in a struct or class.
// Param       : type  - class or struct name
//               field - member variable name
#undef  KG_FetchFieldOffset
#define KG_FetchFieldOffset(type, field) ((ULONG_PTR)&((type *)0)->field)

// Macro       : KG_FetchAddressByField
// Description : fetch the address of a class or struct instance by one member variable of
//               the instance.
// Param       : address - the address of member variable instance
//               type    - class or struct name
//               field   - member variable name
#undef  KG_FetchAddressByField
#ifdef KG_PLATFORM_WINDOWS
    #define KG_FetchAddressByField(address, type, field) CONTAINING_RECORD(address, type, field)
#else
    #define KG_FetchAddressByField(address, type, field) \
        ((type *)((char *)(address) - KG_FetchFieldOffset(type, field)))
#endif

#undef  KG_LOWORD
#define KG_LOWORD(n)        ((WORD)(n))

#undef  KG_HIWORD
#define KG_HIWORD(n)        ((WORD)(((DWORD)(n) >> 16) & 0xFFFF))

#undef  KG_LOBYTE
#define KG_LOBYTE(n)        ((BYTE)(n))

#undef  KG_HIBYTE
#define KG_HIBYTE(n)        ((BYTE)(((WORD)(n) >> 8) & 0xFF))

#undef  KG_MAKEWORD
#define KG_MAKEWORD(l, h)   ((WORD)(((BYTE)(l)) | ((WORD)((BYTE)(h))) << 8))

#undef  KG_MAKELONG
#define KG_MAKELONG(l, h)   ((LONG)(((WORD)(l)) | ((DWORD)((WORD)(h))) << 16))

#undef  KG_INVALID_STR2ID
#define KG_INVALID_STR2ID   static_cast<DWORD>(-1)

#ifdef KG_PLATFORM_WINDOWS                                              // windows platform

#define KG_MAX_FILE_PATH    _MAX_PATH                                   // include '\0' character
#define KG_MAX_FILE_NAME    _MAX_FNAME                                  // include '\0' character
#define KG_MAX_FILE_EXT     _MAX_EXT                                    // include '\0' character
#define KG_MAX_DIR_NAME     _MAX_DIR                                    // include '\0' character

#else                                                                   // linux platform

#define KG_MAX_FILE_PATH    PATH_MAX                                    // include '\0' character
#define KG_MAX_FILE_NAME    (NAME_MAX + 1)                              // include '\0' character
#define KG_MAX_FILE_EXT     (NAME_MAX + 1)                              // include '\0' character
#define KG_MAX_DIR_NAME     (NAME_MAX + 1)                              // include '\0' character

#endif // KG_PLATFORM_WINDOWS

KG_NAMESPACE_BEGIN(xzero)

class KG_UnCopyable
{
protected:
    KG_UnCopyable() {}                                                  // constructor
    virtual ~KG_UnCopyable() {}                                         // destructor
private:
    KG_UnCopyable(const KG_UnCopyable &) {}                             // copy constructor
    const KG_UnCopyable &operator=(const KG_UnCopyable &) {}            // copy assignment
};

class KG_UnConstructable
{
protected:
    virtual ~KG_UnConstructable() {}                                    // destructor
private:
    KG_UnConstructable() {}                                             // constructor
    KG_UnConstructable(const KG_UnConstructable &) {}                   // copy constructor
    const KG_UnConstructable &operator=(const KG_UnConstructable &) {}  // copy assignment
};

template <class T>
inline void KG_ZeroMemory(const T * const p, const size_t nBytes)
{
    if (NULL != p)
    {
        ::memset((void *)p, 0, nBytes);
    }
}

inline void KG_MilliSleep(DWORD dwMilliseconds)
{
#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
    ::Sleep(dwMilliseconds);
#else                                                                   // linux   platform
    ::usleep(dwMilliseconds * 1000);
#endif // KG_PLATFORM_WINDOWS
}

inline void KG_Sleep(DWORD dwSeconds)
{
#ifdef KG_PLATFORM_WINDOWS                                              // windows platform
    ::Sleep(dwSeconds * 1000);
#else                                                                   // linux   platform
    ::sleep(dwSeconds);
#endif // KG_PLATFORM_WINDOWS
}

KG_NAMESPACE_END
