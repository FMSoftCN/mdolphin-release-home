#include <stdlib.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <mdolphin/mdolphin.h>
#include "mdhome.h"
#include "toolbar.h"
#include "tabpage.h"
#include "scrollpanel.h"

#ifdef MINIGUI_V3
#include "magnifier3.h"
#else
#include "magnifier.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define TITLE_TXT_HEIGHT   (16)
#define TITLE_TIMER_ID	   (1010)
extern HWND g_current_hwnd;
//extern HWND g_hwnd[4];
extern HWND g_hwnd[TABCOUNT];
extern HWND g_tb_hwnd;
extern HWND g_ime_hwnd;
extern HWND g_nav_hwnd;
extern const char* home_url;
extern bool g_err_hwnd_show;

static BITMAP bkBmp;
static BITMAP toolBarBmp;
static BITMAP progressBmp;
static BITMAP tabBmp;
int dragflag=0;

static BOOL g_close[TABCOUNT] = {FALSE};/*highlight close button*/
static PLOGFONT titleFont;
static char *g_title_text = NULL;

static volatile BOOL g_ime_opened = FALSE;

int g_scale_width = BROWSER_WIDTH;

#if 0
static RENDERING_MODE g_mode = MD_SCREENVIEW_MODE;
#endif

int tab_begin_x = 133;
int tab_begin_y = 4;
int tab_normal_w = 30;
int tab_normal_h = 21;
int tab_current_w = 124;
int tab_close_w = 14;
int tab_close_h = 22;

static TOOLBAR_INFO default_toolbar[] = {
    {OP_NAV_BACKWARD,	{  3, 3,  29, 25},	FALSE,	FALSE},
    {OP_NAV_FORWARD,	{ 36, 3,  62, 25},	FALSE,	FALSE},
    {OP_NAV_HOME,   	{ 68, 3,  94, 25},	TRUE,	FALSE},
    {OP_NAV_STOP,   	{100, 3, 126, 25},	FALSE,	FALSE},
    {OP_NAV_ZOOMIN,		{355, 3, 379, 25},	TRUE,	FALSE},
    {OP_NAV_ZOOMOUT,	{386, 3, 411, 25},	TRUE,	FALSE},
    {OP_NAV_DRAG,   	{417, 3, 443, 25},	TRUE,	FALSE},
    {OP_NAV_IME,		{449, 3, 475, 25},	FALSE,	FALSE},
    {OP_TAB_PAGE,		{133, 5, 347, 27},	FALSE,	FALSE},
};

enum {
ZOOMIN,
ZOOMOUT
};


void set_toolbar_layout(void)
{
	tab_begin_y = 4;
#ifdef WITH_TWO_TAB
	tab_begin_x = 169;
	tab_normal_w = 232;
	tab_current_w = 232;
#else
	tab_begin_x = 175;
	tab_normal_w = 65;
	tab_current_w = 256;
#endif

	tab_normal_h = 26;
	tab_close_w = 22;
	tab_close_h = 26;
	
	SetRect(&(default_toolbar[0].rect),   6, 3,  36, 29);
	SetRect(&(default_toolbar[1].rect),  46, 3,  76, 29);
	SetRect(&(default_toolbar[2].rect),  86, 3, 116, 29);
	SetRect(&(default_toolbar[3].rect), 126, 3, 156, 29 );
	SetRect(&(default_toolbar[4].rect), 646, 3, 676, 29);
	SetRect(&(default_toolbar[5].rect), 686, 3, 716, 29);
	SetRect(&(default_toolbar[6].rect), 726, 3, 756, 29);
	SetRect(&(default_toolbar[7].rect), 766, 3, 796, 29);
#ifdef WITH_TWO_TAB
	SetRect(&(default_toolbar[8].rect), 169, 4, 626+12, 30);
#else
	SetRect(&(default_toolbar[8].rect), 175, 4, 626, 30);
#endif
}

void notify_ime_status(BOOL opened)
{
    g_ime_opened = opened;
}

static void refresh_menu_item(int id)
{
	RECT rect;
	int itemWidth = toolBarBmp.bmWidth /MENU_ITEM_NUM;
	int itemHeight = toolBarBmp.bmHeight /MENU_ITEM_STATUS;
	int tabWidth = TOOLBAR_WIDTH - itemWidth *MENU_ITEM_NUM;

	if (id < 4)
		SetRect (&rect, itemWidth*id, 0, itemWidth*(id+1), itemHeight);
	else if (id < MENU_ITEM_NUM)
		SetRect (&rect, itemWidth*id+tabWidth, 0, itemWidth*(id+1)+tabWidth, itemHeight);
	else
		return;

	InvalidateRect(g_tb_hwnd, &rect, FALSE);
}
static inline BOOL is_forward_enabled()
{
    return default_toolbar[OP_NAV_FORWARD- OP_NAV_BACKWARD].enabled;
}
static inline BOOL is_backward_enabled()
{
    return default_toolbar[OP_NAV_BACKWARD- OP_NAV_BACKWARD].enabled;
}
static inline BOOL is_home_enabled()
{
    return default_toolbar[OP_NAV_HOME- OP_NAV_BACKWARD].enabled;
}
static inline BOOL is_stop_enabled()
{
    return default_toolbar[OP_NAV_STOP- OP_NAV_BACKWARD].enabled;
}


static inline BOOL is_zoomout_enabled()
{
    return default_toolbar[OP_NAV_ZOOMOUT - OP_NAV_BACKWARD].enabled;
}

static inline BOOL is_drag_enabled()
{
    return default_toolbar[OP_NAV_DRAG - OP_NAV_BACKWARD].enabled;
}

