#ifndef _NEW_DEMO_H
#define _NEW_DEMO_H

#include <minigui/mgconfig.h>

#if defined(MINIGUI_MAJOR_VERSION)&&(MINIGUI_MAJOR_VERSION == 3)
#define MINIGUI_V3 1
#endif

#include <mdolphin/mdconfig.h>

#if defined(MDOLPHIN_MAJOR_VERSION)&&(MDOLPHIN_MAJOR_VERSION == 3)
#define MDOLPHIN_V3 1
#endif
#ifdef __cplusplus
extern "C" {
#endif

/* control height and width of browser window */
#define WINDOW_WIDTH		g_window_width
#define WINDOW_HEIGHT		g_window_height

#define BROWSER_LEFT	    g_browser_left	
#define BROWSER_TOP			g_browser_top
#define BROWSER_WIDTH		g_browser_width
#define BROWSER_HEIGHT		g_browser_height

#define TOOLBAR_LEFT		g_toolbar_left
#define TOOLBAR_TOP			g_toolbar_top
#define TOOLBAR_WIDTH		g_toolbar_width
#define TOOLBAR_HEIGHT		g_toolbar_height

#define VSCROLLBAR_LENGTH   g_vscrollbar_length
#define HSCROLLBAR_LENGTH   g_hscrollbar_length

#define WITH_TWO_TAB    1
#ifdef WITH_TWO_TAB
#define TABCOUNT 2 
#else
#define TABCOUNT 4 
#endif


extern int g_window_width;
extern int g_window_height;

extern int g_browser_left;
extern int g_browser_top;
extern int g_browser_width;
extern int g_browser_height;

extern int g_toolbar_left;
extern int g_toolbar_top;
extern int g_toolbar_width;
extern int g_toolbar_height;

extern int g_vscrollbar_length;
extern int g_hscrollbar_length;

extern BOOL g_is_big_screen;
//extern char g_res_path[40];
//extern char *g_pos_res_path;

enum {
    /* id of control */
    IDC_MDOLPHIN = 101, 
    IDC_TOOLBAR,
};

#ifdef __cplusplus
}
#endif

#endif
