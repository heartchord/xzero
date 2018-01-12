#include "xlist.h"
#include "xdebug.h"

KG_NAMESPACE_BEGIN(xzero)

#ifdef KG_IN_64BIT_MODE                                                 // [ 64-bit mode ]

PKG_InterlockedListHead KG_InitInterlockedList(PKG_InterlockedListHead pListHead)
{
    if (!pListHead)
    {
        pListHead = new KG_InterlockedListHead;
    }

    _KG_InterlockedListHead head;
    head.m_pNext    = 0;
    head.m_nOpTimes = 0;
    pListHead->store(head);

    return pListHead;
}

PKG_InterlockedListNode KG_PushNodeToInterlockedList(PKG_InterlockedListHead pListHead, PKG_InterlockedListNode pListNode)
{
    _KG_InterlockedListHead oldHead = pListHead->load();
    _KG_InterlockedListHead newHead;

    for(;;)
    {
        newHead.m_pNext    = (uint64_t)pListNode;
        pListNode->m_pNext = (PKG_InterlockedListNode)oldHead.m_pNext;
        newHead.m_nOpTimes = oldHead.m_nOpTimes + 1;

        if (pListHead->compare_exchange_weak(oldHead, newHead))
        {
            break;
        }
    }
    return (PKG_InterlockedListNode)oldHead.m_pNext;
}

PKG_InterlockedListNode KG_PopNodeFromInterlockedList(PKG_InterlockedListHead pListHead)
{
    PKG_InterlockedListNode pListNode = NULL;
    _KG_InterlockedListHead oldHead   = pListHead->load();
    _KG_InterlockedListHead newHead;
    

    if (0 == oldHead.m_pNext)
    {
        return NULL;
    }

    for (;;)
    {
        if (0 == oldHead.m_pNext)
        {
            break;
        }

        pListNode          = (PKG_InterlockedListNode)oldHead.m_pNext;
        newHead.m_pNext    = (uint64_t)pListNode->m_pNext;
        newHead.m_nOpTimes = oldHead.m_nOpTimes + 1;

        if (pListHead->compare_exchange_weak(oldHead, newHead))
        {
            break;
        }
    }

    return (PKG_InterlockedListNode)oldHead.m_pNext;
}

#else                                                                   // [ 32-bit mode ]

#if (defined(_MSC_VER) || defined(__ICL))                               // ms compiler or intel compiler