static inline BOOL is_zoomin_enabled()
{
    return default_toolbar[OP_NAV_ZOOMIN - OP_NAV_BACKWARD].enabled;
}
static inline void enable_backward(BOOL enable)
{
    default_toolbar[OP_NAV_BACKWARD- OP_NAV_BACKWARD].enabled = enable;
    refresh_menu_item(OP_NAV_BACKWARD- OP_NAV_BACKWARD);
}
static inline void enable_forward(BOOL enable)
{
    default_toolbar[OP_NAV_FORWARD- OP_NAV_BACKWARD].enabled = enable;
    refresh_menu_item(OP_NAV_FORWARD- OP_NAV_BACKWARD);
}
static inline void enable_home(BOOL enable)
{
    default_toolbar[OP_NAV_HOME- OP_NAV_BACKWARD].enabled = enable;
    refresh_menu_item(OP_NAV_HOME- OP_NAV_BACKWARD);
}
static inline void enable_stop(BOOL enable)
{
    default_toolbar[OP_NAV_STOP- OP_NAV_BACKWARD].enabled = enable;
    refresh_menu_item(OP_NAV_STOP- OP_NAV_BACKWARD);
}



static inline void enable_ime(BOOL enable)
{
	default_toolbar[OP_NAV_IME - OP_NAV_BACKWARD].enabled=enable;
    refresh_menu_item(OP_NAV_IME - OP_NAV_BACKWARD);
}
static inline void enable_zoomout(BOOL enable)
{
    default_toolbar[OP_NAV_ZOOMOUT - OP_NAV_BACKWARD].enabled = enable;
    refresh_menu_item(OP_NAV_ZOOMOUT - OP_NAV_BACKWARD);
}

static inline void enable_drag(BOOL enable)
{
    default_toolbar[OP_NAV_DRAG - OP_NAV_BACKWARD].enabled = enable;
    refresh_menu_item(OP_NAV_DRAG - OP_NAV_BACKWARD);
}

static inline void enable_zoomin(BOOL enable)
{
    default_toolbar[OP_NAV_ZOOMIN - OP_NAV_BACKWARD].enabled = enable;
    refresh_menu_item(OP_NAV_ZOOMIN - OP_NAV_BACKWARD);
}

static void change_progress_bar()
{
	RECT rect;
	int current = look_for_current_id(g_current_hwnd);
	SetRect(&rect, TAB_BEGIN_X+TAB_NORMAL_W*current, TAB_BEGIN_Y,
				TAB_BEGIN_X+TAB_NORMAL_W*current+TAB_CURRENT_W, TAB_BEGIN_Y+TAB_NORMAL_H);
	InvalidateRect(g_tb_hwnd, &rect, FALSE);
}

void change_loading_menu_status(BOOL load, unsigned int progress)
{
	int id = OP_NAV_STOP-OP_NAV_BACKWARD;
	if (load != default_toolbar[id].enabled)  {
		default_toolbar[id].enabled = load;
		refresh_menu_item(id);
	}
	change_progress_bar();
}

void change_history_menu_status(BOOL back, BOOL forward)
{
	int id = OP_NAV_BACKWARD-OP_NAV_BACKWARD;
	if (back != default_toolbar[id].enabled) {
		default_toolbar[id].enabled = back;
		refresh_menu_item(id);
	}

	id = OP_NAV_FORWARD-OP_NAV_BACKWARD;
	if (forward != default_toolbar[id].enabled)  {
		default_toolbar[id].enabled = forward;
		refresh_menu_item(id);
	}
}

// change ime status on toolbar
BOOL change_ime_menu_status(BOOL show)
{
	int id = OP_NAV_IME-OP_NAV_BACKWARD;
	if (show != default_toolbar[id].enabled) {
		default_toolbar[id].enabled = show;
		refresh_menu_item(id);
	}
	return show;
}

static inline BOOL is_ime_enabled()
{
    return default_toolbar[OP_NAV_IME - OP_NAV_BACKWARD].enabled;
}

#define SFKBD_REQID (MAX_SYS_REQID + 9)

void show_ime_window(BOOL show)
{
    int ime_status ;
// for  base-pthread minigui
#if !defined(_LITE_VERSION) && !defined(_MGRM_PROCESSES) && !defined(_STAND_ALONE)
    if (show) {
        // set active and focus to current mdolphin's window
        SetActiveWindow(GetParent(g_current_hwnd));
        SetFocusChild(g_current_hwnd);
        // set target window of IME and show IME window
        SendMessage(g_ime_hwnd, MSG_IME_SETTARGET, (WPARAM)GetParent(g_current_hwnd), 0);
        SendMessage(g_ime_hwnd, MSG_IME_OPEN, 0, 0);
    } else{
        ime_status = SendMessage( g_ime_hwnd, MSG_IME_GETSTATUS, 0, 0);
        if( ime_status ){
            SendMessage( g_ime_hwnd, MSG_IME_CLOSE, 0, 0);
        }
    }
#else
    int ret, msg;
    REQUEST req;
    req.id =SFKBD_REQID;
    req.len_data = sizeof(int);
    if (show) {
        SendMessage(g_current_hwnd, MSG_SETFOCUS, 0, 0);
        msg = MSG_IME_SETTARGET;
        req.data =&msg;
        ClientRequest(&req, &ret, sizeof(int));
        msg = 1; // MSG_IME_OPEN;
        req.data =&msg;
        ClientRequest(&req, &ret, sizeof(int));
        //SendMessage(g_ime_hwnd, MSG_IME_SETTARGET, (WPARAM)g_current_hwnd, 0);
        //SendMessage(g_ime_hwnd, MSG_IME_OPEN, 0, 0);
    } 
    else 
    {
        msg = 0 ; //MSG_IME_CLOSE;
        req.data =&msg;
        ClientRequest(&req, &ret, sizeof(int));
        //SendMessage(g_ime_hwnd, MSG_IME_CLOSE, 0, 0);
    }
#endif

}

