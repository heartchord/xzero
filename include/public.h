#pragma once

#ifdef WIN32                                                            // windows platform
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
    #define _WIN32_WINNT 0x0500
    #include <windows.h>
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
#ifdef KG_PLATFORM_WINDOWS                                          // windows platform
#else                                                               // linux platform
typedef unsigned char           BYTE;       // 1 bytes
typedef unsigned short          WORD;       // 2 bytes
typedef unsigned int            DWORD;      // 4 bytes
typedef int                     LONG;       // 4 bytes
typedef int                     BOOL;       // 4 bytes
typedef signed long long        INT64;      // 8 bytes
typedef unsigned long long      UINT64;     // 8 bytes
#endif // KG_PLATFORM_WINDOWS

namespace xzero 
{
    class KG_UnCopyable
    {
    protected:
        KG_UnCopyable() {}                                              // constructor
        ~KG_UnCopyable() {}                                             // destructor
    private:
        KG_UnCopyable(const KG_UnCopyable &) {}                         // copy constructor
        KG_UnCopyable &operator=(const KG_UnCopyable &) {}              // copy assignment
    };

    class KG_UnConstructable
    {
    protected:
        ~KG_UnConstructable() {}                                        // destructor
    private:
        KG_UnConstructable() {}                                         // constructor
        KG_UnConstructable(const KG_UnConstructable &) {}               // copy constructor
        KG_UnConstructable &operator=(const KG_UnConstructable &) {}    // copy assignment
    };
}

#ifdef _DEBUG
    #define KG_ASSERT(x) assert(x)
#else
    #define KG_ASSERT(x) 0
#endif
