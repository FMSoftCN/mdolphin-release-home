#include <string.h>

#include <minigui/mgconfig.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <mdolphin/mdolphin.h>

#include "mdhome.h"

enum{
    NAV_PIC_PAINT,
    NAV_PIC_BKGND,
    NAV_PIC_LEFT,
    NAV_PIC_UP,
    NAV_PIC_RIGHT,
    NAV_PIC_DOWN,
    NAV_PIC_RETURN,
    NAV_PIC_MAX,
};

static char *s_nav_pic[NAV_PIC_MAX] = {const_cast<char*>("res/nav/navi.png"),
                                  const_cast<char*>("res/nav/navi_left.png"),
                                  const_cast<char*>("res/nav/navi_up.png"),
                                  const_cast<char*>("res/nav/navi_right.png"),
                                  const_cast<char*>("res/nav/navi_down.png"),
                                  const_cast<char*>("res/nav/navi_return.png")};

static BITMAP* s_nav_bmp[NAV_PIC_MAX] = {NULL};

extern HWND g_current_hwnd;
extern HWND g_tb_hwnd;
extern HWND g_main_hwnd;

HWND g_nav_hwnd = HWND_INVALID;

//static RECT window_pos = {720, 40, 1020, 320};
static RECT window_pos = {690, 40, 990, 320};

enum{
    OP_PAGE_LEFT,
    OP_PAGE_UP,
    OP_PAGE_RIGHT,
    OP_PAGE_DOWN,
    OP_RETURN,
    OP_MAX,
};
#if 0
static  RECT navi_rect[OP_MAX] = {{8, 20, 20, 40 },
    {20, 4, 40, 20},
    {44, 20, 60, 40},
    {20, 41, 40, 58},
    {21, 21, 39, 39}};
#endif
static  RECT navi_rect[OP_MAX] = {{14, 32, 34, 64},
                                  {35, 10, 67, 31},
                                  {68, 32, 91, 64},
                                  {35, 65, 67, 88},
                                  {35, 32, 67, 64}};

//#define _DEBUG_NAV   1

#ifdef _DEBUG_NAV
#define DEBUG_PRINT(fmt...)  fprintf (stdout, "NAV:"fmt)
#define ERROR_PRINT(fmt...) fprintf (stderr, "NAV:"fmt)
#else
#define DEBUG_PRINT(fmt...)
#define ERROR_PRINT(fmt...)
#endif

static int nav_key_index(int x, int y)
{
    int i = 0;
    int ret = -1;
    for(; i<OP_MAX; i++)
    {
        if(x >= navi_rect[i].left+window_pos.left && x <= navi_rect[i].right+window_pos.left
                && y >= navi_rect[i].top+window_pos.top && y <= navi_rect[i].bottom+window_pos.top)
            ret = i;
    }

    return ret;
}

static void nav_process_keyd(HWND hWnd, int index)
{
    switch(index)
    {
        case OP_PAGE_LEFT:
            {
                //                DEBUG_PRINT("OP_PAGE_LEFT DOWN\n");
                SendMessage (g_tb_hwnd, MSG_KEYDOWN, SCANCODE_CURSORBLOCKLEFT, 0);
                SetAutoRepeatMessage(g_tb_hwnd,MSG_KEYDOWN, SCANCODE_CURSORBLOCKLEFT, 0);
                s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_LEFT];
                InvalidateRect (hWnd, NULL, 0);

                break;
            }
        case OP_PAGE_RIGHT:
            {
                //                DEBUG_PRINT("OP_PAGE_RIGHT DOWN\n");
                SendMessage (g_tb_hwnd, MSG_KEYDOWN, SCANCODE_CURSORBLOCKRIGHT, 0);
                SetAutoRepeatMessage(g_tb_hwnd,MSG_KEYDOWN, SCANCODE_CURSORBLOCKRIGHT, 0);
                s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_RIGHT];
                InvalidateRect (hWnd, NULL, 0);

                break;
            }
        case OP_PAGE_UP:
            {
                //                DEBUG_PRINT("OP_PAGE_UP DOWN\n");
                SendMessage (g_tb_hwnd, MSG_KEYDOWN, SCANCODE_CURSORBLOCKUP, 0);
                SetAutoRepeatMessage(g_tb_hwnd,MSG_KEYDOWN, SCANCODE_CURSORBLOCKUP, 0);
                s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_UP];
                InvalidateRect (hWnd, NULL, 0);

                break;
            }
        case OP_PAGE_DOWN:
            {
                //                DEBUG_PRINT("OP_PAGE_DOWN DOWN\n");
                SendMessage (g_tb_hwnd, MSG_KEYDOWN, SCANCODE_CURSORBLOCKDOWN, 0);
                SetAutoRepeatMessage(g_tb_hwnd,MSG_KEYDOWN, SCANCODE_CURSORBLOCKDOWN, 0);
                s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_DOWN];
                InvalidateRect (hWnd, NULL, 0);

                break;
            }
        case OP_RETURN:
            {
                //                DEBUG_PRINT("OP_RETURN DOWN\n");
                SendMessage (g_tb_hwnd, MSG_KEYDOWN, SCANCODE_ENTER, 0);
                SetAutoRepeatMessage(g_tb_hwnd,MSG_KEYDOWN, SCANCODE_ENTER, 0);
                s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_RETURN];
                InvalidateRect (hWnd, NULL, 0);

                break;
            }
        default :
            break;
    }

}