/*------------------------------------------------------------------------------------------------------------------*/
/* __declspec(naked) : This tells compiler that all assemble code of this function will be realized by custom and   */
/*                     not to add any code by compiler. User must save context at the beginning and restore it at   */
/*                     the end of this function.                                                                    */
/* __fastcall        : Fast call mode, the first two parameters will be passed by registers and the rest parameters */
/*                     will be passed by stack from right to left. On intel 386 platform, ECX and EDX registers will*/
/*                     be used.                                                                                     */
/* ESP               : Stores the pointer to the top of current stack, it points to the top of current stack frame. */
/* EBP               : Extended base pointer register, it points to the bottom of current stack frame.              */
/* EBX               : Base address register, it stores a base address when memory is addressed.                    */
/* lock              : Lock bus temporarily, ensure that the address will not be modified during this instruction   */
/*                     operation.                                                                                   */
/* cmpxchg8b         : Compares and exchanges 8-bytes data. It will compare the 8-bytes number that represented in  */
/*                     EDX:EAX with a target operand. If comparative numbers equal, move ECX:EBX to target operand, */
/*                     or target operand will be moved to EDX:EAX.                                                  */
/*------------------------------------------------------------------------------------------------------------------*/
__declspec(naked)
inline PKG_InterlockedListNode __fastcall KG_PushNode(PKG_InterlockedListHead pListHead, PKG_InterlockedListNode pListNode)
{
    __asm push ebx                                                      // save context
    __asm push ebp                                                      // save context

    __asm mov ebp, ecx                                                  // pListHead => ebp
    __asm mov ebx, edx                                                  // pListNode => ebx

    __asm mov edx, [ebp + 4]                                            // pListHead->m_nLength, pListHead->m_nOpTimes          => edx
    __asm mov eax, [ebp + 0]                                            // pListHead->m_HeadNode(pListHead->m_HeadNode.m_pNext) => eax

Epsh10:
    /*--------------------------------------------------------------------------------------------------------------*/
    /* Some steps when inserting one node into the head position                                                    */
    /* (1) pListNode->m_pNext            = pListHead->m_HeadNode.m_pNext                                            */
    /* (2) pListHead->m_HeadNode.m_pNext = pListNode                                                                */
    /* (3) pListHead->m_nLength         += 1;                                                                       */
    /* (4) pListHead->m_nOpTimes        += 1;                                                                       */
    /*--------------------------------------------------------------------------------------------------------------*/
    __asm mov [ebx], eax                                                // pListNode->m_pNext = pListHead->m_HeadNode.m_pNext(finish step 1)
    __asm mov ecx,   edx                                                // pListHead->m_nLength, pListHead->m_nOpTimes => ecx
    __asm add ecx,   010001H                                            // pListHead->m_nLength + 1, pListHead->m_nOpTimes + 1(prepare step 3 and 4)

    /*--------------------------------------------------------------------------------------------------------------*/
    /*     We use EBP saves 'pListHead' temporarily, and [ebp] will fetch the data where pListHead points to in real*/
    /* time.                                                                                                        */
    /*     EDX:EAX restores the data where pListHead points to before we start inserting the target node.           */
    /*     If EDX:EAX != [EBP], it indicates the list has been changed already, and we must restart all steps.      */
    /* if edx:eax == qword ptr [ebp]                                                                                */
    /*     ecx:ebx => qword ptr [ebp]   pListNode, pListHead->m_nLength, pListHead->m_nOpTimes => pListHead         */
    /* else                                                                                                         */
    /*     qword ptr [ebp] => edx:eax   pListHead => EDX:EAX                                                        */
    /*--------------------------------------------------------------------------------------------------------------*/
    __asm lock cmpxchg8b qword ptr[ebp]                                 // finish step 2 or start it over.
    __asm jnz  short Epsh10                                             // EDX:EAX != qword ptr [EBP]

    __asm pop ebp                                                       // restore context
    __asm pop ebx                                                       // restore context
    __asm ret                                                           // ret value restored in EAX register(pListHead->m_HeadNode)
}

__declspec(naked)
inline PKG_InterlockedListNode __fastcall KG_PopNode(PKG_InterlockedListHead pListHead)
{
    __asm push ebx                                                      // save context
    __asm push ebp                                                      // save context

    __asm mov ebp, ecx                                                  // pListHead => ebp
    __asm mov edx, [ebp + 4]                                            // pListHead->m_nLength, pListHead->m_nOpTimes          => edx
    __asm mov eax, [ebp + 0]                                            // pListHead->m_HeadNode(pListHead->m_HeadNode.m_pNext) => eax

Epop10:
    __asm or eax, eax                                                   // pListHead->m_HeadNode.m_pNext is NULL?
    __asm jz short Epop20                                               // return NULL

    /*--------------------------------------------------------------------------------------------------------------*/
    /* Some steps when inserting one node into the head position                                                    */
    /* (1) pListHead->m_HeadNode.m_pNext = pListNode->m_pNext                                                                */
    /* (2) pListHead->m_nLength         -= 1;                                                                       */
    /* (3) pListHead->m_nOpTimes        += 1;                                                                       */
    /*--------------------------------------------------------------------------------------------------------------*/
    __asm mov ecx, edx                                                  // pListHead->m_nLength, pListHead->m_nOpTimes => ecx
    __asm add ecx, 0FFFFH                                               // pListHead->m_nLength - 1, pListHead->m_nOpTimes + 1
    __asm mov ebx, [eax]                                                // *(pListHead->m_HeadNode.m_pNext) => ebx

    /*--------------------------------------------------------------------------------------------------------------*/
    /*     We use EBP saves 'pListHead' temporarily, and [ebp] will fetch the data where pListHead points to in real*/
    /* time.                                                                                                        */
    /*     EDX:EAX restores the data where pListHead points to before we start inserting the target node.           */
    /*     If EDX:EAX != [EBP], it indicates the list has been changed already, and we must restart all steps.      */
    /* if edx:eax == qword ptr [ebp]                                                                                */
    /*     ecx:ebx => qword ptr [ebp]                                                                               */
    /*     [*(pListHead->m_HeadNode.m_pNext), pListHead->m_nLength, pListHead->m_nOpTimes => pListHead]             */
    /* else                                                                                                         */
    /*     qword ptr [ebp] => edx:eax   pListHead => EDX:EAX                                                        */
    /*--------------------------------------------------------------------------------------------------------------*/

    __asm lock cmpxchg8b qword ptr[ebp]                                 // finish step 1 or start it over.
    __asm jnz  short Epop10                                             // EDX:EAX != qword ptr [EBP]

Epop20:
    __asm pop ebp                                                       // restore context
    __asm pop ebx                                                       // restore context
    __asm ret                                                           // ret value restored in EAX register(pListHead->m_HeadNode)
}

