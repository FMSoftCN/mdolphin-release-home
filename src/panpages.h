
#ifndef __PANPAGE_H__
#define __PANPAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ASYNC_PANNING 0

typedef enum _PANNING_DIR {
    MOVETO_LEFT = 0,
    MOVETO_RIGHT,
    MOVETO_UP,
    MOVETO_DOWN
}PANNING_DIR;

typedef struct _PANNING_CTXT{
    TAB_INFO* src;
    TAB_INFO* dst;
    HWND hmain;
    int time_id;
    int steps;
    int dir;

    int stepx;
    int stepy;
    int index;
} PANNING_CTXT;

PANNING_CTXT* create_panning_pages_context(HWND hsrc, HWND hdst, HWND hMain);
void destroy_panning_pages_context(PANNING_CTXT** ctxt);
int begin_panning_pages(PANNING_CTXT* ctxt, PANNING_DIR dir, int steps);
void end_panning_pages(PANNING_CTXT* ctxt);
BOOL is_panning_pages(PANNING_CTXT* ctxt);
void panning_pages(PANNING_CTXT* ctxt);

#ifdef __cplusplus
}
#endif

#endif