static void draw_tool_bar_menu(HDC hdc)
{
	int col, row, tabWidth;
	int itemWidth = toolBarBmp.bmWidth /MENU_ITEM_NUM;
	int itemHeight = toolBarBmp.bmHeight /MENU_ITEM_STATUS;

	for(col =0; col<MENU_ITEM_NUM; col++) {
		row = default_toolbar[col].enabled ? 2:0;
		if (row && default_toolbar[col].highlight) 
			row =1;
		tabWidth = (col >3)?TOOLBAR_WIDTH - toolBarBmp.bmWidth:0;

#if 0
		if ((col == 4) && (g_mode != MD_SMALLVIEW_MODE)) 
			FillBoxWithBitmapPart(hdc, col*itemWidth+tabWidth, 0, itemWidth, itemHeight, 
					normalBmp.bmWidth, normalBmp.bmHeight, &normalBmp, 0, row*itemHeight);
		else
#endif 
			FillBoxWithBitmapPart(hdc, col*itemWidth+tabWidth, 0, itemWidth, itemHeight, 
					toolBarBmp.bmWidth, toolBarBmp.bmHeight, &toolBarBmp, col*itemWidth, row*itemHeight);
	}
}

static void draw_progress_bar(HDC hdc)
{
	int current = look_for_current_id(g_current_hwnd);
	TAB_INFO *info = (TAB_INFO *)GetWindowAdditionalData(g_current_hwnd);
	if(info && info->progress && info->load) {
		int x = TAB_BEGIN_X+TAB_NORMAL_W*current;
		int w = (int)(TAB_CURRENT_W*info->progress/100);
		FillBoxWithBitmap(hdc, x+1, TAB_BEGIN_Y+1, w-1, TAB_NORMAL_H-1, &progressBmp);
	}
}

static unsigned int get_title_max_num(const char *title, int width)
{
	SIZE size;
	LOGFONT oldFont;
	
	HDC hdc = GetClientDC(g_tb_hwnd);
	GetLogFontInfo(hdc, &oldFont);
	SelectFont(hdc, titleFont);

	int len = strlen(title);
	GetTextExtent(hdc, title, len, &size);
	if (size.cx > width) {
		while (len) {
			len--;
			GetTextExtent(hdc, title, len, &size);
			if (size.cx < width)
				break;
		}
	}

	SelectFont(hdc, &oldFont);
	ReleaseDC(hdc);
	return len;
}

static int utf8_len_first_char (const unsigned char* mstr, int len)
{
    int t, c = *((unsigned char *)(mstr++));
    int n = 1, ch_len = 0;

    /*for ascii character*/
    if (c < 0x80) {
        return 1;
    }

    if (c & 0x80) {
        while (c & (0x80 >> n))
            n++;

        if (n > len)
            return 0;

        ch_len = n;
        while (--n > 0) {
            t = *((unsigned char *)(mstr++));

            if ((!(t & 0x80)) || (t & 0x40))
                return 0;
        }
    }

    return ch_len;
}

static unsigned int find_title_valid_max_len (const char* title, int maxLen)
{
	int len = strlen(title);
	int pos = 0;
	int offset = 0;
	while(len-pos){
		offset = utf8_len_first_char ((const unsigned char*)title+pos, len-pos);
		if ((pos+offset)>maxLen)
			break;
		pos += offset; 
	}
	return pos;
}

static unsigned int init_tab_bar_title(const char *text)
{
	unsigned int ret;
	unsigned int oLen = strlen(text); 
	unsigned int rLen = get_title_max_num(text, TAB_CURRENT_W-TAB_CLOSE_W-15);
	if (oLen != rLen) 
		ret = find_title_valid_max_len(text, rLen);
	else 
		ret = rLen;

	return ret;
}

static void change_tab_bar_title(unsigned int offset, BOOL init)
{	
	TAB_INFO *info = (TAB_INFO *)GetWindowAdditionalData(g_current_hwnd);
	if (!info || !info->title) 
		return;

	g_title_text = (char *)realloc(g_title_text, info->title_len+1);
	if (!g_title_text)
		return;
	memset(g_title_text, 0, info->title_len+1);

	unsigned int len = strlen(info->title);
	if (init)
		info->title_begin = 0;
	else {
		if ((info->title_begin+offset)<len)
			info->title_begin += offset;
		else
			info->title_begin = 0;
	}

	if (len == info->title_len) {
		strncpy(g_title_text, info->title, len);
		g_title_text[info->title_len]='\0';
	} else {
		unsigned int left = len - info->title_begin;
		if (left > info->title_len){
			unsigned int valid = find_title_valid_max_len(info->title + info->title_begin, info->title_len);
			strncpy(g_title_text, info->title + info->title_begin, valid);
			g_title_text[info->title_len]='\0';
		}else {
			unsigned int valid = find_title_valid_max_len(info->title + info->title_begin, left);
			strncpy(g_title_text, info->title + info->title_begin, valid);
			g_title_text[info->title_len]='\0';
		}
	}
}

static unsigned int find_title_next_break_pos(void)
{
	TAB_INFO *info = (TAB_INFO *)GetWindowAdditionalData(g_current_hwnd);
	if (!info || !info->title) 
		return 0;

	unsigned int offset = 0;
	unsigned int len = strlen(info->title);
	char *text = (char *)malloc(len+1);
	if (text) {
		memset(text, 0, len+1);
		unsigned int pos = info->title_begin;
		strncpy(text, info->title+pos, len-pos);
		text[len]='\0';
		offset = utf8_len_first_char ((const unsigned char*)text, strlen(text));
		free(text);
	}
	return offset;
}

static void refresh_tab_bar_title(void)
{
	RECT rect;
	int current = look_for_current_id(g_current_hwnd);
	SetRect(&rect, TAB_BEGIN_X+TAB_NORMAL_W*current, TAB_BEGIN_Y,
				TAB_BEGIN_X+TAB_NORMAL_W*current+TAB_CURRENT_W, TAB_BEGIN_Y+TAB_NORMAL_H);
	InvalidateRect(g_tb_hwnd, &rect, FALSE);
}

void update_tab_bar_title()
{
	TAB_INFO *info = (TAB_INFO *)GetWindowAdditionalData(g_current_hwnd);
	if (!info || !info->title) 
		return;

	info->title_len = init_tab_bar_title(info->title);
	change_tab_bar_title(0, TRUE);
	refresh_tab_bar_title();
}