#elif defined(__GNUC__)                                                 // gnu compiler

/*------------------------------------------------------------------------------------------------------------------*/
/* __asm__ __volatile__ : This tells compiler that all assemble code of this function will be realized by custom and*/
/*                        not to add any code by compiler. User must save context at the beginning and restore it at*/
/*                        the end of this function.                                                                 */
/*                        format : __asm__(template : output : input : destruction descriptor)                      */
/* %0, %1, %2, ...,     : Operand of instruction or Placeholder. Embedded assembly relies on them to correspond C   */
/*                        language expressions to instruction operands. The expression in brackets is C expression, */
/*                        they correspond to instruction operands "%0", "%1", etc. in the order of occurrence.      */
/*                            a : AX or EAX                                                                         */
/*                            b : BX or EBX                                                                         */
/*                            c : CX or ECX                                                                         */
/*                            d : DX or EDX                                                                         */
/*                            D : DI or EDI                                                                         */
/*                            S : SI or ESI                                                                         */
/*                            q : a, b, c, d register                                                               */
/*                            r : any universal register                                                            */
/*                            = : output                                                                            */
/*                            + : input and output                                                                  */
/*     In ASM, when using register like %EAX, we must put one more '%' in front of it, it's just because in ASM, we */
/* use '%0', '%1' to identify variables.                                                                            */
/*     b : backward jump.                                                                                           */
/*     f : forward  jump.                                                                                           */
/*     In ASM, we may modify some registers and destruct those protected by gcc, so we must tell compiler which     */
/* registers are those we want to change. If we use any memory that not declared in 'input' or 'output' defines,    */
/* they must be target as destructive. We use 'memory' to tell compile that the memory has been changed.            */
/*------------------------------------------------------------------------------------------------------------------*/
inline PKG_InterlockedListNode KG_PushNode(PKG_InterlockedListHead pListHead, PKG_InterlockedListNode pListNode)
{
    PKG_InterlockedListNode pvRetValue;

    __asm__ __volatile__
    (
        "movl (%1),  %%eax\n"                                       // pListHead->m_HeadNode(pListHead->m_HeadNode.m_pNext) => eax
        "movl 4(%1), %%edx\n"                                       // pListHead->m_nLength, pListHead->m_nOpTimes          => edx

        "1:\n"

        "movl %%eax,    (%2)\n"                                     // pListHead->m_HeadNode.m_pNext               => pListNode->m_pNext
        "movl %%edx,    %%ecx\n"                                    // pListHead->m_nLength, pListHead->m_nOpTimes => ecx
        "addl $0x10001, %%ecx\n"                                    // pListHead->m_nLength + 1, pListHead->m_nOpTimes + 1

        /*--------------------------------------------------------------------------------------------------------------*/
        /* if edx:eax == *pListHead                                                                                     */
        /*     ecx:ebx => *pListHead    pListNode, pListHead->m_nLength, pListHead->m_nOpTimes => pListHead             */
        /* else                                                                                                         */
        /*     *pListHead => edx:eax    pListHead->m_HeadNode, pListHead->m_nLength, pListHead->m_nOpTimes => EDX:EAX   */
        /*--------------------------------------------------------------------------------------------------------------*/
        "lock cmpxchg8b (%1)\n"
        "jnz  1b\n"                                                 // EDX:EAX != *pListHead, jump to position 1
        :
            "=a"(pvRetValue)                                        // %0(output : EAX)
        :
            "D"(pListHead),                                         // %1(input  : EDI)
            "b"(pListNode)                                          // %2(input  : EBX)
        :
            "ecx", "edx", "memory"
    );

    return pvRetValue;
}

