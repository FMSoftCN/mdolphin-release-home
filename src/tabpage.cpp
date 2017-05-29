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
#include "panpages.h"

#ifdef MINIGUI_V3
#include "magnifier3.h"
#else
#include "magnifier.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern HWND g_current_hwnd;
extern HWND g_ime_hwnd;
extern HWND g_hwnd[TABCOUNT];
extern HWND g_tb_hwnd;
extern const char* home_url;
extern PANNING_CTXT* g_panning_ctxt;

HWND lookup_window_by_tabinfo(const TAB_INFO* info)
{
    unsigned int i;
    if (!info)
        return HWND_NULL;

    for (i=0; i<TABLESIZE(g_hwnd); i++) {
        if (g_hwnd[i] != HWND_NULL && g_hwnd[i] != HWND_INVALID)
            if (GetWindowAdditionalData(g_hwnd[i]) == (DWORD)info)
                return g_hwnd[i];
    }
    return HWND_NULL;
}

int look_for_current_id(HWND hWnd)
{
	unsigned int i = -1;
	for(i=0; i<TABLESIZE(g_hwnd); i++){
		if ((g_hwnd[i] == hWnd) && (g_hwnd[i] != HWND_NULL)) 
			break;
	}
	return i;
}

#ifdef WITH_TWO_TAB
HWND look_for_inactive_tab_wnd(void)
{
    unsigned int i;
    for(i=0; i<TABLESIZE(g_hwnd); i++){
        if ((g_hwnd[i] != g_current_hwnd) && (g_hwnd[i] != HWND_NULL) && (g_hwnd[i] != HWND_INVALID)) 
            break;
    }
    if( i<TABLESIZE(g_hwnd) ){
        return g_hwnd[i];
    }else{
        return HWND_NULL;
    }
}
#endif

static int look_for_next_tab(int id)
{
	int i;
	for(i=id+1; i<TABCOUNT; i++){
		if(g_hwnd[i] != HWND_NULL)	
			goto end;
	}
	for(i=id-1; i>=0; i--){
		if(g_hwnd[i] != HWND_NULL)	
			goto end;
	}
end:
	return i;
}

static int look_for_unused_tab(HWND hWnd)
{
	int current = look_for_current_id(hWnd);
	int ret = current;
	int i;

	if (current>2) 
		goto reverse;

	for(i=1; (current+i)<TABCOUNT; i++){
		if (g_hwnd[current+i] == HWND_NULL){
			ret = current+i;
			break;
		}
	}

reverse:
	if (ret == current) {
		for(i=0; i<current; i++){
			if (g_hwnd[i] == HWND_NULL){
				ret = i;
				break;
			}
		}

	}

	return ret;
}

int look_for_used_tab_num( void)
{	
	int ret = 0;
	int i;
	for(i = 0; i < TABCOUNT; i++) {
		if(g_hwnd[i] != HWND_NULL)
			ret++;
	}
	return ret;
}

static void update_window_display(HWND hWnd)
{
	if (hWnd == HWND_NULL)
		return;

	if (g_current_hwnd != hWnd) {
		ShowWindow(hWnd, SW_SHOW);

		//disable ime toolbar icon
	       set_tab_window_ime_stauts(hWnd, false);
		change_ime_menu_status(false);

		if (g_current_hwnd != HWND_INVALID)
			ShowWindow(g_current_hwnd, SW_HIDE);

        /* start to pan pages */
        if (g_current_hwnd && hWnd) {
            g_panning_ctxt = create_panning_pages_context(hWnd, g_current_hwnd, GetParent(hWnd));
            int from = look_for_current_id(hWnd);
            int to = look_for_current_id(g_current_hwnd);
            if (from > to)
                begin_panning_pages(g_panning_ctxt, MOVETO_RIGHT, 6);
            else
                begin_panning_pages(g_panning_ctxt, MOVETO_LEFT, 6);
        }

        g_current_hwnd = hWnd;
        // SendMessage(g_ime_hwnd, MSG_IME_SETTARGET, (WPARAM)g_current_hwnd, 0);
        //SendMessage(g_ime_hwnd, MSG_IME_SETTARGET, (WPARAM)g_tb_hwnd, 0);
	}
	update_tool_bar();
}