static void draw_title(HDC hdc)
{
#ifdef WITH_TWO_TAB
    LOGFONT oldFont;
    RECT rect;
    int tab_id;
    GetLogFontInfo(hdc, &oldFont);
    SelectFont(hdc, titleFont);
    SetBkMode(hdc, BM_TRANSPARENT); 
	if (g_title_text){
        // draw current tab window's text
        tab_id = look_for_current_id(g_current_hwnd);
		SetRect(&rect, TAB_BEGIN_X+TAB_NORMAL_W*tab_id+5, TAB_BEGIN_Y,
				TAB_BEGIN_X+TAB_NORMAL_W*tab_id+TAB_CURRENT_W-TAB_CLOSE_W-15, TAB_BEGIN_Y+TAB_NORMAL_H);

		DrawText(hdc, g_title_text, -1, &rect, DT_NOCLIP | DT_LEFT |  DT_VCENTER | DT_SINGLELINE);
	}
    // draw inactive tab window's text
    {
        HWND hWnd_inactive = look_for_inactive_tab_wnd();
        if(hWnd_inactive != HWND_NULL){
            TAB_INFO *info = (TAB_INFO *)GetWindowAdditionalData(hWnd_inactive);
            if(info && info->title){
                tab_id = look_for_current_id(hWnd_inactive);
                SetRect(&rect, TAB_BEGIN_X+TAB_NORMAL_W*tab_id+5, TAB_BEGIN_Y,
                        TAB_BEGIN_X+TAB_NORMAL_W*tab_id+TAB_CURRENT_W-TAB_CLOSE_W-15, TAB_BEGIN_Y+TAB_NORMAL_H);
                DrawText(hdc, info->title, -1, &rect, DT_LEFT |  DT_VCENTER | DT_SINGLELINE);
            }
        }
    }
    SetBkMode(hdc, BM_OPAQUE); 
    SelectFont(hdc, &oldFont);
#else
    LOGFONT oldFont;
    if (g_title_text) {
        RECT rect;
        int current = look_for_current_id(g_current_hwnd);
        SetRect(&rect, TAB_BEGIN_X+TAB_NORMAL_W*current+5, TAB_BEGIN_Y,
                TAB_BEGIN_X+TAB_NORMAL_W*current+TAB_CURRENT_W-TAB_CLOSE_W-15, TAB_BEGIN_Y+TAB_NORMAL_H);

        GetLogFontInfo(hdc, &oldFont);
        SelectFont(hdc, titleFont);
        SetBkMode(hdc, BM_TRANSPARENT);
        DrawText(hdc, g_title_text, -1, &rect, DT_NOCLIP | DT_LEFT |  DT_VCENTER | DT_SINGLELINE);
        SetBkMode(hdc, BM_OPAQUE);
        SelectFont(hdc, &oldFont);
    }
#endif
}

static int select_tab_color(int i)
{
	int ret;
	if (g_hwnd[i] == HWND_NULL)
		ret = 0;/*light white*/
	else if (g_hwnd[i] == g_current_hwnd)
		ret = 2;/*yellow*/
	else
		ret = 1;/*blue*/
	return ret;
}

static void draw_close_button(HDC hdc, int id, int tab_status)
{
	int close_status = g_close[id]?1:0;
	int itemHeight = (tabBmp.bmHeight/3);
	// display the close button zjwang 
	//BOOL close = (look_for_used_tab_num()>1)?TRUE:FALSE;
	BOOL close = (look_for_used_tab_num()>0)?TRUE:FALSE;
	if (close && (g_hwnd[id]!= HWND_NULL)) {
		int x = TAB_BEGIN_X+TAB_NORMAL_W*id;
#if WITH_TWO_TAB
		FillBoxWithBitmapPart(hdc, x+TAB_CURRENT_W-TAB_CLOSE_W, TAB_BEGIN_Y, TAB_CLOSE_W, TAB_CLOSE_H, 
				tabBmp.bmWidth, tabBmp.bmHeight, &tabBmp, 
				256+TAB_CLOSE_W*close_status, tab_status*itemHeight);
#else
		FillBoxWithBitmapPart(hdc, x+TAB_CURRENT_W-TAB_CLOSE_W, TAB_BEGIN_Y, TAB_CLOSE_W, TAB_CLOSE_H, 
				tabBmp.bmWidth, tabBmp.bmHeight, &tabBmp, 
				TAB_CURRENT_W+TAB_CLOSE_W*close_status, tab_status*itemHeight);
#endif
	}
}
static void draw_each_tab_bar(HDC hdc, int id)
{
	int tab_status = select_tab_color(id);
	int itemHeight = (tabBmp.bmHeight/3);
	int x = TAB_BEGIN_X+TAB_NORMAL_W*id;
#ifdef WITH_TWO_TAB
	FillBoxWithBitmapPart(hdc, x, TAB_BEGIN_Y, TAB_CURRENT_W, TAB_NORMAL_H, 
			tabBmp.bmWidth - 29, tabBmp.bmHeight, &tabBmp, 0, tab_status*itemHeight);
#else
	FillBoxWithBitmapPart(hdc, x, TAB_BEGIN_Y, TAB_CURRENT_W, TAB_NORMAL_H, 
			tabBmp.bmWidth, tabBmp.bmHeight, &tabBmp, 0, tab_status*itemHeight);
#endif
	draw_close_button(hdc, id, tab_status);
}

static void draw_tab_bar(HDC hdc)
{
	int i;
#ifdef WITH_TWO_TAB
	/*draw tab page before the current page*/
	for(i = 0; i < TABCOUNT; i++)
		draw_each_tab_bar(hdc, i);
#else
	int current = look_for_current_id(g_current_hwnd);
	/*draw tab page before the current page*/
	for(i = 0; i < current; i++)
		draw_each_tab_bar(hdc, i);

	/*draw tab page after the current page*/
	for(i = TABLESIZE(g_hwnd)-1; i >current; i--)
		draw_each_tab_bar(hdc, i);

	/*draw current page*/
	draw_each_tab_bar(hdc, current);
#endif
	draw_progress_bar(hdc);
	draw_title(hdc);
}

