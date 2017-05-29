#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "magnifier.h"
#include "mdhome.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAGNIFIER_HEIGHT magnifier_height
#define MAGNIFIER_WIDTH  WINDOW_WIDTH 

#define SET_RECT(rc, l, t, r, b) \
    rc.left = (l); \
    rc.top = (t); \
    rc.right = (r); \
    rc.bottom = (b)

static void set_magnifier_rect(int swidth);
static void get_browser_rect(HWND browser, int x, int y, PRECT prc);

static RECT rcMagnifier;
static RECT rcZoomout;
static RECT rcBtnClose;

static BITMAP* bmpMagnifier = NULL;

static int magnifier_height = 45;

static void set_magnifier_rect(int swidth)
{
	if (g_is_big_screen)
		magnifier_height = 36;

	SET_RECT(rcMagnifier, 0, 0, MAGNIFIER_WIDTH, MAGNIFIER_HEIGHT);
	if (swidth == 800) {
        SET_RECT(rcZoomout, 3, 3, 758, 32);
        SET_RECT(rcBtnClose, 768, 6, 794, 28);
    } else {
        SET_RECT(rcZoomout, 3, 4, 439, 39);
        SET_RECT(rcBtnClose, 438, 9, 474, 32);
    }
}

int show_magnifier(HWND toolbar, HWND browser)
{
    if (browser == HWND_INVALID || toolbar == HWND_INVALID)
        return -1;

    if (!bmpMagnifier) {
        if (!(bmpMagnifier = (BITMAP*)malloc(sizeof(BITMAP))))
            return -1;
        if (!g_is_big_screen) {
            if (LoadBitmap(HDC_SCREEN, bmpMagnifier, "res/magnifier.png")) 
                return -1;
        } else {
            if (LoadBitmap(HDC_SCREEN, bmpMagnifier, "res-800/magnifier.png")) 
                return -1;
        }
        set_magnifier_rect(WINDOW_WIDTH);
    }

    /* hide toolbar */
    ShowWindow(toolbar, SW_HIDE);

	/* move browser down */
	if (!g_is_big_screen)
		MoveWindow(browser, BROWSER_LEFT, MAGNIFIER_HEIGHT,
				BROWSER_WIDTH, BROWSER_HEIGHT - (MAGNIFIER_HEIGHT - TOOLBAR_HEIGHT), TRUE);

    /* fill magnifier backgroud */
    InvalidateRect(HWND_DESKTOP, &rcMagnifier, FALSE);
    FillBoxWithBitmap(HDC_SCREEN, 0, 0, MAGNIFIER_WIDTH, MAGNIFIER_HEIGHT, bmpMagnifier);

	if (g_is_big_screen)
		FillBoxWithBitmapPart(HDC_SCREEN, 768, 6, 26, 22, bmpMagnifier->bmWidth, 
				bmpMagnifier->bmHeight, bmpMagnifier, 768, 9);

	if (!g_is_big_screen)
		UpdateWindow(browser, FALSE);
    return 0;
}

int hide_magnifier(HWND toolbar, HWND browser)
{
    if (browser == HWND_INVALID || toolbar == HWND_INVALID)
        return -1;

    /* show toolbar */
    ShowWindow(toolbar, SW_SHOW);

    /* move browser up */
	if (!g_is_big_screen)
		MoveWindow(browser, BROWSER_LEFT, BROWSER_TOP, BROWSER_WIDTH, BROWSER_HEIGHT, TRUE);

    UnloadBitmap(bmpMagnifier);
    bmpMagnifier = 0;
    return 0;
}

BOOL is_magnifier_visible()
{
    return bmpMagnifier != NULL;
}

BOOL try_hide_magnifier(HWND toolbar, HWND browser, int x, int y)
{
    if (PtInRect(&rcBtnClose, x, y))
        return (0 == hide_magnifier(toolbar, browser));
    return FALSE;
}

void get_screen_to_magnifier(HWND browser, int x, int y)
{
    HDC dc;
    RECT rc;

    get_browser_rect(browser, x, y, &rc);

    dc = GetClientDC(browser);
    ScreenToClient(browser, &rc.left, &rc.top);
    ScreenToClient(browser, &rc.right, &rc.bottom);
    StretchBlt(dc, rc.left, rc.top, RECTW(rc), RECTH(rc), 
            HDC_SCREEN, rcZoomout.left, rcZoomout.top,
            RECTW(rcZoomout), RECTH(rcZoomout), 0);
    ReleaseDC(dc);
}

static void get_browser_rect(HWND browser, int x, int y, PRECT prc)
{
    RECT rc;

    prc->left = x - RECTW(rcZoomout)/4;
    prc->top = y - RECTH(rcZoomout)/4;
    prc->right = prc->left + RECTW(rcZoomout)/2;
    prc->bottom = prc->top + RECTH(rcZoomout)/2;

    GetWindowRect(browser, &rc);
    if (prc->left > rc.right - RECTW(rcZoomout)/2) {
        prc->left = rc.right - RECTW(rcZoomout)/2;
        prc->right = prc->left + RECTW(rcZoomout)/2;
    }
    if (prc->top > rc.bottom - RECTH(rcZoomout)/2) {
        prc->top = rc.bottom - RECTH(rcZoomout)/2;
        prc->bottom = prc->top + RECTH(rcZoomout)/2;
    }
}

#ifdef __cplusplus
}
#endif
