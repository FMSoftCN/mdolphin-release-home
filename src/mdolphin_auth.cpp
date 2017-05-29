
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <minigui/mgconfig.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define IDC_AUTH_USER  901
#define IDC_AUTH_PASS  902

extern HWND g_current_hwnd;

static char * user=NULL;
static char * pass=NULL;
static int isOk = 0;
static DLGTEMPLATE PopWinData =
{
     WS_CAPTION | WS_THINFRAME | WS_BORDER,
     WS_EX_NONE, 
     0, 0 , 318, 217,
     "Prompt",
     0, 0,
     7,
     NULL, 0
};

static CTRLDATA CtrlInitProgress [] =
{
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        10, 22, 300, 23,
        0,
        "Enter username and password for Auth",
        0, WS_EX_TRANSPARENT
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE,
        10, 56, 90, 23,
        0,
        "User Name:",
        0, WS_EX_TRANSPARENT
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        100, 56, 191, 23,
        IDC_AUTH_USER,
        NULL,
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE,
        10, 90, 90, 23,
        0,
        "Password:",
        0, WS_EX_TRANSPARENT
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_PASSWORD | WS_BORDER,
        100, 90, 191, 23,
        IDC_AUTH_PASS,
        NULL,
        0
    },
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
        51, 145, 92, 25,
        IDOK, 
        "Ok",
        0
    },
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
        177, 145, 92, 25,
        IDCANCEL, 
        "Cancel",
        0
    }
};

static int AuthBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{

    RECT rc;
    int len = 0;
    switch (message) {
        case MSG_INITDIALOG: 
            GetWindowRect ((HWND)(lParam), &rc);
            MoveWindow (hDlg , (rc.right-rc.left)/2-160 , (rc.bottom-rc.top)/2-120 , 320 , 220 , TRUE);
            return 1;
        case MSG_COMMAND:
            switch (wParam) {
                case IDOK: 
                    {
                        isOk = 1;
                        len = GetWindowTextLength (GetDlgItem (hDlg, IDC_AUTH_USER));
                        if (len > 0) 
                        {
                            user = (char*) malloc( len + 1);
                            if (user) {
                                memset(user, 0, len+1);
                                GetWindowText (GetDlgItem (hDlg, IDC_AUTH_USER), user, len);
                            }
                        }
                        len = GetWindowTextLength (GetDlgItem (hDlg, IDC_AUTH_PASS));
                        if (len > 0) 
                        {
                            pass = (char*) malloc( len + 1);
                            if (pass) {
                                memset(pass, 0, len+1);
                                GetWindowText (GetDlgItem (hDlg, IDC_AUTH_PASS), pass, len);
                            }
                        }
                    }    
                case IDCANCEL:
                    EndDialog (hDlg, wParam);
                    break;
            }
            break;

        case MSG_CLOSE:
            EndDialog (hDlg, IDCANCEL);
            break;
    }

    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int create_auth_window(HWND hParent, const char* title)
{
    PopWinData.controls = CtrlInitProgress;
    if (title && strlen(title)) {
        char buf[256] = "Enter username and password for ";
        strncat(buf, title, strlen(title));
        buf[255]='\0';
        CtrlInitProgress[0].caption = buf;
    }
    return DialogBoxIndirectParam (&PopWinData, hParent, AuthBoxProc, (DWORD)hParent);
}

BOOL my_provide_auth_callbackv3 (const char* title, char *username, size_t userNameLen, 
        char *password, size_t passwordLen)
{
    BOOL result = FALSE;
    create_auth_window(g_current_hwnd, title);
    if (isOk) {
        if (user && pass) {
            strcpy(username, user);
            strcpy(password, pass);
        }
        isOk = 0;
        result = TRUE;
    }
    if (user) {
        free(user);
        user = NULL;
    }
    if (pass) {
        free(pass);
        pass = NULL;
    }
    return result;
}

BOOL my_provide_auth_callback (const char* title, char *username, char *password)
{
    BOOL result = FALSE;
    create_auth_window(g_current_hwnd, title);
    if (isOk) {
        if (user && pass) {
            strcpy(username, user);
            strcpy(password, pass);
        }
        isOk = 0;
        result = TRUE;
    }
    if (user) {
        free(user);
        user = NULL;
    }
    if (pass) {
        free(pass);
        pass = NULL;
    }
    return result;
}