static void draw_tab_bar_bkg(HDC hdc)
{
	FillBoxWithBitmap(hdc, 0, 0, TOOLBAR_WIDTH, TOOLBAR_HEIGHT,&bkBmp);	
}


static int look_for_tab_index_by_click(int x, int y, BOOL *close)
{
	int i, left, right;
	RECT rect, small;
	*close = FALSE;
	int current = look_for_current_id(g_current_hwnd);

	for (i = 0; i < TABCOUNT; i++) {
		if (i <= current)
			left = i*TAB_NORMAL_W;
		else
			left = (i-1)*TAB_NORMAL_W+TAB_CURRENT_W;

		if (i < current)
			right = i*TAB_NORMAL_W+TAB_NORMAL_W;
		else
			right = i*TAB_NORMAL_W+TAB_CURRENT_W;

		SetRect(&rect, TAB_BEGIN_X+left, TAB_BEGIN_Y, TAB_BEGIN_X+right, TAB_BEGIN_Y+TAB_NORMAL_H);
		if (PtInRect(&rect, x, y)) {
			//if ((g_hwnd[i] != HWND_NULL) && (look_for_used_tab_num()>1) && (i >= current)) {
			if ((g_hwnd[i] != HWND_NULL) && (look_for_used_tab_num()>0)) {
				SetRect(&small, TAB_BEGIN_X+right-TAB_CLOSE_W , TAB_BEGIN_Y, 
						TAB_BEGIN_X+right, TAB_BEGIN_Y+TAB_CLOSE_H);
				if (PtInRect(&small, x, y))
					*close = TRUE;
			}
			break;
		}
	}
	return i;
}

static void	update_toolbar_info()
{
	TAB_INFO *info = (TAB_INFO *)GetWindowAdditionalData(g_current_hwnd);

	if (info) {	
		int id = OP_NAV_BACKWARD-OP_NAV_BACKWARD;
		BOOL current = info->back? TRUE:FALSE;
		if (current != default_toolbar[id].enabled) {
			default_toolbar[id].enabled = current;
			refresh_menu_item(id);
		}

		id = OP_NAV_FORWARD-OP_NAV_BACKWARD;
		current = info->forward? TRUE:FALSE;
		if (current != default_toolbar[id].enabled) {
			default_toolbar[id].enabled = current;
			refresh_menu_item(id);
		}

		id = OP_NAV_STOP-OP_NAV_BACKWARD;
		current = info->load? TRUE:FALSE;
		if (current != default_toolbar[id].enabled) {
			default_toolbar[id].enabled = current;
			refresh_menu_item(id);
		}
		
#if 0
		RENDERING_MODE mode;
		mdolphin_get_rendering_mode(g_current_hwnd, &mode);
		g_mode = mode;

		if (mode == MD_SCREENVIEW_MODE) {
			enable_zoomin(TRUE);
			enable_drag(TRUE);
			enable_zoomout(TRUE);
		}else if(mode == MD_SMALLVIEW_MODE) {
			enable_zoomin(TRUE);
			enable_drag(FALSE);
			enable_zoomout(TRUE);
		}else if (mode == MD_VIRTUALVIEW_MODE) {
			enable_zoomin(FALSE);
			enable_drag(TRUE);
			enable_zoomout(FALSE);
		}

#endif
		if (g_title_text)
			memset(g_title_text, 0, strlen(g_title_text));
	}
}

void update_tool_bar(void)
{
	update_toolbar_info();
	InvalidateRect(g_tb_hwnd, &(default_toolbar[OP_TAB_PAGE-OP_NAV_BACKWARD].rect), FALSE);
}

static void click_tab_page_bar(int x, int y)
{
	BOOL close;
	int click = look_for_tab_index_by_click(x, y, &close);
	if(click>=TABCOUNT)
		return;
	if ((g_hwnd[click] == g_current_hwnd) && !close && (g_current_hwnd != HWND_NULL))
		return;
    navigate_compass_showwindow();
	update_mdolphin_window(click, close, dragflag);
}

static void highlight_current_menu(int toolbar_id, BOOL high)
{
	if (toolbar_id > OP_BEGIN  && toolbar_id < OP_TAB_PAGE)  {
		int id = toolbar_id - OP_NAV_BACKWARD;
		default_toolbar[id].highlight = high;
		refresh_menu_item(id);
	}
}

static void highlight_close_button(int x, int y, BOOL high)
{
	BOOL close;
	int click = look_for_tab_index_by_click(x, y, &close);
	if (!close)
		return;

	if (high){
		RECT small;
		int x = TAB_BEGIN_X+TAB_NORMAL_W*click;
		*(&(g_close[click])) = TRUE;
		SetRect(&small, x+TAB_CURRENT_W-TAB_CLOSE_W, TAB_BEGIN_Y, x+TAB_CURRENT_W,  TAB_BEGIN_Y+TAB_CLOSE_H);
		InvalidateRect(g_tb_hwnd, &small, FALSE);
	}else {
		memset(g_close, 0, sizeof(g_close));
	}
}
const int STEP=9;
static int zoomfactor[STEP]={30,50,80,100,120,150,200,300,400};
static void page_zoom(HWND hPageWnd,int zoom)
{
	if(!hPageWnd)
		return;
	enable_zoomin(false);
	enable_zoomout(false);
	int percent;
	mdolphin_get_text_size_multiplier(hPageWnd, &percent);
	int i=0;
	switch(zoom){
		case ZOOMIN:
			for(i=0;i<STEP;++i)
				if(percent<zoomfactor[i])
					break;
			if(i<STEP)
				percent=zoomfactor[i];
			else // the max 
				percent=400;
				
			mdolphin_set_text_size_multiplier(hPageWnd, percent);
			break;

		case ZOOMOUT:
			for(i=STEP-1;i>0;--i)
				if(percent>zoomfactor[i])
					break;
			if(i>0)
				percent=zoomfactor[i];
			else // the min 
				percent=30;
		mdolphin_set_text_size_multiplier(hPageWnd, percent);
			break;
		default:
			break;
	}
	
	if(percent<300)
	    enable_zoomin(true);
	if(percent>50)
		enable_zoomout(true);
	
	return;
}

