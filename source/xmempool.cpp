#include "xmempool.h"

#include <algorithm>

KG_NAMESPACE_BEGIN(xzero)

UINT32 g_uDefaultMemBlockSizeArray[] =
{
    1  * 8    + 32,
    2  * 8    + 32,
    3  * 8    + 32,
    4  * 8    + 32,
    5  * 8    + 32,
    6  * 8    + 32,
    7  * 8    + 32,
    8  * 8    + 32,
    16 * 8    + 32,
    32 * 8    + 32,
    64 * 8    + 32,
    96 * 8    + 32,
    1  * 1024 + 32,
    2  * 1024 + 32,
    3  * 1024 + 32,
    4  * 1024 + 32,
    5  * 1024 + 32,
    6  * 1024 + 32,
    7  * 1024 + 32,
    8  * 1024 + 32,
    16 * 1024 + 32,
    32 * 1024 + 32,
    64 * 1024 + 32
};

const UINT32 g_uDefaultMemBlockSizeArraySize = sizeof(g_uDefaultMemBlockSizeArray) / sizeof(g_uDefaultMemBlockSizeArray[0]);
static KG_MemoryPool<g_uDefaultMemBlockSizeArraySize, g_uDefaultMemBlockSizeArray> g_DefaultMemoryPool;

static inline bool KG_CompareMemBlockList(const KG_MemBlockList &lhs, const KG_MemBlockList &rhs)
{
    return lhs.m_uEachSize < rhs.m_uEachSize;
}

static inline int KG_UnInitMemBlockList(PKG_InterlockedListHead pListHead)
{
    int                     nResult   = false;
    PKG_MemBlock            pMemBlock = NULL;
    PKG_InterlockedListNode pListNode = NULL;

    // parameters check
    KG_PROCESS_PTR_ERROR_Q(pListHead);

    // do uninitialization
    while (true)
    {
        pListNode = KG_PopNodeFromInterlockedList(pListHead);
        if (!pListNode)
        {
            break;
        }

        pMemBlock = KG_FetchAddressByField(pListNode, KG_MemBlock, m_Node);
    //#ifdef _DEBUG
    //    printf("KG_UnInitMemBlockList() pMemBlock = %X\n", pMemBlock);
    //#endif

        delete[](unsigned char *)pMemBlock;
    }

    nResult = true;
Exit0:
    return nResult;
}

static inline int KG_FindMemBlockList(UINT32 uListArraySize, PKG_MemBlockList pListArray,
    UINT32 uRequiredSize, KG_InterlockedListHead **ppListHead, UINT32 *pnEachSize)
{
    int              nResult = false;
    PKG_MemBlockList pList   = NULL;
    KG_MemBlockList  key;

    KG_PROCESS_ERROR_Q(uRequiredSize > 0 && uListArraySize > 0);
    KG_PROCESS_PTR_ERROR_Q(ppListHead);
    KG_PROCESS_PTR_ERROR_Q(pnEachSize);
    KG_PROCESS_PTR_ERROR_Q(pListArray);

    key.m_uEachSize = uRequiredSize;
    pList = std::lower_bound(pListArray, pListArray + uListArraySize, key, KG_CompareMemBlockList);

    if ((pListArray + uListArraySize) == pList)
    { //not found
        *ppListHead = NULL;
        *pnEachSize = uRequiredSize;
        KG_PROCESS_SUCCESS(true);
    }

    // found
    *ppListHead = &(pList->m_Head);
    *pnEachSize = pList->m_uEachSize;

Exit1:
    nResult = true;
Exit0:
    return nResult;
}

int KG_InitMemBlockListArray(UINT32 uListArraySize, PKG_MemBlockList pListArray, UINT32 uSizeArraySize, UINT32 *pSizeArray)
{
    int nResult = false;

    // parameters check
    KG_PROCESS_ERROR_Q(uListArraySize == uSizeArraySize && uSizeArraySize > 0);
    KG_PROCESS_PTR_ERROR_Q(pListArray);
    KG_PROCESS_PTR_ERROR_Q(pSizeArray);

    // do initialization
    for (UINT32 i = 0; i < uSizeArraySize; ++i)
    {
        KG_ASSERT(pSizeArray[i] > 0);                                   // we can't manage a memory block with 0-byte size
        pListArray[i].m_uEachSize = pSizeArray[i];
        KG_InitInterlockedList(&(pListArray[i].m_Head));
    }

    // sorting
    std::sort(pListArray, pListArray + uSizeArraySize, KG_CompareMemBlockList);

    nResult = true;
Exit0:
    return nResult;
}

int KG_UnInitMemBlockListArray(UINT32 uListArraySize, PKG_MemBlockList pListArray)
{
    int nResult  = false;
    int nRetCode = false;

    // parameters check
    KG_PROCESS_ERROR_Q(uListArraySize > 0);
    KG_PROCESS_PTR_ERROR_Q(pListArray);

    // do uninitialization
    for (UINT32 i = 0; i < uListArraySize; ++i)
    {
        nRetCode = KG_UnInitMemBlockList(&(pListArray[i].m_Head));
        KG_ASSERT(nRetCode);
    }

    nResult = true;
Exit0:
    return nResult;
}

