#pragma once

#include "xpublic.h"

#include <memory>

KG_NAMESPACE_BEGIN(xzero)

#pragma pack(push)                                                      // save current alignment
#pragma pack(4)                                                         // change to 4-bytes alignment

typedef struct KG_InterlockedListNode
{
    KG_InterlockedListNode *m_pNext;                                    // pointer to next node
}*PKG_InterlockedListNode;

typedef union KG_InterlockedListHead
{
    UINT64 m_nAlignment;                                                // aligned to 8-bytes(64-bits) boundary
    struct
    {
        KG_InterlockedListNode m_HeadNode;                              // head node of the list
        WORD                   m_nLength;                               // length, the node count of the list
        WORD                   m_nOpTimes;                              // op times, the call times of 'pop' and 'push' functions
    };
}*PKG_InterlockedListHead;

#pragma pack(pop)                                                       // reset to saved alignment

PKG_InterlockedListHead KG_InitInterlockedList       (PKG_InterlockedListHead pListHead = NULL);
PKG_InterlockedListNode KG_PushNodeToInterlockedList (PKG_InterlockedListHead pListHead, PKG_InterlockedListNode pListNode);
PKG_InterlockedListNode KG_PopNodeFromInterlockedList(PKG_InterlockedListHead pListHead);

class KG_ListNode
{
public:
    KG_ListNode();
    virtual ~KG_ListNode();

public:
    KG_ListNode *Next() const;                                          // get the pointer to the next node
    KG_ListNode *Prev() const;                                          // get the pointer to the prev node

    void InsertPrev(KG_ListNode *pNode);                                // insert one node before current node
    void InsertNext(KG_ListNode *pNode);                                // insert one node after  current node

    void Remove();                                                      // remove current node from 'KG_List'
    bool IsLinked() const;                                              // is current node linked to a 'KG_List'?

public:
    KG_ListNode *m_pNext;                                               // pointer to the next node
    KG_ListNode *m_pPrev;                                               // pointer to the prev node
};

typedef KG_ListNode *                PKG_ListNode;
typedef std::shared_ptr<KG_ListNode> SPKG_ListNode;

class KG_List
{
public:
    KG_List();
    virtual ~KG_List();

public:
    KG_ListNode *Front() const;                                         // access the first element.
    KG_ListNode *Back () const;                                         // access the last  element.

    void PushFront(KG_ListNode *pNode);                                 // insert an element at the beginning.
    void PushBack (KG_ListNode *pNode);                                 // insert an element at the end.

    KG_ListNode * PopFront();                                           // remove the first element.
    KG_ListNode * PopBack();                                            // remove the last  element.

    bool         Empty() const;                                         // returns whether the list container is empty.
    UINT32       Size()  const;                                         // returns the number of elements in the list container.
    KG_ListNode *Advance(UINT32 uDistance) const;                       // returns the element which advances 'uDistance' from the fisrt element.

private:
    KG_ListNode m_Head;                                                 // head node
    KG_ListNode m_Tail;                                                 // tail node
};

KG_NAMESPACE_END
