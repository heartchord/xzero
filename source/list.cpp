#include "list.h"

namespace xzero
{
#if (defined(_MSC_VER) || defined(__ICL))                               // ms compiler or intel compiler

    /*------------------------------------------------------------------------------------------------------------------*/
    /* __declspec(naked) : Tell compiler that all assemble code of this function will be realized by user, not to add   */
    /*                     any code by compiler. User must save context at the beginning of the function and restore it */
    /*                     at the end of the function.                                                                  */
    /* __fastcall        : Fast call mode, the first two parameters will be passed by registers and the rest parameters */
    /*                     will be passed by stack from right to left. On intel 386 platform, ECX and EDX registers will*/
    /*                     be used.                                                                                     */
    /* esp               : The pointer to the top of current stack, it will point to the top of current stack frame.    */
    /* ebp               : Extended base pointer register, it will point to the bottom of current stack frame.          */
    /* ebx               : Base address register, it stores a base address when memory is addressed.                    */
    /* lock              : Lock bus temporarily, ensure that the address will not be modified during this instruction   */
    /*                     operation.                                                                                   */
    /* cmpxchg8b         : Compares and exchanges 8-bytes data. It will compare the 64-bit number in EDX:EAX with target*/
    /*                     operand, if comparative numbers equal, move ECX:EBX to target, or target operand will be     */
    /*                     moved to EDX:EAX                                                                             */
    /*------------------------------------------------------------------------------------------------------------------*/
    __declspec(naked)
    inline PKG_InterlockedListNode __fastcall KG_PushNodeToInterlockedList(PKG_InterlockedListHead pListHead, PKG_InterlockedListNode pListNode)
    {
        __asm push ebx                                                  // save context
        __asm push ebp                                                  // save context

        __asm mov ebp, ecx                                              // pListHead => ebp
        __asm mov ebx, edx                                              // pListNode => ebx

        __asm mov edx, [ebp + 4]                                        // pListHead->m_nLength, pListHead->m_nOpTimes => edx
            __asm mov eax, [ebp + 0]                                        // pListHead->m_HeadNode.m_pNext               => eax

            Epsh10:
        // pListNode->m_pNext            = pListHead->m_HeadNode.m_pNext
        // pListHead->m_HeadNode.m_pNext = pListNode
        // pListHead->m_nLength         += 1;
        // pListHead->m_nOpTimes        += 1;
        __asm mov[ebx], eax                                            // pListNode->m_pNext = pListHead->m_HeadNode.m_pNext
        __asm mov ecx, edx                                            // pListHead->m_nLength, pListHead->m_nOpTimes => ecx
        __asm add ecx, 010001H                                        // pListHead->m_nLength + 1, pListHead->m_nOpTimes + 1

                                                                      // EBP saves pListHead, and [ebp] will fetch the data where pListHead points to,
                                                                      // EDX:EAX restores the data where pListHead points to before inserting the target node,
                                                                      // If edx:eax != qword ptr [ebp], it indicates the list has been changed already, and must update the data.
                                                                      // if edx:eax == qword ptr [ebp]
                                                                      //     ecx:ebx => qword ptr [ebp]       pListNode, pListHead->m_nLength, pListHead->m_nOpTimes => pListHead
                                                                      // else
                                                                      //     qword ptr [ebp] => edx:eax       pListHead->m_HeadNode, pListHead->m_nLength, pListHead->m_nOpTimes => EDX:EAX
        __asm lock cmpxchg8b qword ptr[ebp]
            __asm jnz  short Epsh10                                         // EDX:EAX != qword ptr [EBP]

        __asm pop ebp                                                   // restore context
        __asm pop ebx                                                   // restore context
        __asm ret                                                       // ret value restored in EAX register(pListNode)
    }

