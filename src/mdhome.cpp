#include <unistd.h>
#include <string.h>

#include <math.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <mdolphin/mdolphin.h>
#include "mdhome.h"
#include "toolbar.h"
#include "network.h"
#include "tabpage.h"

#ifdef MINIGUI_V3
#include "magnifier3.h"
#else
#include "magnifier.h"
#endif

#include "panpages.h"

#ifdef __cplusplus
extern "C" {
#endif
extern double kjs_strtod(char *);
#define MAX_DRAG_STEP   5

extern "C" HWND mgiCreateSoftKeypad(void (*notify)(BOOL));
extern void set_callback_info(HWND hParent);
extern void start_up_dialog_box (HWND hWnd);
extern void notify_ime_status(BOOL);
extern int dragflag;

int g_window_width;
int g_window_height;

int g_browser_left;
int g_browser_top;
int g_browser_width;
int g_browser_height;

int g_toolbar_left;
int g_toolbar_top;
int g_toolbar_width;
int g_toolbar_height;

int g_vscrollbar_length;
int g_hscrollbar_length;

#ifdef MINIGUI_V3
static void DrawBoxFromBitmap (HDC hdc, const RECT* box, const BITMAP* bmp, BOOL h_v, BOOL do_clip);
#endif

/* g_hwnd[0]~g_hwnd[3] for mdolphin 1~4 tab hwnd. 
 * */ 
//HWND g_hwnd[4] = { HWND_NULL, HWND_NULL, HWND_NULL, HWND_NULL};
HWND g_hwnd[TABCOUNT] = { HWND_NULL };
HWND g_current_hwnd = HWND_NULL; /*current table hwnd*/
HWND g_ime_hwnd = HWND_NULL; /*current ime hwnd*/
HWND g_tb_hwnd = HWND_NULL;
HCURSOR cur;

#ifdef MINIGUI_V3
extern HWND g_glass_hwnd;
#endif

static bool g_draged = false;

BOOL g_is_big_screen;
PANNING_CTXT* g_panning_ctxt = NULL;

HDC Htrack[2] = { HDC_INVALID, HDC_INVALID };
BITMAP htbmp;
BITMAP hbbmp;
BITMAP htbmp2;
BITMAP hbbmp2;

const char *home_url=NULL;

static void set_window_layout(void)
{
    if (g_window_width >= 800)
    {
        g_toolbar_height = 36;
        g_window_width = 800;
        g_is_big_screen = true;
    }
    else
    {
        g_toolbar_height = 32;
        g_is_big_screen = false;
        g_window_width = 480;
    }

    g_toolbar_left = 0;
    g_toolbar_top = 0;
    g_toolbar_width = g_window_width;
     
    g_browser_left = 0;
    g_browser_top = g_toolbar_top + g_toolbar_height;
    g_browser_width = g_window_width;
    g_browser_height = g_window_height - g_toolbar_height - g_toolbar_top;

    g_vscrollbar_length = g_browser_height - 5;
    g_hscrollbar_length = g_browser_width - 5;

}
static void set_preference_info(void)
{
	if (g_current_hwnd) {
		SETUP_INFO setup_info;
		mdolphin_fetch_setup(g_current_hwnd, &setup_info);
		setup_info.default_encoding = MD_CHARSET_GB2312_0;
		setup_info.enable_javascript = TRUE;
		//setup_info.enable_plugin = FALSE;
        //setup_info.block_popupwindow = FALSE;
        setup_info.block_popupwindow = true;
        setup_info.scrollbar_width.RegularScrollbarWidth = -1;
        setup_info.scrollbar_width.SmallScrollbarWidth = 0;
        setup_info.scrollbar_width.MiniScrollbarWidth = 0;
     //   setup_info.autoload_images = FALSE;
		mdolphin_import_setup(g_current_hwnd, &setup_info);
	}
}

static void get_home_url(void )
{
    if (home_url)
        return;
    char *current_dir = get_current_dir_name();
    if (current_dir) {
        int len;
            len = strlen("file://")+strlen(current_dir)+strlen("/res/tabview_input.html");
        char *fileName = (char *)malloc(len+1);
        if (fileName) {
            memset(fileName, 0, len+1);
            strncpy(fileName, "file://", strlen("file://"));
            strncat(fileName, current_dir, strlen(current_dir));
            strncat(fileName, "/res/tabview_input.html", strlen("/res/tabview_input.html"));
            fileName[len]='\0';
            home_url = fileName;
            free(current_dir);
        }
    }
}

static void create_scrollbar(HWND parent)
{

    HDC hdc = GetClientDC(parent);
    LoadBitmap(hdc, &htbmp, "res/htrack.png");
    LoadBitmap(hdc, &htbmp2, "res/htrack2.png");
    Htrack[0] = CreateCompatibleDCEx (hdc, 12, VSCROLLBAR_LENGTH);
    SetBrushColor(Htrack[0], RGB2Pixel(hdc, 0xFF,0xFF,0xFF));

    FillBox(Htrack[0],0, 0, 12, VSCROLLBAR_LENGTH);
    SetMemDCAlpha (Htrack[0], MEMDC_FLAG_SRCALPHA, 160);
    SetMemDCColorKey (Htrack[0], MEMDC_FLAG_SRCCOLORKEY, RGB2Pixel(hdc, 0xFF,0xFF,0xFF));

    Htrack[1] = CreateCompatibleDCEx (hdc, HSCROLLBAR_LENGTH, 12);
    SetBrushColor(Htrack[1], RGB2Pixel(hdc, 0xFF,0xFF,0xFF));
    FillBox(Htrack[1],0, 0, HSCROLLBAR_LENGTH, 12);
    SetMemDCAlpha (Htrack[1], MEMDC_FLAG_SRCALPHA, 160);
    SetMemDCColorKey (Htrack[1], MEMDC_FLAG_SRCCOLORKEY, RGB2Pixel(hdc, 0xFF,0xFF,0xFF));

    ReleaseDC(hdc);

    FillBoxWithBitmapPart (Htrack[0], 0, 0, 12, 15, 0, 0, &htbmp, 0, 0);
    float scale = (float)(VSCROLLBAR_LENGTH)/5; 
    int h = (int)roundf(htbmp.bmHeight*scale);
    FillBoxWithBitmapPart (Htrack[0], 0, 15, 12, VSCROLLBAR_LENGTH-18, -1, h, &htbmp, 0, (int)roundf(5*scale));
    FillBoxWithBitmapPart (Htrack[0], 0, VSCROLLBAR_LENGTH-13, 12, 15, 0, 0, &htbmp, 0, 10);

    FillBoxWithBitmapPart (Htrack[1], 0, 0, 5, 12, 0, 0, &htbmp2, 0, 0);
    scale = (float)(HSCROLLBAR_LENGTH)/5; 
    int w = (int)roundf(htbmp2.bmWidth*scale);
    FillBoxWithBitmapPart (Htrack[1], 5, 0, HSCROLLBAR_LENGTH-18, 12, w, -1, &htbmp2, (int)roundf(5*scale), 0);
    FillBoxWithBitmapPart (Htrack[1], HSCROLLBAR_LENGTH-13, 0, 5, 12, 0, 0, &htbmp2, 10, 0);
}


static void paint_scroll_bar(HDC hdc)
{
    int w = 0, h = 0, lv = 0, lh = 0, x = 0, y = 0 , sv = 0, sh = 0;
    mdolphin_get_contents_size(g_current_hwnd, &w, &h);
    mdolphin_get_contents_position(g_current_hwnd, &x, &y);
    if (h&&w) {
        lv = (VSCROLLBAR_LENGTH * VSCROLLBAR_LENGTH)/h;
        sv = (VSCROLLBAR_LENGTH * y)/h;
        lh = (HSCROLLBAR_LENGTH * HSCROLLBAR_LENGTH)/w;
        sh = (HSCROLLBAR_LENGTH * x)/w;
    } else {
        lv = 30;
        lh = 30;
        sv = 0;
        sh = 0;
    }

    if (hbbmp.bmBits == NULL) {
        LoadBitmap (hdc, &hbbmp, "res/hbar.png");
    }
    if (hbbmp2.bmBits == NULL) {
        LoadBitmap(hdc, &hbbmp2, "res/hbar2.png");
    }


	if (lv <10)lv = 10;
	if (lh <10)lh = 10;

    RECT rc = {BROWSER_WIDTH-5, sv+2, BROWSER_WIDTH-5+4, sv+lv-2};
    RECT rc2 = {sh+2, BROWSER_HEIGHT-5, sh+lh-2, BROWSER_HEIGHT-5+4};
    BitBlt(Htrack[0], 0, 0, -1, -1, hdc, BROWSER_WIDTH-8, 0, 0);
    BitBlt(Htrack[1], 0, 0, -1, -1, hdc, 0, BROWSER_HEIGHT-8, 0);
    DrawBoxFromBitmap (hdc, &rc, &hbbmp, FALSE, FALSE);
    DrawBoxFromBitmap (hdc, &rc2, &hbbmp2, TRUE, FALSE);
#ifdef MDOLPHIN_MAJOR_VERSION 
    if (MDOLPHIN_MAJOR_VERSION >= 3) {
        // fill rect corner
        RECT rc3 = {BROWSER_WIDTH-13,  BROWSER_HEIGHT-8, BROWSER_WIDTH, BROWSER_HEIGHT };
        gal_pixel oldBrushColor = SetBrushColor(hdc, PIXEL_lightwhite);
        FillBox(hdc, rc3.left, rc3.top, RECTW(rc3), RECTH(rc3));
        SetBrushColor(hdc, oldBrushColor);
        //exclude scrollbar region, the rectangle should be same as the rect in clear_scroll_bar.
        RECT rc = {BROWSER_WIDTH-8, 0, BROWSER_WIDTH, BROWSER_HEIGHT};
        RECT rc2 = {0, BROWSER_HEIGHT-8, BROWSER_WIDTH, BROWSER_HEIGHT};
        ExcludeClipRect(hdc, &rc);
        ExcludeClipRect(hdc, &rc2);
    }

#endif
}

void my_owner_draw(HWND hWnd, HDC hdc, const RECT* prc)
{
    if (g_draged) 
        paint_scroll_bar(hdc);
}

static void clear_scroll_bar(HWND hwnd)
{
    RECT rc = {BROWSER_WIDTH-8, 0, BROWSER_WIDTH, BROWSER_HEIGHT};
    RECT rc2 = {0, BROWSER_HEIGHT-8, BROWSER_WIDTH, BROWSER_HEIGHT};
    InvalidateRect(hwnd, &rc, FALSE);
    InvalidateRect(hwnd, &rc2, FALSE);
}

#ifdef MINIGUI_V3
static void DrawBoxFromBitmap (HDC hdc, const RECT* box, const BITMAP* bmp, BOOL h_v, BOOL do_clip)
{
    int bmp_w, bmp_h, bmp_x, bmp_y, x, y;

    if (do_clip)
        ClipRectIntersect (hdc, box);

    if (h_v) {
        bmp_w = bmp->bmWidth/3;
        bmp_h = bmp->bmHeight;
        bmp_y = (box->bottom + box->top - bmp_h)>>1;

        FillBoxWithBitmapPart (hdc, box->left, bmp_y,
                        bmp_w, bmp_h, 0, 0, bmp, 0, 0);
        for (x = box->left + bmp_w; x < box->right - bmp_w; x += bmp_w)
            FillBoxWithBitmapPart (hdc, x, bmp_y,
                        bmp_w, bmp_h, 0, 0, bmp, bmp_w, 0);
        FillBoxWithBitmapPart (hdc, box->right - bmp_w, bmp_y, 
                        bmp_w, bmp_h, 0, 0, bmp, bmp_w*2, 0);
    }
    else {
        bmp_w = bmp->bmWidth;
        bmp_h = bmp->bmHeight/3;
        bmp_x = (box->right + box->left - bmp_w)>>1;

        FillBoxWithBitmapPart (hdc, bmp_x, box->top,
                        bmp_w, bmp_h, 0, 0, bmp, 0, 0);
        for (y = box->top + bmp_h; y < box->bottom - bmp_h; y += bmp_h)
            FillBoxWithBitmapPart (hdc, bmp_x, y,
                        bmp_w, bmp_h, 0, 0, bmp, 0, bmp_h);
        FillBoxWithBitmapPart (hdc, bmp_x, box->bottom - bmp_h,
                        bmp_w, bmp_h, 0, 0, bmp, 0, bmp_h*2);
    }
}
#endif
int mouse_down_x,mouse_down_y,mouse_up_x,mouse_up_y;
int MDolphinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    static bool capture = false;
    static int startX = 0;
    static int startY = 0;
    switch (message)
    {
		case MSG_CREATE:
			{
				g_tb_hwnd = create_toobar_window(hWnd);
				create_scrollbar(hWnd);
				create_mdolphin_window(hWnd, &g_hwnd[0], NULL);
				set_callback_info(hWnd);
				set_preference_info();
				get_home_url();
                mdolphin_set_text_encoding_name(g_hwnd[0], MD_CHARSET_AUTODETECT);
				mdolphin_navigate(g_hwnd[0], NAV_GOTO, home_url, FALSE);
			    register_networkobject();
            }
            return 0;

        case MSG_KEYDOWN:
            SendMessage(g_tb_hwnd, MSG_KEYDOWN, wParam, lParam);
            break;
        
        case MSG_KEYUP:
            SendMessage(g_tb_hwnd, MSG_KEYUP, wParam, lParam);
            break;

        case MSG_LBUTTONDOWN:
#if 1
            mouse_down_x=LOSWORD(lParam);
            mouse_down_y=HISWORD(lParam);
            startX = LOSWORD(lParam);
			startY = HISWORD(lParam);
            if (startY > TOOLBAR_HEIGHT){ 
                capture = true;
				SetCapture(hWnd);
			}
			
			if(dragflag){
				return 0;
			}
#endif
#if 0
            int ime_status ;
            ime_status = SendMessage( g_ime_hwnd, MSG_IME_GETSTATUS, 0, 0);
            if( ime_status ){
                return 0;
            }
#endif
			break;
        case MSG_LBUTTONUP:
            if(capture&&GetCapture()==hWnd){
				ReleaseCapture();
				capture = false;
			}

            startX = 0;
            startY = 0;
            if (g_draged) {
                g_draged = false;
                clear_scroll_bar(g_current_hwnd);
                fprintf(stderr,"not move\n");
                return 0;
            }
#if 0 
            mouse_up_x=LOSWORD(lParam);
            mouse_up_y=HISWORD(lParam);
            if (dragflag) {

                int currentX = 0;
                int currentY = 0;
                int offsetX = mouse_down_x-mouse_up_x;
                int offsetY = mouse_down_y-mouse_up_y;
                fprintf(stderr,"offsetX=%d,offsetY=%d\n",offsetX,offsetY);
                if (ABS(offsetX) > MAX_DRAG_STEP|| ABS(offsetY)> MAX_DRAG_STEP) {
                    mdolphin_get_contents_position(g_current_hwnd, &currentX, &currentY);
                    fprintf(stderr,"move offsetX=%d,offsetY=%d\n",offsetX,offsetY);
                    mdolphin_set_contents_position(g_current_hwnd, currentX + offsetX , currentY + offsetY);
                }
                return 0;
            }
            break;
#endif
#if 0
#ifndef MINIGUI_V3
            if (is_magnifier_visible()) {
                int x = LOSWORD(lParam);
                int y = HISWORD(lParam);
                if (try_hide_magnifier(g_tb_hwnd, g_current_hwnd, x, y)) {
                    return 0;
                }
            }
#endif
#endif
			break;
        case MSG_MOUSEMOVE:
#if 0
            if (is_magnifier_visible()) {
                int y = HIWORD(lParam);
                int x = LOWORD(lParam);
	            
#ifndef MINIGUI_V3

                get_screen_to_magnifier(g_current_hwnd, x, y);
#else
                RECT rcBrowser;
                GetWindowRect(g_current_hwnd, &rcBrowser);
                //y = (y + 1) % g_window_height;
                //x = (x + 1) % g_window_width;
                //printf("%d, %d\n", x, y);
                if (PtInRect(&rcBrowser, x, y))
                {
                    get_screen_to_magnifier(g_current_hwnd, x, y);
                }
                else
                {
                   // try_hide_magnifier(g_tb_hwnd, g_current_hwnd, x, y);
                   hide_magnifier();
                }

#endif
                 }
#endif 
#if 1 
            if (capture&&dragflag) {
                MSG msg;
                int x_pos = LOSWORD (lParam);
                int y_pos = HISWORD (lParam);

                while (PeekPostMessage (&msg, hWnd, MSG_MOUSEMOVE, MSG_MOUSEMOVE,
                            PM_REMOVE)) {
                    x_pos = LOSWORD (msg.lParam);
                    y_pos = HISWORD (msg.lParam);

                    fprintf(stderr,"x-pos=%d,y-pos=%d\n",x_pos,y_pos);
                }
                int currentX = 0;
                int currentY = 0;
                int offsetX = startX - x_pos;
                int offsetY = startY - y_pos;
                if (ABS(offsetX) > MAX_DRAG_STEP|| ABS(offsetY)> MAX_DRAG_STEP) {
                    mdolphin_get_contents_position(g_current_hwnd, &currentX, &currentY);
                    bool b = mdolphin_set_contents_position(g_current_hwnd, currentX + offsetX , currentY + offsetY);
                    startX = LOSWORD(lParam);
                    startY = HISWORD(lParam);
                    if (b) g_draged = true;
                }
                return 0;
            }
#endif
            if(dragflag)
            {
                SetCursor(cur);
                return 0;
            }
            else
                break;
#if ASYNC_PANNING
        case MSG_TIMER:
            if (is_panning_pages(g_panning_ctxt))
                panning_pages(g_panning_ctxt);
            else {
                destroy_panning_pages_context(g_panning_ctxt);
                g_panning_ctxt = NULL;
            }
            break;
#endif
        case MSG_DESTROY:
            DestroyAllControls (hWnd);
            return 0;
		case MSG_CLOSE:
			DestroyMainWindow(g_tb_hwnd);
			DestroyMainWindow (hWnd);
			PostQuitMessage (hWnd);
			return 0;
	}
    return DefaultMainWinProc (hWnd, message, wParam, lParam);
}