static void nav_process_keyu(HWND hWnd, int index)
{
    //    DEBUG_PRINT("%s\n", __FUNCTION__);
    switch(index)
    {
        case OP_PAGE_LEFT:
            {
                //                DEBUG_PRINT("OP_PAGE_LEFT UP\n");
                SendMessage (g_tb_hwnd, MSG_KEYUP, SCANCODE_CURSORBLOCKLEFT, 0);
                SetAutoRepeatMessage(NULL,MSG_KEYDOWN, SCANCODE_CURSORBLOCKLEFT, 0);
                s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_BKGND];
                InvalidateRect (hWnd, NULL, 0);

                break;
            }
        case OP_PAGE_RIGHT:
            {
                //                DEBUG_PRINT("OP_PAGE_RIGHT UP\n");
                SendMessage (g_tb_hwnd, MSG_KEYUP, SCANCODE_CURSORBLOCKRIGHT, 0);
                SetAutoRepeatMessage(NULL,MSG_KEYDOWN, SCANCODE_CURSORBLOCKRIGHT, 0);
                s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_BKGND];
                InvalidateRect (hWnd, NULL, 0);

                break;
            }
        case OP_PAGE_UP:
            {
                //                DEBUG_PRINT("OP_PAGE_UP UP\n");
                SendMessage (g_tb_hwnd, MSG_KEYUP, SCANCODE_CURSORBLOCKUP, 0);
                SetAutoRepeatMessage(NULL,MSG_KEYDOWN, SCANCODE_CURSORBLOCKUP, 0);
                s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_BKGND];
                InvalidateRect (hWnd, NULL, 0);

                break;
            }
        case OP_PAGE_DOWN:
            {
                //                DEBUG_PRINT("OP_PAGE_DOWN UP\n");
                SendMessage (g_tb_hwnd, MSG_KEYUP, SCANCODE_CURSORBLOCKDOWN, 0);
                SetAutoRepeatMessage(NULL,MSG_KEYDOWN, SCANCODE_CURSORBLOCKDOWN, 0);
                s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_BKGND];
                InvalidateRect (hWnd, NULL, 0);

                break;
            }
        case OP_RETURN:
            {
                //                DEBUG_PRINT("OP_RETURN UP\n");
                SendMessage (g_tb_hwnd, MSG_KEYUP, SCANCODE_ENTER, 0);
                SetAutoRepeatMessage(NULL,MSG_KEYDOWN, SCANCODE_ENTER, 0);
                s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_BKGND];
                InvalidateRect (hWnd, NULL, 0);

                break;
            }

        default :
            break;
    }
}

int key_index;