    __declspec(naked)
    inline PKG_InterlockedListNode __fastcall KG_PopNodeFromInterlockedList(PKG_InterlockedListHead pListHead)
    {
        __asm push ebx                                                  // save context
        __asm push ebp                                                  // save context

        __asm mov ebp, ecx                                              // pListHead => ebp
        __asm mov edx, [ebp + 4]                                        // pListHead->m_nLength, pListHead->m_nOpTimes => edx
            __asm mov eax, [ebp + 0]                                        // pListHead->m_HeadNode.m_pNext               => eax

            Epop10:
        __asm or eax, eax                                               // pListHead->m_HeadNode.m_pNext is NULL?
        __asm jz short Epop20                                           // return NULL

        __asm mov ecx, edx                                              // pListHead->m_nLength, pListHead->m_nOpTimes => ecx
        __asm add ecx, 0FFFFH                                           // pListHead->m_nLength - 1, pListHead->m_nOpTimes + 1
        __asm mov ebx, [eax]                                            // *(pListHead->m_HeadNode.m_pNext) => ebx

                                                                        // EDX:EAX restores the data where pListHead points to before inserting the target node,
                                                                        // if edx:eax == qword ptr [ebp]
                                                                        //     ecx:ebx => qword ptr [ebp]       *(pListHead->m_HeadNode.m_pNext), pListHead->m_nLength, pListHead->m_nOpTimes => pListHead
                                                                        // else
                                                                        //     qword ptr [ebp] => edx:eax       pListHead->m_HeadNode, pListHead->m_nLength, pListHead->m_nOpTimes => EDX:EAX
            __asm lock cmpxchg8b qword ptr[ebp]
            __asm jnz  short Epop10                                         // EDX:EAX != qword ptr [EBP]

        Epop20 :
               // 还原现场
               __asm pop ebp                                                   // restore context
               __asm pop ebx                                                   // restore context
               __asm ret                                                       // ret value restored in EAX register(pListNode)
    }

#elif defined(__GNUC__)                             // gnu compiler

    /*------------------------------------------------------------------------------------------------------------------*/
    /* __asm__ __volatile__ : Tell compiler that all assemble code of this function will be realized by user, not to add*/
    /*                        any code by compiler. User must save context at the beginning of the function and restore */
    /*                        it at the end of the function.                                                            */
    /*                        __asm__(template : output : input : destruction descriptor)                               */
    /* %0, %1, %2, ...,     : Operand of instruction or Placeholder. Embedded assembly relies on them to correspond C   */
    /*                        language expressions to instruction operands. The expression in brackets is C expression, */
    /*                        they correspond to instruction operands "%0", "%1", etc. in the order of occurrence.      */
    /*                        a : AX or EAX                                                                             */
    /*                        b : BX or EBX                                                                             */
    /*                        c : CX or ECX                                                                             */
    /*                        d : DX or EDX                                                                             */
    /*                        D : DI or EDI                                                                             */
    /*                        S : SI or ESI                                                                             */
    /*                        q : a, b, c, d register                                                                   */
    /*                        r : any universal register                                                                */
    /*                        = : output                                                                                */
    /*                        + : input and output                                                                      */
    /*     In ASM, when using register like %EAX, we must put one more '%' in front, it's just because in ASM, we use   */
    /* '%0', '%1' ro identify variables.                                                                                */
    /*     b : backward jump                                                                                            */
    /*     f : forward  jump                                                                                            */
    /*     In ASM, we may modify some registers and destruct those protected by gcc, so we must tell compiler which     */
    /* registers are those we want t change. If we use any memory that not declared in 'input' or 'output' defines, they*/
    /* must be target as destructive. We use 'memory' to tell compile that the memory has been changed.                 */
    /*------------------------------------------------------------------------------------------------------------------*/
    inline PKG_InterlockedListNode KG_PushNodeToInterlockedList(PKG_InterlockedListHead pListHead, PKG_InterlockedListNode pListNode)
    {
        PKG_InterlockedListNode pvRetValue;

        __asm__ __volatile__
        (
            "movl (%1),  %%eax\n"                                       // pListHead->m_HeadNode.m_pNext               => eax
            "movl 4(%1), %%edx\n"                                       // pListHead->m_nLength, pListHead->m_nOpTimes => edx

            "1:\n"

            "movl %%eax,    (%2)\n"                                     // pListNode->m_pNext = pListHead->m_HeadNode.m_pNext
            "movl %%edx,    %%ecx\n"                                    // pListHead->m_nLength, pListHead->m_nOpTimes => ecx
            "addl $0x10001, %%ecx\n"                                    // pListHead->m_nLength + 1, pListHead->m_nOpTimes + 1

                                                                        // if edx:eax == *pListHead
                                                                        //     ecx:ebx => *pListHead        [pListNode, pListHead->m_nLength, pListHead->m_nOpTimes] => pListHead
                                                                        // else
                                                                        //     *pListHead => edx:eax        pListHead->m_HeadNode, pListHead->m_nLength, pListHead->m_nOpTimes => EDX:EAX
            "lock cmpxchg8b (%1)\n"
            "jnz  1b\n"                                                 // EDX:EAX != *pListHead
            :
        "=a"(pvRetValue)                                        // %0(output : EAX)
            :
            "D"(pListHead),                                         // %1(input : EDI)
            "b"(pListNode)                         // %2(input : ebx)
            :
            "ecx", "edx", "memory"
            );

        return pvRetValue;
    }

