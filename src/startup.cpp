
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "mdhome.h" 

#ifdef __cplusplus
extern "C" {
#endif

static DLGTEMPLATE DlgInitProgress =
{
    WS_THINFRAME | WS_VISIBLE, 
    WS_EX_TOPMOST,
    0, 0, 480, 240, 
    "mDophin Starting Up ",
    0, 0,
    0, NULL,
    0
};

static BITMAP sBmp;

#define STARTUP_TIMER_ID	1011
#define STARTUP_TIMER_TIME	200

static int InitDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
    switch (message) {
    case MSG_INITDIALOG:
		if (LoadBitmap(HDC_SCREEN, &sBmp, "res/startup.png")) 
			return -1;

		GetWindowRect((HWND)(lParam), &rc);
		MoveWindow(hDlg, rc.left, rc.top, RECTW(rc), RECTH(rc), TRUE);

		SetTimer(hDlg, STARTUP_TIMER_ID, STARTUP_TIMER_TIME);
		return 1;
        
    case MSG_PAINT:
		{
			HDC hdc = BeginPaint(hDlg);
			GetWindowRect(hDlg, &rc);
			SetBrushColor(hdc, RGB2Pixel(hdc, 0, 0, 0));
			FillBox(hdc, 0, 0, RECTW(rc), RECTH(rc));
			FillBoxWithBitmap(hdc, (RECTW(rc)-sBmp.bmWidth)/2, (RECTH(rc)-sBmp.bmHeight)/2, -1, -1, &sBmp);
			EndPaint(hDlg, hdc);
			return 0;
		}
	case MSG_TIMER:
		UnloadBitmap(&sBmp);
		KillTimer(hDlg, STARTUP_TIMER_ID);
		EndDialog (hDlg, IDCANCEL);
		break;
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void start_up_dialog_box (HWND hParent)
{
    DlgInitProgress.controls = NULL;
    
    DialogBoxIndirectParam (&DlgInitProgress, hParent, InitDialogBoxProc, (DWORD)hParent);
}

#ifdef __cplusplus
}
#endif
