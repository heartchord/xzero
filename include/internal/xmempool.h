#pragma once

#include "list.h"
#include "debug.h"

#include <atomic>

KG_NAMESPACE_BEGIN(xzero)

#pragma pack(push)                                                          // save current alignment
#pragma pack(1)                                                             // change to 1-byte alignment

typedef struct KG_MemBlock
{
    KG_InterlockedListNode m_Node;                                          // interlocked list node
    UINT32                 m_uSize;                                         // required size of memory block
    BYTE                   m_pData[1];                                      // data buffer
}*PKG_MemBlock;

#pragma pack(pop)                                                           // reset to saved alignment

#pragma pack(push)                                                          // save current alignment
#pragma pack(4)                                                             // change to 4-bytes alignment

typedef struct KG_MemBlockList
{
    UINT32                 m_uEachSize;                                     // the size of each memory block in this list
    KG_InterlockedListHead m_Head;                                          // interlocked list head
}*PKG_MemBlockList;

#pragma pack(pop)                                                           // reset to saved alignment

int          KG_InitMemBlockListArray  (UINT32 uListArraySize, PKG_MemBlockList pListArray, UINT32 uSizeArraySize, UINT32 *pSizeArray);
int          KG_UnInitMemBlockListArray(UINT32 uListArraySize, PKG_MemBlockList pListArray);
PKG_MemBlock KG_AllocateMemBlock       (UINT32 uListArraySize, PKG_MemBlockList pListArray, UINT32 uRequiredSize);
int          KG_RecycleMemBlock        (UINT32 uListArraySize, PKG_MemBlockList pListArray, PKG_MemBlock pRecycledBlock);

template <UINT32 uSizeArraySize, UINT32 pSizeArray[]>
class KG_MemoryPool : public KG_UnCopyable
{
public:
    KG_MemoryPool();
    ~KG_MemoryPool();

public:
    int Get(void ** ppMemBlock, UINT32 uRequiredSize);
    int Put(void ** ppMemBlock);

private:
    std::atomic<long> m_lGetTimes;
    std::atomic<long> m_lPutTimes;
    KG_MemBlockList   m_pListArray[uSizeArraySize];
};

template <UINT32 uSizeArraySize, UINT32 pSizeArray[]>
KG_MemoryPool<uSizeArraySize, pSizeArray>::KG_MemoryPool()
{
    int nRetCode = false;

    KG_ASSERT(uSizeArraySize > 0);
    KG_ASSERT(NULL != pSizeArray);

    m_lGetTimes = 0;
    m_lPutTimes = 0;

    nRetCode = KG_InitMemBlockListArray(uSizeArraySize, m_pListArray, uSizeArraySize, pSizeArray);
    KG_ASSERT(nRetCode);
}

template <UINT32 uSizeArraySize, UINT32 pSizeArray[]>
KG_MemoryPool<uSizeArraySize, pSizeArray>::~KG_MemoryPool()
{
    int nRetCode = false;

    nRetCode = KG_UnInitMemBlockListArray(uSizeArraySize, m_pListArray);
    KG_ASSERT(nRetCode);
    KG_ASSERT(m_lGetTimes == m_lPutTimes && "[ERROR] Memory Pool : memory block leak detected!");

    if (m_lGetTimes != m_lPutTimes)
    {
        KG_DebugPrintln("[ERROR] Memory Pool : memory block leak detected, leaked block num - [%d]", m_lGetTimes - m_lPutTimes);
    }
}

template <UINT32 uSizeArraySize, UINT32 pSizeArray[]>
int KG_MemoryPool<uSizeArraySize, pSizeArray>::Get(void ** ppMemBlock, UINT32 uRequiredSize)
{
    int          nResult   = false;
    int          nRetCode  = false;
    PKG_MemBlock pMemBlock = NULL;

    KG_PROCESS_ERROR_Q(uRequiredSize > 0);
    KG_PROCESS_PTR_ERROR_Q(ppMemBlock);

    *ppMemBlock = NULL;

    pMemBlock = KG_AllocateMemBlock(uSizeArraySize, m_pListArray, uRequiredSize);
    KG_PROCESS_PTR_ERROR_Q(pMemBlock);

//#ifdef _DEBUG
//    printf("KG_MemoryPool::Get() pMemBlock   = %X\n", pMemBlock);
//#endif

    *ppMemBlock = (void *)pMemBlock->m_pData;

//#ifdef _DEBUG
//    printf("KG_MemoryPool::Get() *ppMemBlock = %X\n", *ppMemBlock);
//#endif

    m_lGetTimes++;
    nResult = true;
Exit0:
    return nResult;
}

template <UINT32 uSizeArraySize, UINT32 pSizeArray[]>
int KG_MemoryPool<uSizeArraySize, pSizeArray>::Put(void ** ppMemBlock)
{
    int          nResult   = false;
    int          nRetCode  = false;
    PKG_MemBlock pMemBlock = NULL;

    KG_PROCESS_PTR_ERROR_Q(ppMemBlock);
    KG_PROCESS_PTR_ERROR_Q(*ppMemBlock);

//#ifdef _DEBUG
    //printf("KG_MemoryPool::Put() *ppMemBlock = %X\n", *ppMemBlock);
//#endif

    pMemBlock = KG_FetchAddressByField(*ppMemBlock, KG_MemBlock, m_pData);
    KG_ASSERT(NULL != pMemBlock);

//#ifdef _DEBUG
//    printf("KG_MemoryPool::Put() pMemBlock   = %X\n", pMemBlock);
//#endif

    nRetCode = KG_RecycleMemBlock(uSizeArraySize, m_pListArray, pMemBlock);
    KG_PROCESS_ERROR_Q(nRetCode);

    *ppMemBlock = NULL;

    m_lPutTimes++;
    nResult = true;
Exit0:
    return nResult;
}

int KG_GetFromDefaultMemPool(void ** ppMemBlock, UINT32 uRequiredSize);
int KG_PutIntoDefaultMemPool(void ** ppMemBlock);


KG_NAMESPACE_END