    inline PKG_InterlockedListNode KG_PopNodeFromInterlockedList(PKG_InterlockedListHead pListHead)
    {
        PKG_InterlockedListNode pvRetValue;

        __asm__ __volatile__
        (
            "movl 4(%1), %%edx\n"                       // pListHead->m_nLength, pListHead->m_nOpTimes] => EDX
            "movl (%1),  %%eax\n"                       // pListHead->m_HeadNode.m_pNext => EAX

            "1:\n"

            "or  %%eax, %%eax\n"                        // pListHead->m_HeadNode.m_pNext is NULL?
            "jz  2f\n"                                  // return NULL

            "movl %%edx,   %%ecx\n"                     // pListHead->m_nLength, pListHead->m_nOpTimes => ECX
            "addl $0xFFFF, %%ecx\n"                     // pListHead->m_nLength - 1, pListHead->m_nOpTimes + 1
            "movl (%%eax), %%ebx\n"                     // *(pListHead->m_HeadNode.m_pNext) => EBX

                                                        // if edx:eax == *pListHead
                                                        //     ecx:ebx => *pListHead        *(pListHead->m_HeadNode.m_pNext), pListHead->m_nLength, pListHead->m_nOpTimes => pListHead
                                                        // else
                                                        //     *pListHead => edx:eax        pListHead->m_HeadNode, pListHead->m_nLength, pListHead->m_nOpTimes => EDX:EAX
            "lock cmpxchg8b (%1)\n"
            "jnz  1b\n"                                 // EDX:EAX != qword ptr [EBP]

            "2:\n"
            :
        "=a"(pvRetValue)                        // %0(output : EAX)
            :
            "D"(pListHead)                          // %1(input : EDI)
            :
            "ebx", "ecx", "edx", "memory"
            );

        return pvRetValue;
    }
#endif

    PKG_InterlockedListHead KG_InterlockedList::Initialize(PKG_InterlockedListHead pListHead)
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

    PKG_InterlockedListNode KG_InterlockedList::KG_PushNode(PKG_InterlockedListHead pListHead, PKG_InterlockedListNode pListNode)
    {
        return KG_PushNodeToInterlockedList(pListHead, pListNode);
    }

    PKG_InterlockedListNode KG_InterlockedList::KG_PopNode(PKG_InterlockedListHead pListHead)
    {
        return KG_PopNodeFromInterlockedList(pListHead);
    }
}
