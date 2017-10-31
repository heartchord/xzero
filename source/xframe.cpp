#include "xframe.h"
#include "xtime.h"

KG_NAMESPACE_BEGIN(xzero)

KG_LogicFrameCtrl::KG_LogicFrameCtrl(DWORD dwMaxFps)
{
    m_dwStartTime  = 0;
    m_dwStartFrame = 0;
    m_dwCurFrame   = 0;
    m_dwMaxFps     = dwMaxFps;
}

KG_LogicFrameCtrl::~KG_LogicFrameCtrl()
{
}

void KG_LogicFrameCtrl::Start(const DWORD dwStartFrame)
{
    m_dwStartFrame = dwStartFrame;
    m_dwCurFrame   = dwStartFrame;
    m_dwStartTime  = KG_GetTickCount();
}

void KG_LogicFrameCtrl::AddFrame()
{
    m_dwCurFrame++;
}

bool KG_LogicFrameCtrl::CanExecute() const
{
    DWORD    dwCurTime     = 0;
    uint64_t uElapsedTime  = 0;
    uint64_t uElapsedFrame = 0;

    uElapsedFrame = (m_dwCurFrame - m_dwStartFrame) * 1000;
    dwCurTime     = KG_GetTickCount();
    uElapsedTime  = dwCurTime - m_dwStartTime;

    if (uElapsedFrame > uElapsedTime * m_dwMaxFps)
    {
        return false;
    }

    return true;
}

DWORD KG_LogicFrameCtrl::GetFps() const
{
    DWORD dwCurTime      = KG_GetTickCount();
    DWORD dwElapsedTime  = dwCurTime - m_dwStartTime;
    DWORD dwElapsedFrame = m_dwCurFrame - m_dwStartFrame;

    if (0 == dwElapsedFrame || 0 == m_dwStartFrame)
    {
        return 0;
    }

    return dwElapsedFrame * 1000 / dwElapsedTime;
}

DWORD KG_LogicFrameCtrl::GetTotalFrames() const
{
    return m_dwCurFrame - m_dwStartFrame;
}

KG_NAMESPACE_END