inline PKG_InterlockedListNode KG_PopNode(PKG_InterlockedListHead pListHead)
{
    PKG_InterlockedListNode pvRetValue;

    __asm__ __volatile__
    (
        "movl 4(%1), %%edx\n"                                       // pListHead->m_nLength, pListHead->m_nOpTimes          => edx
        "movl (%1),  %%eax\n"                                       // pListHead->m_HeadNode(pListHead->m_HeadNode.m_pNext) => eax

        "1:\n"

        "or  %%eax, %%eax\n"                                        // pListHead->m_HeadNode.m_pNext is NULL?
        "jz  2f\n"                                                  // return NULL

        "movl %%edx,   %%ecx\n"                                     // pListHead->m_nLength, pListHead->m_nOpTimes => ecx
        "addl $0xFFFF, %%ecx\n"                                     // pListHead->m_nLength - 1, pListHead->m_nOpTimes + 1
        "movl (%%eax), %%ebx\n"                                     // *(pListHead->m_HeadNode.m_pNext) => EBX

        /*--------------------------------------------------------------------------------------------------------------*/
        /* if edx:eax == *pListHead                                                                                     */
        /*     ecx:ebx => *pListHead                                                                                    */
        /*     *(pListHead->m_HeadNode.m_pNext), pListHead->m_nLength, pListHead->m_nOpTimes => pListHead               */
        // else                                                                                                         */
        /*     *pListHead => edx:eax    pListHead->m_HeadNode, pListHead->m_nLength, pListHead->m_nOpTimes => EDX:EAX   */
        /*--------------------------------------------------------------------------------------------------------------*/

        "lock cmpxchg8b (%1)\n"
        "jnz  1b\n"                                                 // EDX:EAX != *pListHead

        "2:\n"
        :
            "=a"(pvRetValue)                                        // %0(output : EAX)
        :
            "D"(pListHead)                                          // %1(input  : EDI)
        :
            "ebx", "ecx", "edx", "memory"
    );

    return pvRetValue;
}
#endif

PKG_InterlockedListHead KG_InitInterlockedList(PKG_InterlockedListHead pListHead)
{
    if (!pListHead)
    {
        pListHead = new KG_InterlockedListHead;
    }

    pListHead->m_HeadNode.m_pNext = NULL;
    pListHead->m_nLength          = 0;
    pListHead->m_nOpTimes         = 0;

    return pListHead;
}

PKG_InterlockedListNode KG_PushNodeToInterlockedList(PKG_InterlockedListHead pListHead, PKG_InterlockedListNode pListNode)
{
    return KG_PushNode(pListHead, pListNode);
}

PKG_InterlockedListNode KG_PopNodeFromInterlockedList(PKG_InterlockedListHead pListHead)
{
    return KG_PopNode(pListHead);
}

#endif // KG_IN_64BIT_MODE