static void nav_process_lbd(HWND hWnd, DWORD wParam, DWORD lParam)
{
    int  x, y;

    x = LOSWORD (lParam);
    y = HISWORD (lParam);

    DEBUG_PRINT("lbutton down x:%d, y:%d\n", x, y);
#ifdef _MGRM_PROCESSES            
    //    DEBUG_PRINT("button down x:%d, y:%d\n", x, y);
    if (x >= TOOLBAR_LEFT && x <= TOOLBAR_WIDTH+TOOLBAR_LEFT
            && y>=TOOLBAR_TOP && y <= TOOLBAR_HEIGHT+TOOLBAR_TOP)
    {
        SendMessage (g_tb_hwnd, MSG_LBUTTONDOWN, wParam, lParam); 
    }
    else if (x >= navi_rect[OP_PAGE_LEFT].left+window_pos.left 
            && x <= navi_rect[OP_PAGE_RIGHT].right+window_pos.left
            && y >= navi_rect[OP_PAGE_UP].top+window_pos.top
            && y <= navi_rect[OP_PAGE_DOWN].bottom+window_pos.bottom)
    {
        key_index = nav_key_index(x, y);
        nav_process_keyd(hWnd, key_index);
    }
    else
    {
        SendMessage (g_current_hwnd, MSG_LBUTTONDOWN, wParam, MAKELONG(x, y-TOOLBAR_HEIGHT)); 
    }
#elif _MGRM_THREADS
    key_index = nav_key_index(x+window_pos.left, y+window_pos.top);
    nav_process_keyd(hWnd, key_index);
#endif
}

static void nav_process_lbu(HWND hWnd, DWORD wParam, DWORD lParam)
{
    int key_index, x, y;

    x = LOSWORD (lParam)-window_pos.left;
    y = HISWORD (lParam)-window_pos.top;

    DEBUG_PRINT("lbutton up x:%d, y:%d\n", x, y);
#ifdef _MGRM_PROCESSES            
    //    DEBUG_PRINT("button up x:%d, y:%d\n", x, y);
    if (x >= TOOLBAR_LEFT && x <= TOOLBAR_WIDTH+TOOLBAR_LEFT
            && y>=TOOLBAR_TOP && y <= TOOLBAR_HEIGHT+TOOLBAR_TOP)
    {
        SendMessage (g_tb_hwnd, MSG_LBUTTONUP, wParam, MAKELONG(x,y)); 
    }
    else if (x >= navi_rect[OP_PAGE_LEFT].left+window_pos.left
            && x <= navi_rect[OP_PAGE_RIGHT].right+window_pos.left
            && y >= navi_rect[OP_PAGE_UP].top+window_pos.top
            && y <= navi_rect[OP_PAGE_DOWN].bottom+window_pos.top)
    {
        key_index = nav_key_index(x, y);
        nav_process_keyu(hWnd, key_index);
    }
    else
    {
        SendMessage (g_current_hwnd, MSG_LBUTTONDOWN, wParam, MAKELONG(x, y-2*TOOLBAR_HEIGHT)); 
    }
#elif _MGRM_THREADS
    //key_index = nav_key_index(x+window_pos.left, y+window_pos.top);
    nav_process_keyu(hWnd, key_index);
#endif
}

static void nav_process_move(HWND hWnd, DWORD wParam, DWORD lParam)
{
    int x = LOSWORD (lParam);
    int y = HISWORD (lParam);

    
    //    DEBUG_PRINT("mouse move x:%d, y:%d\n", x, y);
    if (x >= TOOLBAR_LEFT && x <= TOOLBAR_WIDTH+TOOLBAR_LEFT
            && y>=TOOLBAR_TOP && y <= TOOLBAR_HEIGHT+TOOLBAR_TOP)
    {
        //SendMessage (GetParent(g_current_hwnd), MSG_MOUSEMOVE, wParam, MAKELONG(x,y));
    }

}