void create_mdolphin_window(HWND parent, HWND *mdolphin_hwnd, const char* url)
{
    if (*mdolphin_hwnd) {
        mdolphin_navigate(*mdolphin_hwnd, NAV_STOP, NULL, FALSE);
        TAB_INFO * tabInfo = (TAB_INFO *)GetWindowAdditionalData(*mdolphin_hwnd);
        memset(tabInfo, 0, sizeof(TAB_INFO));
    } else {
	    *mdolphin_hwnd = CreateWindow (MDOLPHIN_CTRL,
		    	"",
			    WS_VISIBLE | WS_CHILD,
			    IDC_MDOLPHIN,
			    BROWSER_LEFT, BROWSER_TOP, BROWSER_WIDTH, BROWSER_HEIGHT, parent, 0);	
	    TAB_INFO * info = (TAB_INFO *)malloc(sizeof(TAB_INFO));
	    if (!info) {
		    fprintf(stderr, "malloc is error!\n");
		    return;
	    }
	    memset(info, 0, sizeof(TAB_INFO));
	    SetWindowAdditionalData(*mdolphin_hwnd, (DWORD)info);
    }

    if(url)
		mdolphin_navigate(*mdolphin_hwnd, NAV_GOTO, url, FALSE);

	update_window_display(*mdolphin_hwnd);
}

void my_close_tab_window(HWND hWnd)
{
	int id = look_for_current_id(hWnd);
	if (g_hwnd[id] == HWND_NULL)
		return; 

	mdolphin_navigate(g_hwnd[id], NAV_STOP, NULL, FALSE);

	if (g_current_hwnd == g_hwnd[id]) {
		int next = look_for_next_tab(id);
		//if there is only one tab page ,we open homepage instead
		if(next>=TABCOUNT||next<0){
			mdolphin_navigate(g_current_hwnd, NAV_GOTO, home_url, FALSE);
			return;
		}
	}

	TAB_INFO *info = (TAB_INFO *)GetWindowAdditionalData(g_hwnd[id]);
	if (info) {
		if (info->status)
			free(info->status);
		if (info->location)
			free(info->location);
		if (info->title)
			free(info->title);
		free(info);
		SetWindowAdditionalData(g_hwnd[id], (DWORD)0);
	}

#if ASYNC_PANNING 
    /*FIXME: stop panning pages*/
    destroy_panning_pages_context(&g_panning_ctxt);
#endif

	if (g_current_hwnd == g_hwnd[id]) {
		int next = look_for_next_tab(id);
		//next must in [0,TABCOUNT),or it should not reach here
		g_current_hwnd = g_hwnd[next];
	}
	ShowWindow(g_current_hwnd, SW_SHOW);
	update_tool_bar();
	DestroyWindow(g_hwnd[id]);
	g_hwnd[id] = HWND_NULL;
}
// set current tab window's IME status
BOOL set_tab_window_ime_stauts(HWND hWnd, BOOL show)
{
    int i;
    TAB_INFO *tab_info;
    if(hWnd == HWND_INVALID || hWnd == HWND_NULL){
        fprintf(stderr, "[Warning] Set tab window's IME status: hwnd is invalida!\n");
        return FALSE;
    }
    for( i=0; i<TABCOUNT; i++){
        if( hWnd == g_hwnd[i] ){
            tab_info = (TAB_INFO*)GetWindowAdditionalData(hWnd);
            if(!tab_info){
                fprintf(stderr, "[Warning] Set tab window's IME status: tab info is NULL!\n");
                return FALSE;
            }
            tab_info->ime_status = show;
            return TRUE;
        }
    }
    return FALSE;
}

// get current tab window's IME status
BOOL get_tab_window_ime_status(HWND hWnd)
{
    TAB_INFO *tab_info;
    if(hWnd == HWND_INVALID || hWnd == HWND_NULL){
        return FALSE;
    }
    tab_info = (TAB_INFO*)GetWindowAdditionalData(hWnd);
    if( !tab_info ){
        return FALSE;
    }
    return tab_info->ime_status;
}
void update_mdolphin_window(int id, int close, BOOL is_draged_mode)
{
    BOOL is_enable_ime;
	if (close)
		my_close_tab_window(g_hwnd[id]);
	else if (g_current_hwnd != g_hwnd[id]) {
		if (g_hwnd[id] == HWND_NULL){
			create_mdolphin_window(GetParent(g_current_hwnd), &g_hwnd[id], home_url);
        }else{
			update_window_display(g_hwnd[id]);
        }
	}
    // hide ime window
    show_ime_window(FALSE);
    if( is_draged_mode ){
        is_enable_ime = FALSE;
    }else{
        is_enable_ime = get_tab_window_ime_status(g_current_hwnd);
    }
    change_ime_menu_status(is_enable_ime);
}


HWND my_create_new_window(const char* url, DWORD styles, int x, int y, int width, int height)
{
#if 0
#ifndef MINIGUI_V3
	/* If magnifier is visible then close it */
    if (is_magnifier_visible())
        hide_magnifier(g_tb_hwnd, g_current_hwnd);
#endif
#endif

    int id = look_for_current_id(g_current_hwnd);
    int theOtherTabId = (int)(!id);
    create_mdolphin_window(GetParent(g_current_hwnd), &g_hwnd[theOtherTabId], url);
    return g_hwnd[theOtherTabId];
}

#ifdef __cplusplus
}
#endif
