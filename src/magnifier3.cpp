#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "magnifier3.h"
#include "mdhome.h"
#include "tabpage.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GLASS_WIDTH		g_Glass_Width
#define GLASS_HEIGHT	g_Glass_Height
    
int g_Glass_Width;
int g_Glass_Height;

extern HWND g_current_hwnd;

HWND g_glass_hwnd = HWND_NULL;

//static HDC glass_mem_dc = HDC_INVALID;
static BOOL glass_show = FALSE;
static BOOL show_at_once = TRUE;
static BITMAP glass_Bmp; 
static RECT g_rcMagnifier;
static BLOCKHEAP g_cliprc_heap;
int show_magnifier(HWND toolbar, HWND browser)
{
    if (browser == HWND_INVALID || toolbar == HWND_INVALID)
        return -1;
	if (glass_show) {
		glass_show = FALSE;
		show_at_once = TRUE;
		ShowWindow(g_glass_hwnd, SW_HIDE);
	} else 
		glass_show = TRUE;

    return 0;
}


BOOL is_magnifier_visible()
{
	if ((g_glass_hwnd != HWND_NULL) && glass_show)
		return TRUE;
	return FALSE;
}

int hide_magnifier()
{
    HWND hMagnifier = g_glass_hwnd;
    if (hMagnifier == HWND_INVALID)
        return -1;

    /* show toolbar */
    ShowWindow(hMagnifier, SW_HIDE);
	show_at_once = TRUE;
    return 0;
}

static CLIPRGN srgn;
static void draw_to_magnifier(HDC hdc, int x, int y, RECT rc)
{
	if (g_current_hwnd && hdc) {
        TAB_INFO* ti = (TAB_INFO*)GetWindowAdditionalData(g_current_hwnd);
        if (!ti || !ti->hdc)
            return;
        ScreenToClient(g_current_hwnd, &rc.left, &rc.top);
        ScreenToClient(g_current_hwnd, &rc.right, &rc.bottom);

        SelectClipRegion(hdc, &srgn);
        StretchBlt(ti->hdc, rc.left, rc.top, RECTW(rc), RECTH(rc), hdc, 0, 0, GLASS_WIDTH, GLASS_HEIGHT , 0);
    }
}

static void compute_location(HWND browser, int x, int y, int &left, int &top, RECT &rcMagnifier)
{
   	
    RECT rc;
    GetWindowRect(browser, &rc);

	left = x-GLASS_WIDTH/2;
	if (left < 0) left = 0;
	if ((left + GLASS_WIDTH)>rc.right) left = rc.right - GLASS_WIDTH;

	top = y - GLASS_HEIGHT - 5;
	if (top < rc.top) 
    {
        top = y + 5;
        SetRect(&rcMagnifier, x-GLASS_WIDTH/4, y-GLASS_HEIGHT*2/10, x+GLASS_WIDTH/4, y+GLASS_HEIGHT*3/10);
    }   
    else
    {
	    SetRect(&rcMagnifier, x-GLASS_WIDTH/4, y-GLASS_HEIGHT*3/10, x+GLASS_WIDTH/4, y+GLASS_HEIGHT*2/10);
    }

}
void get_screen_to_magnifier(HWND browser, int x, int y)
{
    int left, top;
    compute_location(browser, x, y, left, top, g_rcMagnifier);
	if (show_at_once) {
		show_at_once = FALSE;
		ShowWindow(g_glass_hwnd, SW_SHOW);
	}

	MoveWindow(g_glass_hwnd, left, top, GLASS_WIDTH, GLASS_HEIGHT, TRUE);
}

static CLIPRGN crgn;
static int GlassWinProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
        case MSG_CREATE:
            {
                int nGlassWidth = g_Glass_Width - 10;
                InitFreeClipRectList(&g_cliprc_heap, 100);
                InitClipRgn(&crgn, &g_cliprc_heap);
                InitClipRgn(&srgn, &g_cliprc_heap);

                InitCircleRegion(&crgn, g_Glass_Width/2, g_Glass_Height/2, g_Glass_Width/2);
                InitCircleRegion(&srgn, g_Glass_Width/2, g_Glass_Height/2, nGlassWidth/2);

            }

            break;
		case MSG_MOUSEMOVE:
		case MSG_LBUTTONDOWN:
		case MSG_LBUTTONUP:
        case MSG_RBUTTONDOWN:
            if (is_magnifier_visible()) {
				RECT rc;
				GetWindowRect(hwnd, &rc);
				int y = HIWORD(lParam)+rc.top;
				int x = LOWORD(lParam)+rc.left;
				get_screen_to_magnifier(g_current_hwnd, x, y);
			}
            return 0;
#if 0            
        case MSG_PAINT:
            {
                RECT rc;
                GetWindowRect(hwnd, &rc);
                ScreenToClient(g_current_hwnd, &rc.left, &rc.top);
                HDC hdc = BeginPaint(hwnd);
                draw_to_magnifier(hdc, rc.left, rc.top, g_rcMagnifier);
                ReleaseDC(hdc);
                return 0;
            }
#endif
		case MSG_ERASEBKGND:
            if (is_magnifier_visible()) 
            {
                TAB_INFO* ti = (TAB_INFO*)GetWindowAdditionalData(g_current_hwnd);
                if (!ti || !ti->hdc)
                {
                    return 0;
                }
               
                HDC hdc = (HDC) wParam;
                bool fGetDC = FALSE;
                if (hdc == 0)
                {
                    hdc = GetClientDC(hwnd);
                    fGetDC = TRUE;
                }

                RECT rc;
                GetWindowRect(hwnd, &rc);
                ScreenToClient(g_current_hwnd, &rc.left, &rc.top);
                SaveDC(hdc);
                BitBlt(ti->hdc, rc.left, rc.top, g_Glass_Width, g_Glass_Height , hdc, 0, 0, 0); 

                SelectClipRegion(hdc, &crgn);
                FillBoxWithBitmap(hdc, 0, 0, -1, -1,  &glass_Bmp);
                draw_to_magnifier(hdc, rc.left, rc.top, g_rcMagnifier);
                RestoreDC(hdc, -1);
                if (fGetDC)
                    ReleaseDC(hdc);
            }
            return 0;
		case MSG_CLOSE:
			EmptyClipRgn(&srgn);
			EmptyClipRgn(&crgn);
			DestroyFreeClipRectList(&g_cliprc_heap);
			break;
		case MSG_DESTROY:
            return 0;
	}
	return DefaultMainWinProc(hwnd, message, wParam, lParam);
}

int create_glass_window(HWND parent)
{
    if (!g_is_big_screen)
    {
        if (LoadBitmap(HDC_SCREEN, &glass_Bmp, "res/zoom_glass.png")) 
            return -1;
    }
    else
    {
        if (LoadBitmap(HDC_SCREEN, &glass_Bmp, "res-800/zoom_glass.png")) 
            return -1;
    }
    g_Glass_Width = glass_Bmp.bmWidth ;
    g_Glass_Height =  glass_Bmp.bmHeight ;

    MAINWINCREATE CreateInfo;

    /* Initialize the MAINWINCREATE structure. */
    CreateInfo.dwStyle = WS_NONE;
    CreateInfo.spCaption= "";
    CreateInfo.dwExStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = GlassWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = glass_Bmp.bmWidth;
    CreateInfo.by = glass_Bmp.bmHeight;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;

    /* Create the main window. */
    g_glass_hwnd = CreateMainWindow(&CreateInfo);
    if (g_glass_hwnd == HWND_INVALID)
        return 0;
    return 0;
}
#ifdef __cplusplus
}
#endif
