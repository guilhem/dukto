#ifndef ECWIN7_H
#define ECWIN7_H

#include <QtGlobal>

#ifdef Q_WS_WIN

/* Include from microsoft sdk converted for mingw */
#include <windows.h>
#include <initguid.h>
#define CMIC_MASK_ASYNCOK SEE_MASK_ASYNCOK

typedef /* [v1_enum] */
enum THUMBBUTTONMASK
{
    THB_BITMAP	= 0x1,
    THB_ICON	= 0x2,
    THB_TOOLTIP	= 0x4,
    THB_FLAGS	= 0x8
} THUMBBUTTONMASK;

typedef /* [v1_enum] */
enum THUMBBUTTONFLAGS
{
    THBF_ENABLED	= 0,
    THBF_DISABLED	= 0x1,
    THBF_DISMISSONCLICK	= 0x2,
    THBF_NOBACKGROUND	= 0x4,
    THBF_HIDDEN	= 0x8,
    THBF_NONINTERACTIVE	= 0x10
} THUMBBUTTONFLAGS;

typedef IUnknown *HIMAGELIST;

typedef struct THUMBBUTTON
{
    THUMBBUTTONMASK dwMask;
    UINT iId;
    UINT iBitmap;
    HICON hIcon;
    WCHAR szTip[ 260 ];
    THUMBBUTTONFLAGS dwFlags;
} THUMBBUTTON;

typedef enum TBPFLAG
{
    TBPF_NOPROGRESS = 0,
    TBPF_INDETERMINATE      = 0x1,
    TBPF_NORMAL     = 0x2,
    TBPF_ERROR      = 0x4,
    TBPF_PAUSED     = 0x8
} TBPFLAG;

typedef struct THUMBBUTTON *LPTHUMBBUTTON;

DECLARE_INTERFACE_(ITaskbarList3,IUnknown)
{
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface) (THIS_ REFIID riid,void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
    /*** ITaskbarList specific methods ***/
    STDMETHOD(HrInit) (THIS) PURE;
    STDMETHOD(AddTab) (THIS_ HWND hwnd) PURE;
    STDMETHOD(DeleteTab) (THIS_ HWND hwnd) PURE;
    STDMETHOD(ActivateTab) (THIS_ HWND hwnd) PURE;
    STDMETHOD(SetActiveAlt) (THIS_ HWND hwnd) PURE;
    STDMETHOD (MarkFullscreenWindow) (THIS_ HWND hwnd, int fFullscreen) PURE;
    /*** ITaskbarList3 members ***/
    STDMETHOD (SetProgressValue)     (THIS_ HWND hwnd, ULONGLONG ullCompleted, ULONGLONG ullTotal) PURE;
    STDMETHOD (SetProgressState)     (THIS_ HWND hwnd, TBPFLAG tbpFlags) PURE;
    STDMETHOD (RegisterTab)          (THIS_ HWND hwndTab,HWND hwndMDI) PURE;
    STDMETHOD (UnregisterTab)        (THIS_ HWND hwndTab) PURE;
    STDMETHOD (SetTabOrder)          (THIS_ HWND hwndTab, HWND hwndInsertBefore) PURE;
    STDMETHOD (SetTabActive)         (THIS_ HWND hwndTab, HWND hwndMDI, DWORD dwReserved) PURE;
    STDMETHOD (ThumbBarAddButtons)   (THIS_ HWND hwnd, UINT cButtons, LPTHUMBBUTTON pButton) PURE;
    STDMETHOD (ThumbBarUpdateButtons)(THIS_ HWND hwnd, UINT cButtons, LPTHUMBBUTTON pButton) PURE;
    STDMETHOD (ThumbBarSetImageList) (THIS_ HWND hwnd, HIMAGELIST himl) PURE;
    STDMETHOD (SetOverlayIcon)       (THIS_ HWND hwnd, HICON hIcon, LPCWSTR pszDescription) PURE;
    STDMETHOD (SetThumbnailTooltip)  (THIS_ HWND hwnd, LPCWSTR pszTip) PURE;
    STDMETHOD (SetThumbnailClip)     (THIS_ HWND hwnd, RECT *prcClip) PURE;
};
typedef ITaskbarList3 *LPITaskbarList3;

/* End */

#endif

#include <QWidget>

class EcWin7
{
public:
    EcWin7();
    void init(WId wid);
    void setOverlayIcon(QString iconName, QString description);
    bool winEvent(MSG * message, long * result);

    enum ToolBarProgressState {
        NoProgress = 0,
        Indeterminate = 1,
        Normal = 2,
        Error = 4,
        Paused = 8
    };

    void setProgressValue(int value, int max);
    void setProgressState(ToolBarProgressState state);

private:
    WId mWindowId;
#ifdef Q_WS_WIN
    UINT mTaskbarMessageId;
    ITaskbarList3 *mTaskbar;
    HICON mOverlayIcon;
#endif
};

#endif // ECWIN7_H
