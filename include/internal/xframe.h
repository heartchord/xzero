#pragma once

#include "xpublic.h"

#include <memory>

KG_NAMESPACE_BEGIN(xzero)

class IKG_FrameCtrl : private KG_UnCopyable
{
public:
    IKG_FrameCtrl() {}
    virtual ~IKG_FrameCtrl() {}

public:
    virtual void  Start(const DWORD dwStartFrame = 0) = 0;
    virtual void  AddFrame()                          = 0;
    virtual bool  CanExecute()                  const = 0;
    virtual DWORD GetFps()                      const = 0;
    virtual DWORD GetTotalFrames()              const = 0;
};

typedef IKG_FrameCtrl *                PIKG_FrameCtrl;
typedef std::shared_ptr<IKG_FrameCtrl> SPIKG_FrameCtrl;


class KG_LogicFrameCtrl : public IKG_FrameCtrl
{
protected:
    DWORD m_dwStartTime;                                                // game startup time
    DWORD m_dwStartFrame;                                               // loop startup frame
    DWORD m_dwCurFrame;                                                 // current logical frame
    DWORD m_dwMaxFps;

public:
    KG_LogicFrameCtrl(DWORD dwMaxFps);
    virtual ~KG_LogicFrameCtrl();

public:
    virtual void  Start(const DWORD dwStartFrame = 0);
    virtual void  AddFrame();
    virtual bool  CanExecute() const;
    virtual DWORD GetFps() const;
    virtual DWORD GetTotalFrames() const;
};

KG_NAMESPACE_END