static POINT s_tab_cur_pos[TABCOUNT];

/*get cur id */
static int get_cur_id(HWND hwnd)
{
	return look_for_current_id(hwnd);
}

enum{
    GET_POSITION,
    SET_POSITION
};

/*get cur coordinate*/
static void cur_postition(HWND hwnd, int index)
{
    int cur_id = 0;
    int tab_i = 0;
    cur_id = get_cur_id(hwnd);
    switch(index)
    {
        case GET_POSITION: 
            for(tab_i=0; tab_i<TABCOUNT; tab_i++)
            {
                if(g_hwnd[cur_id] != HWND_NULL && tab_i == cur_id)
                    mdolphin_get_contents_position(hwnd, &s_tab_cur_pos[cur_id].x, &s_tab_cur_pos[cur_id].y);
            }
            break;
            
        case SET_POSITION:
            mdolphin_set_contents_position(hwnd, s_tab_cur_pos[cur_id].x, s_tab_cur_pos[cur_id].y);
            break;

        default:
            break;
    }
}

static int setDragMode()
{
	dragflag=!dragflag;
	if(dragflag){
		//enable_backward(FALSE);
		//enable_forward(FALSE);
		//enable_home(FALSE);
		enable_stop(FALSE);
        enable_zoomin(FALSE);
        enable_zoomout(FALSE);
        change_ime_menu_status(FALSE);

		highlight_current_menu(OP_NAV_DRAG, TRUE);
        cur_postition(g_current_hwnd, GET_POSITION);
        navigate_compass_window(TRUE);
	}else{
		//enable_home(TRUE);
		enable_zoomin(TRUE);
		enable_zoomout(TRUE);
		enable_drag(TRUE);
        // change ime status on toolbar
        change_ime_menu_status(get_tab_window_ime_status(g_current_hwnd));

		update_toolbar_info();
        navigate_compass_window(FALSE);
        SetActiveWindow(GetParent(g_current_hwnd));
	}
    return dragflag;
}

#if 0
static void change_global_mode(void)
{
	RENDERING_MODE mode;
	int percent=0;
	mdolphin_get_text_size_multiplier(g_current_hwnd, &percent);
	if(percent!=100){
		percent=100;
		mdolphin_set_text_size_multiplier(g_current_hwnd, percent);
	}

	mdolphin_get_rendering_mode(g_current_hwnd, &mode);

	if (mode == MD_SCREENVIEW_MODE) {
		int width, height;
		mdolphin_get_contents_size(g_current_hwnd, &width, &height);
		if (width)
			g_scale_width = width;
		else
			g_scale_width = BROWSER_WIDTH;

		enable_backward(FALSE);
		enable_forward(FALSE);
		enable_home(FALSE);
		enable_stop(FALSE);
		enable_zoomin(FALSE);
		enable_zoomout(FALSE);
		if (g_scale_width < BROWSER_WIDTH)
			g_scale_width = BROWSER_WIDTH;
		mdolphin_set_rendering_mode(g_current_hwnd, MD_VIRTUALVIEW_MODE, BROWSER_WIDTH*800/g_scale_width);
		g_mode = MD_VIRTUALVIEW_MODE;
	} else if (mode == MD_VIRTUALVIEW_MODE) {

		update_toolbar_info();
		enable_home(TRUE);
		enable_zoomin(TRUE);
		enable_zoomout(TRUE);
		mdolphin_set_rendering_mode(g_current_hwnd, MD_SCREENVIEW_MODE, BROWSER_WIDTH);
		g_mode = MD_SCREENVIEW_MODE;
	}
	enable_drag(true);
}
#endif

static void deal_with_key_event(int message, int toolbar_id)
{
	static int s1_menu_item = 0;
    if(g_err_hwnd_show == true)
        return;
    if(message == MSG_KEYUP)
    {
		if (s1_menu_item) {
			highlight_current_menu(s1_menu_item, FALSE);
			BOOL ret = (s1_menu_item == toolbar_id)?FALSE:TRUE;
			s1_menu_item = 0;
			if(ret)
				return;
		}
    }else{
		highlight_current_menu(toolbar_id, TRUE);
		s1_menu_item = toolbar_id;
		return;
    }

    if(toolbar_id < OP_NAV_BACKWARD || toolbar_id >OP_TAB_PAGE)
        return;

    switch(toolbar_id)
    {
        case OP_NAV_DRAG:
            setDragMode();
            break;
        default:
            break;
    }
}

