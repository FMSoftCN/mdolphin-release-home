
#ifndef __MAGNIFIER3_H__
#define __MAGNIFIER3_H__

#ifdef __cplusplus
extern "C" {
#endif

int create_glass_window(HWND parent);

int show_magnifier(HWND toolbar, HWND browser);
BOOL is_magnifier_visible();
void get_screen_to_magnifier(HWND browser, int x, int y);
int hide_magnifier();
#ifdef __cplusplus
}
#endif

#endif