int MiniGUIMain (int args, const char* argv[])
{
    MSG Msg;
    MAINWINCREATE CreateInfo;
    if (argv[1])
        home_url = argv[1];
    g_window_width = g_rcScr.right ;
    g_window_height = g_rcScr.bottom;
    set_window_layout();

#ifdef MINIGUI_V3
    SetDefaultWindowElementRenderer(argv[2]);
#endif
    mdolphin_register_minigui_ctrlclass ();

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , "mDolphin" , 0 , 0);
#endif

    if (args > 1 && argv[1])
        home_url = argv[1];

    CreateInfo.dwStyle = WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "mDolphin";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(IDC_ARROW);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = MDolphinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = WINDOW_WIDTH;
    CreateInfo.by = WINDOW_HEIGHT;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;

    cur = LoadCursorFromFile("res/hand.cur");

    HWND hMainWnd = CreateMainWindow (&CreateInfo);
    if (hMainWnd == HWND_INVALID)
        return -1;

	start_up_dialog_box (hMainWnd);
    ShowWindow(hMainWnd, SW_SHOWNORMAL);

#if !defined(_LITE_VERSION) && !defined(_MGRM_PROCESSES) && !defined(_STAND_ALONE)
    g_ime_hwnd = mgiCreateSoftKeypad(notify_ime_status);
#endif

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

#ifdef MINIGUI_V3
//	DestroyWindow(g_glass_hwnd);
#endif
    mdolphin_unregister_minigui_ctrlclass ();

    MainWindowThreadCleanup (hMainWnd);

    mdolphin_cleanup ();

    return 0;
}

#ifdef __cplusplus
}
#endif
