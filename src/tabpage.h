#ifndef _TABPAGE_H
#define _TABPAGE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _TAB_INFO {
    char*       status;
    char*       location;
    char*       title;
    unsigned int progress;
    unsigned int back;
    unsigned int forward;
    HDC         hdc;
    unsigned int title_len; 
    unsigned int title_begin;
    BOOL        load;
    BOOL        ime_status;
} TAB_INFO;

HWND my_create_new_window(const char* url, DWORD styles, int x, int y, int width, int height);
void my_close_tab_window(HWND hWnd);
void create_mdolphin_window(HWND parent, HWND *mdolphin_hwnd, const char* url);

int look_for_used_tab_num( void);
int look_for_current_id(HWND hWnd);
#ifdef WITH_TWO_TAB
HWND look_for_inactive_tab_wnd(void);
#endif

void update_mdolphin_window(int click, BOOL close, BOOL is_draged_mode);
HWND lookup_window_by_tabinfo(const TAB_INFO* info);

// get current tab window's IME status
BOOL get_tab_window_ime_status(HWND hWnd);
// set current tab window's IME status
BOOL set_tab_window_ime_stauts(HWND hWnd, BOOL show);


#ifdef __cplusplus
}
#endif

#endif