PKG_MemBlock KG_AllocateMemBlock(UINT32 uListArraySize, PKG_MemBlockList pListArray, UINT32 uRequiredSize)
{
    int                     nRetCode   = false;
    UINT32            uEachSize  = 0;
    UINT32            uAllocSize = 0;
    PKG_MemBlock            pResult    = NULL;
    PKG_MemBlock            pMemBlock  = NULL;
    PKG_InterlockedListHead pListHead  = NULL;
    PKG_InterlockedListNode pListNode  = NULL;

    // parameters check
    KG_PROCESS_ERROR(uRequiredSize > 0 && "[ERROR] You're trying to allocate a 0-byte memory block from memory pool.");
    KG_PROCESS_ERROR_Q(uListArraySize > 0);
    KG_PROCESS_PTR_ERROR_Q(pListArray);

    nRetCode = KG_FindMemBlockList(uListArraySize, pListArray, uRequiredSize, &pListHead, &uEachSize);
    KG_PROCESS_ERROR_Q(nRetCode);

    if (!pListHead) // uRequiredSize > All KG_MemBlockList.m_uEachSize, just delete it when recycling.
    { // just allocate one by 'new' operation and KG_MemBlock.m_uSize = uRequiredSize
        uAllocSize = sizeof(KG_MemBlock) + uEachSize;
        KG_PROCESS_ERROR_Q(uAllocSize > sizeof(KG_MemBlock));

        pMemBlock = (PKG_MemBlock)::new(std::nothrow) unsigned char[uAllocSize];
        KG_PROCESS_PTR_ERROR_Q(pMemBlock);
        KG_PROCESS_SUCCESS(true);
    }

    pListNode = KG_PopNodeFromInterlockedList(pListHead);
    if (pListNode)
    { // KG_MemBlockList is not empty, so pop one node from it. uEachSize = pMemBlockList->m_uEachSize
        pMemBlock = KG_FetchAddressByField(pListNode, KG_MemBlock, m_Node);
        KG_PROCESS_SUCCESS(true);
    }

    // KG_MemBlockList is empty, just allocate one by 'new' operation. uEachSize = pMemBlockList->m_uEachSize
    uAllocSize = sizeof(KG_MemBlock) + uEachSize;
    KG_PROCESS_ERROR_Q(uAllocSize > sizeof(KG_MemBlock));

    pMemBlock = (KG_MemBlock *)::new(std::nothrow) unsigned char[uAllocSize];
    KG_PROCESS_PTR_ERROR_Q(pMemBlock);

Exit1:
    pMemBlock->m_uSize = uRequiredSize;
    pResult = pMemBlock;

Exit0:
    return pResult;
}

int KG_RecycleMemBlock(UINT32 uListArraySize, PKG_MemBlockList pListArray, PKG_MemBlock pRecycledBlock)
{
    int                     nResult       = false;
    int                     nRetCode      = false;
    UINT32            uRequiredSize = 0;
    UINT32            uEachSize     = 0;
    PKG_InterlockedListHead pListHead     = NULL;

    // parameters check
    KG_PROCESS_ERROR(NULL != pRecycledBlock && "You're trying to recycle a 0-byte memory block into memory pool.");
    KG_PROCESS_ERROR_Q(uListArraySize > 0);
    KG_PROCESS_PTR_ERROR_Q(pListArray);

    uRequiredSize = pRecycledBlock->m_uSize;
    KG_PROCESS_ERROR(uRequiredSize > 0);

    nRetCode = KG_FindMemBlockList(uListArraySize, pListArray, uRequiredSize, &pListHead, &uEachSize);
    KG_PROCESS_ERROR_Q(nRetCode);

    if (!pListHead) // uRequiredSize > All KG_MemBlockList.m_uEachSize, just delete it when recycling.
    { // just delete it.
        delete[](unsigned char *)pRecycledBlock;
        KG_PROCESS_SUCCESS(true);
    }

    // do recycle operation.
    pRecycledBlock->m_uSize = 0;
    KG_PushNodeToInterlockedList(pListHead, &(pRecycledBlock->m_Node));

Exit1:
    pRecycledBlock = NULL;
    nResult        = true;
Exit0:
    return nResult;
}

int KG_GetFromDefaultMemPool(void ** ppMemBlock, UINT32 uRequiredSize)
{
    return g_DefaultMemoryPool.Get(ppMemBlock, uRequiredSize);
}

int KG_PutIntoDefaultMemPool(void ** ppMemBlock)
{
    return g_DefaultMemoryPool.Put(ppMemBlock);
}

KG_NAMESPACE_END