static void deal_with_mouse_event(int message, WPARAM wParam, LPARAM lParam)
{
	int x = LOSWORD (lParam);
	int y = HISWORD (lParam);
	int toolbar_id = OP_BEGIN;

    if(g_err_hwnd_show == true)
        return;

	ClientToScreen(g_tb_hwnd, &x, &y);
	for (unsigned i = 0; i < TABLESIZE(default_toolbar); i++) {
		if (PtInRect(&default_toolbar[i].rect, x, y)) {
			toolbar_id = default_toolbar[i].id;
			break;
		}
	}

	static int s_close_x = 0;
	static int s_close_y = 0;
	static int s_menu_item = 0;
	if (message == MSG_LBUTTONUP) {
		if (s_close_x && s_close_y) 
        {
			highlight_close_button(s_close_x, s_close_y, FALSE);
			s_close_x = 0;
			s_close_y = 0;
		}
		if (s_menu_item) {
			highlight_current_menu(s_menu_item, FALSE);
			BOOL ret = (s_menu_item == toolbar_id)?FALSE:TRUE;
			s_menu_item = 0;
			if(ret)
				return;
		}
	} else {
		highlight_current_menu(toolbar_id, TRUE);
		highlight_close_button(x, y, TRUE);
		s_close_x = x;
		s_close_y = y;
		s_menu_item = toolbar_id;
		return;
	}

	if (toolbar_id < OP_NAV_BACKWARD || toolbar_id >OP_TAB_PAGE) 
		return;
	int percent=0;
	switch (toolbar_id) {
		case OP_NAV_BACKWARD:
			if(is_backward_enabled()){
				mdolphin_navigate(g_current_hwnd, NAV_BACKWARD, NULL, FALSE);
				mdolphin_get_text_size_multiplier(g_current_hwnd, &percent);
				if(percent!=100){
					percent=100;
					mdolphin_set_text_size_multiplier(g_current_hwnd, percent);
				}
				change_ime_menu_status(FALSE);
			}
			break;
		case OP_NAV_FORWARD:
			if(is_forward_enabled()){
				mdolphin_navigate(g_current_hwnd, NAV_FORWARD, NULL, FALSE);
				mdolphin_get_text_size_multiplier(g_current_hwnd, &percent);
				if(percent!=100){
					percent=100;
					mdolphin_set_text_size_multiplier(g_current_hwnd, percent);
				}
				change_ime_menu_status(FALSE);
			}
			break;
		case OP_NAV_HOME:

			if(is_home_enabled()){
				mdolphin_navigate(g_current_hwnd, NAV_GOTO, home_url, FALSE);
				mdolphin_get_text_size_multiplier(g_current_hwnd, &percent);
				if(percent!=100&&percent!=0){
					percent=100;
					mdolphin_set_text_size_multiplier(g_current_hwnd, percent);
				}

				change_ime_menu_status(FALSE);
			}
			break;
		case OP_NAV_STOP:
			if(is_stop_enabled()){
				mdolphin_navigate(g_current_hwnd, NAV_STOP, NULL, FALSE);
			}
			break;
		case OP_NAV_ZOOMIN:
#if 0
			if(is_smart_enabled())
				change_smart_mode();
#endif
			if(is_zoomin_enabled())
					page_zoom(g_current_hwnd,ZOOMIN);


			//ShowWindow(g_busy_hwnd, SW_SHOWNORMAL);
			break; 	
        case OP_NAV_DRAG:
			//if(is_drag_enabled())
			setDragMode();
				//change_global_mode();
            break; 	
        case OP_NAV_ZOOMOUT:
			if(is_zoomout_enabled())
				page_zoom(g_current_hwnd,ZOOMOUT);
#if 0
            if (is_magnifier_enabled())
                show_magnifier(g_tb_hwnd, g_current_hwnd);
#endif
            break;
        case OP_NAV_IME:
            if (is_ime_enabled()){
                show_ime_window(!g_ime_opened);
            }
            break;
        case OP_TAB_PAGE:
            click_tab_page_bar(x, y);
            break;
        default:
            break;
    }
}

static HDC tb_mem_dc = HDC_INVALID;

