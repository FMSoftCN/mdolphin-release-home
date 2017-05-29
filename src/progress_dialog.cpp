#include <string.h>
#include <minigui/mgconfig.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "progress_dialog.h"

#define IDC_SAVE_PROGRESS    910
#define IDC_SAVE_STATUS      911
//extern HWND g_current_hwnd;
extern HWND g_current_hwnd;
extern void cancel_save_progress(int id);

struct savePar {
    char *url;
    HWND hParent;
    HWND hDlg;
    int  id;
};

static DLGTEMPLATE PopWinData =
{
     WS_CAPTION | WS_BORDER,
     WS_EX_TOPMOST, 
     0, 0 , 400, 150,
     "Save File",
     0, 0,
     3,
     NULL, 0
};

static CTRLDATA CtrlInitProgress [] =
{
    {
        CTRL_STATIC,
        WS_VISIBLE | WS_TABSTOP,
        10, 10, 380, 36,
        IDC_SAVE_STATUS,
        NULL,
        0, WS_EX_TRANSPARENT
    },
    {
        CTRL_PROGRESSBAR,
        WS_VISIBLE,
        10, 60, 380, 20,
        IDC_SAVE_PROGRESS,
        NULL,
        0
    },
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
        170, 90, 60, 25,
        IDCANCEL, 
        "Cancel",
        0
    }
};

static void saveParfree( struct savePar *info)
{
    if (info) { 
        if (info->url)
            free(info->url);
        free(info);
    }
}

static int SaveBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{

    RECT rc;
    struct savePar *info;
    switch (message) {
        case MSG_INITDIALOG: 
            SetWindowAdditionalData(hDlg, lParam);
            info = (struct savePar*)GetWindowAdditionalData (hDlg);
            GetWindowRect (info->hParent, &rc);
            MoveWindow (hDlg , (rc.right-rc.left)/2-160 , (rc.bottom-rc.top)/2-120 , 400 , 150 , TRUE);
            SetWindowText(GetDlgItem (hDlg, IDC_SAVE_STATUS), info->url);
            info->hDlg = GetDlgItem (hDlg, IDC_SAVE_PROGRESS);
            return 1;
        case MSG_COMMAND:
            info = (struct savePar*)GetWindowAdditionalData (hDlg);
            switch (wParam) {
                case IDCANCEL:
                    cancel_save_progress(info->id);
                    saveParfree(info);
                    DestroyMainWindowIndirect(hDlg);
                    break;
            }
            break;
        case MSG_CLOSE:
            info = (struct savePar*)GetWindowAdditionalData (hDlg);
            cancel_save_progress(info->id);
            saveParfree(info);
            DestroyMainWindowIndirect(hDlg);
            break;
    }

    return DefaultWindowProc (hDlg, message, wParam, lParam);
}

HWND create_save_window(const char* url, int id)
{
    struct savePar *info = NULL;
    info = (savePar *)malloc(sizeof(savePar));
    if (info) {
        memset(info, 0, sizeof(savePar));
        info->url =strdup(url);
        info->hParent = GetParent(g_current_hwnd);
        info->hDlg = 0;
        info->id = id;
        PopWinData.controls = CtrlInitProgress;
        CreateMainWindowIndirectParam (&PopWinData, GetParent(g_current_hwnd), SaveBoxProc, (DWORD)info);
        return info->hDlg;
    }
    return HWND_INVALID;
}

void destroy_save_window(HWND hwnd)
{
    struct savePar *info;
    info = (struct savePar*)GetWindowAdditionalData (GetParent(hwnd));
    saveParfree(info);
    DestroyMainWindowIndirect(GetParent(hwnd));
}