static int NonRegularWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case MSG_CREATE:
            break;
        case MSG_PAINT:
            {
                HDC hdc = BeginPaint(hWnd);
                FillBoxWithBitmap(hdc, 0, 0, 0, 0,  s_nav_bmp[NAV_PIC_PAINT]);
                EndPaint(hWnd, hdc);
            }
            return 0;
        case MSG_KEYDOWN:
            {
                switch(LOWORD(wParam))
                {
                    case SCANCODE_CURSORBLOCKLEFT:
                        s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_LEFT];
                        InvalidateRect (hWnd, NULL, 0);
                        break;
                    case SCANCODE_CURSORBLOCKRIGHT:
                        s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_RIGHT];
                        InvalidateRect (hWnd, NULL, 0);
                        break;
                    case SCANCODE_CURSORBLOCKUP:
                        s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_UP];
                        InvalidateRect (hWnd, NULL, 0);
                        break;
                    case SCANCODE_ENTER:
                        s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_RETURN];
                        InvalidateRect (hWnd, NULL, 0);
                        break;
                    case SCANCODE_CURSORBLOCKDOWN:
                        s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_DOWN];
                        InvalidateRect (hWnd, NULL, 0);
                        break;
                    default :
                        break;
                }
                SendMessage(g_tb_hwnd, message, wParam, lParam);
                break;
            }
        case MSG_KEYUP:
            {
                switch(LOWORD(wParam))
                {
                    case SCANCODE_CURSORBLOCKLEFT:
                        s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_BKGND];
                        InvalidateRect (hWnd, NULL, 0);
                        break;
                    case SCANCODE_CURSORBLOCKRIGHT:
                        s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_BKGND];
                        InvalidateRect (hWnd, NULL, 0);
                        break;
                    case SCANCODE_CURSORBLOCKUP:
                        s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_BKGND];
                        InvalidateRect (hWnd, NULL, 0);
                        break;
                    case SCANCODE_ENTER:
                        s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_BKGND];
                        InvalidateRect (hWnd, NULL, 0);
                        break;
                    case SCANCODE_CURSORBLOCKDOWN:
                        s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_BKGND];
                        InvalidateRect (hWnd, NULL, 0);
                        break;
                    default :
                        break;
                }
                SendMessage(g_tb_hwnd, message, wParam, lParam);
                break;
            }
         case MSG_LBUTTONDOWN:
            {
                SetCapture(hWnd);
                nav_process_lbd(hWnd, wParam, lParam);
                break;
            }
        case MSG_LBUTTONUP:
            {
                ReleaseCapture();
                nav_process_lbu(hWnd, wParam, lParam);
                break;
            }
        case MSG_MOUSEMOVE:
            {
                nav_process_move(hWnd, wParam, lParam);
                break;
            }
        case MSG_CLOSE:
            {
                DestroyMainWindow (hWnd);
                return 0;
            }
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

void navigate_compass_window(bool isshow)
{
    MAINWINCREATE CreateInfo;
    int i;

    if(isshow == FALSE)
        if(g_nav_hwnd != HWND_INVALID)
        {
            SendMessage(g_nav_hwnd, MSG_CLOSE, 0, 0);
            return;
        }    

    if(g_nav_hwnd != HWND_INVALID)
    {
        SendMessage(g_nav_hwnd, MSG_CLOSE, 0, 0);
    }    
    
    for(i=NAV_PIC_BKGND; i<NAV_PIC_MAX;i++)
    {
        if ((s_nav_bmp[i] = (BITMAP* )LoadResource(s_nav_pic[i-1], RES_TYPE_IMAGE, (DWORD)HDC_SCREEN)) == NULL)
            ERROR_PRINT("LoadResource %s failed.\n", s_nav_pic[i-1]);
    }

    s_nav_bmp[NAV_PIC_PAINT] = s_nav_bmp[NAV_PIC_BKGND];

    CreateInfo.dwStyle = WS_NONE;
    CreateInfo.dwExStyle = WS_EX_NONE | WS_EX_TOPMOST;
    CreateInfo.spCaption = "scroll panel window";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(IDC_HAND_POINT);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = NonRegularWinProc;
    CreateInfo.lx = window_pos.left;
    CreateInfo.ty = window_pos.top;
    CreateInfo.rx = window_pos.right;
    CreateInfo.by = window_pos.bottom;
    CreateInfo.iBkColor = PIXEL_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;

    g_nav_hwnd = CreateMainWindow(&CreateInfo);

    SetWindowMaskEx(g_nav_hwnd, HDC_SCREEN, s_nav_bmp[NAV_PIC_PAINT]);

    ShowWindow (g_nav_hwnd, SW_SHOWNORMAL);
}

void navigate_compass_showwindow()
{
    InvalidateRect( g_nav_hwnd, NULL, TRUE);
}
