
#ifndef __MAGNIFIER_H__
#define __MAGNIFIER_H__

#ifdef __cplusplus
extern "C" {
#endif

int show_magnifier(HWND toolbar, HWND browser);
int hide_magnifier(HWND toolbar, HWND browser);
BOOL try_hide_magnifier(HWND toolbar, HWND browser, int x, int y);
BOOL is_magnifier_visible();
void get_screen_to_magnifier(HWND browser, int x, int y);

#ifdef __cplusplus
}
#endif

#endif
