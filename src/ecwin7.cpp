#include "ecwin7.h"

#include <QDebug>

#if defined(Q_WS_WIN)

// GUID and IID definition
//DEFINE_GUID(IID_IDropTargetHelper,0x4657278b,0x411b,0x11d2,0x83,0x9a,0x00,0xc0,0x4f,0xd9,0x18,0xd0);
//DEFINE_GUID(IID_IPersistFolder2,0x1ac3d9f0,0x175c,0x11d1,0x95,0xbe,0x00,0x60,0x97,0x97,0xea,0x4f);
//DEFINE_GUID(IID_IShellFolder2,0x93f2f68c,0x1d1b,0x11d3,0xa3,0x0e,0x00,0xc0,0x4f,0x79,0xab,0xd1);
//DEFINE_GUID(IID_IPersistFolder3,0xcef04fdf,0xfe72,0x11d2,0x87,0xa5,0x00,0xc0,0x4f,0x68,0x37,0xcf);
DEFINE_GUID(CLSID_TaskbarList,0x56fdf344,0xfd6d,0x11d0,0x95,0x8a,0x0,0x60,0x97,0xc9,0xa0,0x90);
//DEFINE_GUID(IID_ITaskbarList,0x56fdf342,0xfd6d,0x11d0,0x95,0x8a,0x0,0x60,0x97,0xc9,0xa0,0x90);
DEFINE_GUID(IID_ITaskbarList3,0xea1afb91,0x9e28,0x4b86,0x90,0xE9,0x9e,0x9f,0x8a,0x5e,0xef,0xaf);

#endif

EcWin7::EcWin7()
{
    mOverlayIcon = NULL;
}

void EcWin7::init(WId wid)
{
    mWindowId = wid;
#ifdef Q_WS_WIN
    mTaskbarMessageId = RegisterWindowMessage(L"TaskbarButtonCreated");
#endif
}

bool EcWin7::winEvent(MSG * message, long * result)
{
#ifdef Q_WS_WIN
    if (message->message == mTaskbarMessageId)
    {
        HRESULT hr = CoCreateInstance(CLSID_TaskbarList,
                                      0,
                                      CLSCTX_INPROC_SERVER,
                                      IID_ITaskbarList3,
                                      reinterpret_cast<void**> (&(mTaskbar)));
        *result = hr;
        return true;
    }
#endif
    return false;
}

void EcWin7::setProgressValue(int value, int max)
{
#ifdef Q_WS_WIN
    mTaskbar->SetProgressValue(mWindowId, value, max);
#endif
}

void EcWin7::setProgressState(ToolBarProgressState state)
{
#ifdef Q_WS_WIN
    mTaskbar->SetProgressState(mWindowId, (TBPFLAG)state);
#endif
}

void EcWin7::setOverlayIcon(QString iconName, QString description)
{
#ifdef Q_WS_WIN
    HICON oldIcon = NULL;
    if (mOverlayIcon != NULL) oldIcon = mOverlayIcon;
    if (iconName == "")
    {
        mTaskbar->SetOverlayIcon(mWindowId, NULL, NULL);
        mOverlayIcon = NULL;
    }
    else
    {
        mOverlayIcon = (HICON) LoadImage(GetModuleHandle(NULL),
                                 iconName.toStdWString().c_str(),
                                 IMAGE_ICON,
                                 0,
                                 0,
                                 NULL);
        mTaskbar->SetOverlayIcon(mWindowId, mOverlayIcon, description.toStdWString().c_str());
    }
    if ((oldIcon != NULL) && (oldIcon != mOverlayIcon))
    {
        DestroyIcon(oldIcon);
    }
#endif
}
