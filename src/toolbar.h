#ifndef _TOOLBAR_H
#define _TOOLBAR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _TOOLBAR_INFO {
    int id;/* ID of toolbar */
    RECT rect;
    BOOL enabled;/* true for enabled, false for disabled*/
	BOOL highlight; /*whether highlight*/
} TOOLBAR_INFO;

/* ID of toolbar */
enum {
	OP_BEGIN = 10 ,
	OP_NAV_BACKWARD,
	OP_NAV_FORWARD,
	OP_NAV_HOME,
	OP_NAV_STOP,
	OP_NAV_ZOOMIN,
	OP_NAV_ZOOMOUT,
	OP_NAV_DRAG,
	OP_NAV_IME,
	OP_TAB_PAGE,
	OP_END,
};
extern int tab_begin_x;
extern int tab_begin_y;
extern int tab_normal_w;
extern int tab_normal_h;
extern int tab_current_w;

extern int tab_close_w;
extern int tab_close_h;


#define MENU_ITEM_NUM		8
#define MENU_ITEM_STATUS	3

#define TAB_BEGIN_X		tab_begin_x
#define TAB_BEGIN_Y		tab_begin_y
#define TAB_NORMAL_W	tab_normal_w
#define TAB_NORMAL_H	tab_normal_h
#define TAB_CURRENT_W	tab_current_w

#define TAB_CLOSE_W		tab_close_w
#define TAB_CLOSE_H		tab_close_h

void change_loading_menu_status(BOOL load, unsigned int progress);
void change_history_menu_status(BOOL back, BOOL forward);
BOOL change_ime_menu_status(BOOL show);

void update_tool_bar(void);
void update_tab_bar_title(void);

void show_ime_window(BOOL show);

HWND create_toobar_window(HWND host);
#ifdef __cplusplus
}
#endif

#endif
