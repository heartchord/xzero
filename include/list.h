#pragma once

#include "public.h"

KG_NAMESPACE_BEGIN(xzero)

#pragma pack(push)                                                          // save current alignment
#pragma pack(4)                                                             // change to 4-bytes alignment

typedef struct KG_InterlockedListNode
{
    KG_InterlockedListNode *m_pNext;                                        // pointer to next node
}*PKG_InterlockedListNode;

typedef union KG_InterlockedListHead
{
    UINT64 m_nAlignment;                                                    // aligned to 8-bytes(64-bits) boundary
    struct
    {
        KG_InterlockedListNode m_HeadNode;                                  // head node of the list
        WORD                   m_nLength;                                   // length, the node count of the list
        WORD                   m_nOpTimes;                                  // op times, the call times of 'pop' and 'push' functions
    };
}*PKG_InterlockedListHead;

#pragma pack(pop)                                                           // reset to saved alignment

PKG_InterlockedListHead KG_InitInterlockedList (PKG_InterlockedListHead pListHead = NULL);
PKG_InterlockedListNode KG_PushNodeToInterlockedList (PKG_InterlockedListHead pListHead, PKG_InterlockedListNode pListNode);
PKG_InterlockedListNode KG_PopNodeFromInterlockedList(PKG_InterlockedListHead pListHead);

KG_NAMESPACE_END