static void create_tool_bar_mem_dc(HWND hwnd)
{
    HDC hdc = GetClientDC(hwnd);
    tb_mem_dc = CreateCompatibleDCEx (hdc, TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
    ReleaseDC(hdc);
}

enum{
    move_left,
    move_right,
    move_up,
    move_down,
};

static void  move_key_browser(int key_id, int scrollbar_move_speed)
{
       int x_pos = 0;
       int y_pos = 0 ;
        mdolphin_get_contents_position(g_current_hwnd, &x_pos, &y_pos);
        switch(key_id){
            case move_left:
                x_pos -= scrollbar_move_speed ;
                break;
            case move_right:
                x_pos += scrollbar_move_speed ;
                break;
            case move_up:
                y_pos -= scrollbar_move_speed ;
                break;
            case move_down:
                y_pos += scrollbar_move_speed;
                break;
            default:
                break;
        }
        mdolphin_set_contents_position(g_current_hwnd, x_pos, y_pos);
}

static int chose_page = 1;
static bool close_id ;
static int alwaysKeyPressCnt = 0;

static int TBWinProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    RECT    rc1;
    int     speedy = 0;
    int     speedx = 0;
    MSG msg;

    if( GetWindowRect(GetParent(g_current_hwnd), &rc1) )
    {
        speedy = RECTH(rc1);
        speedx = RECTW(rc1);
    }
	switch (message) {
		case MSG_CREATE:
            if (!g_is_big_screen)
            {
                if (LoadBitmap(HDC_SCREEN, &bkBmp, "res/toolbar_bk.png")) 
                    return -1;
                if (LoadBitmap(HDC_SCREEN, &toolBarBmp, "res/toolbar.png")) 
                    return -1;
                if (LoadBitmap(HDC_SCREEN, &progressBmp, "res/progress_bar.png")) 
                    return -1;
                if (LoadBitmap(HDC_SCREEN, &tabBmp, "res/tab.png")) 
                    return -1;
#if 0
                if (LoadBitmap(HDC_SCREEN, &normalBmp, "res/normal.png")) 
                    return -1;
#endif
            }
            else
            {
                if (LoadBitmap(HDC_SCREEN, &bkBmp, "res-800/toolbar_bk.png")) 
                    return -1;
                if (LoadBitmap(HDC_SCREEN, &toolBarBmp, "res-800/toolbar.png")) 
                    return -1;
                if (LoadBitmap(HDC_SCREEN, &progressBmp, "res-800/progress_bar.png")) 
                    return -1;
                if (LoadBitmap(HDC_SCREEN, &tabBmp, "res-800/tab.png")) 
                    return -1;
#if 0
                if (LoadBitmap(HDC_SCREEN, &normalBmp, "res-800/normal.png")) 
                    return -1;
#endif
            }
            titleFont = CreateLogFont(NULL, "fmkai", "UTF-8",
                    FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL, FONT_OTHER_NIL,
                    FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, TITLE_TXT_HEIGHT, 0);

            if (g_is_big_screen)
                set_toolbar_layout();

            SetTimer(hwnd, TITLE_TIMER_ID, 60);
            create_tool_bar_mem_dc(hwnd);
            return 0;
		case MSG_MOUSEMOVE:
			break;
		case MSG_LBUTTONDOWN:
			SetCapture(hwnd);
			deal_with_mouse_event(message, wParam, lParam);
            break;
		case MSG_LBUTTONUP:
			ReleaseCapture();
			deal_with_mouse_event(message, wParam, lParam);
            break;
        case MSG_KEYUP:
            alwaysKeyPressCnt = 0;
            SendMessage(g_current_hwnd, message, wParam, lParam);
            switch(wParam)
            {
                case SCANCODE_F8:
                    ReleaseCapture();
                    deal_with_key_event(message, OP_NAV_DRAG);
                    break;
                default:
                    break;
            }
            break;

        case MSG_KEYDOWN:
            while(PeekMessage(&msg, hwnd, MSG_KEYDOWN, MSG_KEYDOWN, PM_REMOVE))
            {
             //  printf("---------- PeekMessage()\n");
            }
            switch(wParam)
            {
                case SCANCODE_F5:
                    chose_page = get_cur_id(g_current_hwnd);
                    if ((g_hwnd[chose_page] != g_current_hwnd) && !close_id && (g_current_hwnd != HWND_NULL))
                    {
                        return 0;
                    }
                    if(chose_page == 1)
                    {
                        chose_page=0;

                    }else {
                        chose_page=1;
                    }
                    update_mdolphin_window(chose_page, close_id, dragflag);
                    navigate_compass_showwindow();
                    break;

                case SCANCODE_F7:
                    if(is_zoomout_enabled())
                        page_zoom(g_current_hwnd,ZOOMOUT);
                    break;

                case SCANCODE_F6:
                    if(is_zoomin_enabled())
                        page_zoom(g_current_hwnd,ZOOMIN);
                    break;

                case SCANCODE_F8:
                    SetCapture(hwnd);
                    deal_with_key_event(message, OP_NAV_DRAG);
                    break;

                case SCANCODE_ENTER:
                    if(!dragflag)
                        return 0;
                    cur_postition(g_current_hwnd, SET_POSITION);
                    break;

                case SCANCODE_CURSORBLOCKLEFT:
                    if(!dragflag)
                        return 0;
                    ++alwaysKeyPressCnt;
                    if (alwaysKeyPressCnt > 3) {
                        move_key_browser(move_left, speedx/2);
                    } else {
                        move_key_browser(move_left, speedx/10);
                    }
                    break;
                case SCANCODE_CURSORBLOCKRIGHT:
                    if(!dragflag)
                        return 0;
                    ++alwaysKeyPressCnt;
                    if (alwaysKeyPressCnt > 3) {
                        move_key_browser(move_right, speedx/2);
                    } else {
                        move_key_browser(move_right, speedx/10);
                    }
                    break;
                case SCANCODE_CURSORBLOCKUP:
                    if(!dragflag)
                        return 0;
                    ++alwaysKeyPressCnt;
                    if (alwaysKeyPressCnt > 3) {
                        move_key_browser(move_up, speedy/3);
                    } else {
                        move_key_browser(move_up, speedy/10);
                    }
                    break;
                case SCANCODE_CURSORBLOCKDOWN:
                    if(!dragflag)
                        return 0;
                    ++alwaysKeyPressCnt;
                    if (alwaysKeyPressCnt > 3) {
                        move_key_browser(move_down, speedy/3);
                    } else {
                        move_key_browser(move_down, speedy/10);
                    }
                   break;
            }
            break;

        case MSG_RBUTTONDOWN:
            break;

        case MSG_DOESNEEDIME:
            return FALSE;
        case MSG_PAINT: 
			{
				HDC hdc = BeginPaint(hwnd);
				draw_tab_bar_bkg(tb_mem_dc);
				draw_tab_bar(tb_mem_dc);
				draw_tool_bar_menu(tb_mem_dc);
				BitBlt(tb_mem_dc, 0, 0, -1, -1, hdc, 0, 0, 0);
				EndPaint(hwnd, hdc);
				return 0;
			}
		case MSG_TIMER:
			{	
				unsigned int offset = find_title_next_break_pos();
				change_tab_bar_title(offset, FALSE);
				refresh_tab_bar_title();
				break;
			}
		case MSG_CLOSE:
			if (titleFont)
				DestroyLogFont(titleFont);
			UnloadBitmap(&bkBmp);
			UnloadBitmap(&toolBarBmp);
			UnloadBitmap(&progressBmp);
			UnloadBitmap(&tabBmp);
		//	UnloadBitmap(&normalBmp);

			KillTimer(hwnd , TITLE_TIMER_ID);
			if (tb_mem_dc)
				DeleteMemDC(tb_mem_dc);
			DestroyMainWindow(hwnd);
			PostQuitMessage(hwnd);
			return 0;
		case MSG_DESTROY:
			return 0;
	}
	return DefaultMainWinProc(hwnd, message, wParam, lParam);
}

HWND create_toobar_window(HWND host)
{
    HWND hTbWnd;
    MAINWINCREATE CreateInfo;

    CreateInfo.dwStyle      = WS_VISIBLE;
    CreateInfo.dwExStyle    = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
    CreateInfo.spCaption    = "toolbar";
    CreateInfo.hMenu        = 0;
    CreateInfo.hCursor      = GetSystemCursor (IDC_ARROW);
    CreateInfo.hIcon        = 0;
    CreateInfo.MainWindowProc = TBWinProc;
    CreateInfo.lx           = TOOLBAR_LEFT;
    CreateInfo.ty           = TOOLBAR_TOP;
    CreateInfo.rx           = TOOLBAR_WIDTH;
    CreateInfo.by           = TOOLBAR_HEIGHT;
    CreateInfo.iBkColor     = PIXEL_lightgray;
    CreateInfo.dwAddData    = (DWORD)0;
    CreateInfo.hHosting     = host;

    hTbWnd = CreateMainWindow(&CreateInfo);

    return hTbWnd;
}

#ifdef __cplusplus
}
#endif
