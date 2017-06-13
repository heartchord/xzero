#pragma once

#include "public.h"

namespace xzero
{
#pragma pack(push)                                                          // save current alignment
#pragma pack(4)                                                             // change to 4-bytes alignment

    typedef struct KG_InterlockedListNode
    {
        KG_InterlockedListNode *m_pNext;                                    // pointer to next list node
    }*PKG_InterlockedListNode;

    typedef union KG_InterlockedListHead
    {
        UINT64 m_nAlignment;                                                // aligned to 8-bytes boundary
        struct
        {
            KG_InterlockedListNode m_HeadNode;                              // entrance, head node of the list
            WORD                   m_nLength;                               // length, the node count of the list
            WORD                   m_nOpTimes;                              // op times, the operation times of 'pop' and 'push' calls
        };
    }*PKG_InterlockedListHead;

#pragma pack(pop)                                                           // reset to saved alignment

    class KG_InterlockedList
    {
    public:
        static PKG_InterlockedListHead Initialize (PKG_InterlockedListHead pListHead = NULL);
        static PKG_InterlockedListNode KG_PushNode(PKG_InterlockedListHead pListHead, PKG_InterlockedListNode pListNode);
        static PKG_InterlockedListNode KG_PopNode (PKG_InterlockedListHead pListHead);
    };
}
