#pragma once

#include "list.h"

KG_NAMESPACE_BEGIN(xzero)

#pragma pack(push)                                                          // save current alignment
#pragma pack(1)                                                             // change to 1-byte alignment

typedef struct KG_MemBlock
{
    KG_InterlockedListNode m_Node;                                          // interlocked list node
    unsigned int           m_uSize;                                         // required size of memory block
    BYTE                   m_pData[1];                                      // data buffer
}*PKG_MemBlock;

#pragma pack(pop)                                                           // reset to saved alignment

#pragma pack(push)                                                          // save current alignment
#pragma pack(4)                                                             // change to 4-bytes alignment

typedef struct KG_MemBlockList
{
    unsigned int           m_uEachSize;                                     // the size of each memory block in this list
    KG_InterlockedListHead m_Head;                                          // interlocked list head
}*PKG_MemBlockList;

#pragma pack(pop)                                                           // reset to saved alignment

int KG_InitMemBlockListArray  (unsigned int uListArraySize, PKG_MemBlockList pListArray, unsigned int uSizeArraySize, unsigned int *pSizeArray);
int KG_UnInitMemBlockListArray(unsigned int uListArraySize, PKG_MemBlockList pListArray);
PKG_MemBlock KG_AllocateMemBlock(unsigned uListArraySize, PKG_MemBlockList pListArray, unsigned uRequiredSize);

KG_NAMESPACE_END