KG_ListNode::KG_ListNode()
{
    m_pNext = NULL;
    m_pPrev = NULL;
}

KG_ListNode::~KG_ListNode()
{
}

KG_ListNode *KG_ListNode::Next() const
{
    if (m_pNext && m_pNext->m_pNext)
    { // next node exists and next node is not the tail node.
        return m_pNext;
    }

    return NULL;
}

KG_ListNode *KG_ListNode::Prev() const
{
    if (m_pPrev && m_pPrev->m_pPrev)
    { // prev node exists and prev node is not the head node.
        return m_pPrev;
    }

    return NULL;
}

void KG_ListNode::InsertPrev(KG_ListNode *pNode)
{
    if (!pNode || !m_pPrev)
    { // if inserted node is NULL or the node before current node is NULL.
        KG_ASSERT(false);
        return;
    }

    if (pNode->m_pNext || pNode->m_pPrev)
    { // is inserted node is linked.
        KG_ASSERT(false);
        return;
    }

    // linked
    pNode->m_pPrev   = m_pPrev;
    pNode->m_pNext   = this;
    m_pPrev->m_pNext = pNode;
    m_pPrev          = pNode;
}

void KG_ListNode::InsertNext(KG_ListNode *pNode)
{
    if (!pNode || !m_pNext)
    { // if inserted node is NULL or the node after current node is NULL.
        KG_ASSERT(false);
        return;
    }

    if (pNode->m_pNext || pNode->m_pPrev)
    { // is inserted node is linked.
        KG_ASSERT(false);
        return;
    }

    // linked
    pNode->m_pPrev   = this;
    pNode->m_pNext   = m_pNext;
    m_pNext->m_pPrev = pNode;
    m_pNext          = pNode;
}

void KG_ListNode::Remove()
{
    if (!m_pPrev || !m_pNext)
    {
        KG_ASSERT(false);
        return;
    }

    m_pPrev->m_pNext = m_pNext;
    m_pNext->m_pPrev = m_pPrev;

    m_pPrev = NULL;
    m_pNext = NULL;
}

bool KG_ListNode::IsLinked() const
{
    return (m_pPrev && m_pNext);
}

KG_List::KG_List()
{
    m_Head.m_pNext = &m_Tail;
    m_Tail.m_pPrev = &m_Head;
}

KG_List::~KG_List()
{
}

KG_ListNode *KG_List::Front() const
{
    return m_Head.Next();
}

KG_ListNode *KG_List::Back() const
{
    return m_Tail.Prev();
}

void KG_List::PushFront(KG_ListNode *pNode)
{
    m_Head.InsertNext(pNode);
}

void KG_List::PushBack(KG_ListNode *pNode)
{
    m_Tail.InsertPrev(pNode);
}

KG_ListNode *KG_List::PopFront()
{
    KG_ListNode *pNode = m_Head.Next();

    if (pNode)
    {
        pNode->Remove();
    }

    return pNode;
}

KG_ListNode* KG_List::PopBack()
{
    KG_ListNode *pNode = m_Tail.Prev();

    if (pNode)
    {
        pNode->Remove();
    }

    return pNode;
}

bool KG_List::Empty() const
{
    return (m_Head.Next() == NULL);
}

UINT32 KG_List::Size() const
{
    UINT32       uSize = 0;
    KG_ListNode *pNode = Front();

    while (pNode)
    {
        pNode = pNode->Next();
        uSize++;
    }

    return uSize;
}

KG_ListNode *KG_List::Advance(UINT32 uDistance) const
{
    UINT32       uCurDistance = 0;
    KG_ListNode *pNode        = Front();
    KG_ListNode *pTarget      = NULL;

    while (pNode)
    {
        if (uCurDistance == uDistance)
        {
            pTarget = pNode;
            break;
        }

        uCurDistance += 1;
        pNode = pNode->Next();
    }

    return pTarget;
}

KG_NAMESPACE_END
