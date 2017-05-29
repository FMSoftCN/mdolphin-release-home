#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "tabpage.h"
#include "panpages.h"
#include "mdhome.h"

#ifdef __cplusplus
extern "C" {
#endif

PANNING_CTXT* create_panning_pages_context(HWND hsrc, HWND hdst, HWND hMain)
{
    PANNING_CTXT* ctxt;
    TAB_INFO* ti_dst, *ti_src;

    ti_src = (TAB_INFO*)GetWindowAdditionalData(hsrc);
    ti_dst = (TAB_INFO*)GetWindowAdditionalData(hdst);
    if (!(ti_src && ti_dst))
        return NULL;

    if (!ti_src->hdc && ti_dst->hdc)
        return NULL;

    if (!(ctxt = (PANNING_CTXT*)calloc(1, sizeof(PANNING_CTXT))))
        return NULL;

    ctxt->hmain = hMain;
    ctxt->dst = ti_src;
    ctxt->src = ti_dst;
    return ctxt;
}

int begin_panning_pages(PANNING_CTXT* ctxt, PANNING_DIR dir, int steps)
{
    if (!ctxt)
        return -1;

    ctxt->dir = dir;
    ctxt->steps = steps;
    ctxt->time_id = 0x1000;
    switch (dir) {
    case MOVETO_LEFT:
    case MOVETO_RIGHT:
        ctxt->stepx = (int)((float)WINDOW_WIDTH/(float)steps);
        ctxt->stepy = 0;
        break;
    case MOVETO_UP:
    case MOVETO_DOWN:
        ctxt->stepy = (int)((float)WINDOW_HEIGHT/(float)steps);
        ctxt->stepy = 0;
        break;
    default:
        return -1;
    }

#if ASYNC_PANNING
    SetTimer(ctxt->hmain, ctxt->time_id, 2);
#else
    while (is_panning_pages(ctxt)) {
        panning_pages(ctxt);
        usleep(20000);
    }
    destroy_panning_pages_context(&ctxt);
#endif
    return 0;
}

void end_panning_pages(PANNING_CTXT* ctxt)
{
    if (!ctxt)
        return;
#if ASYNC_PANNING
    KillTimer(ctxt->hmain, ctxt->time_id);
#endif
    ctxt->index = ctxt->steps;
}

BOOL is_panning_pages(PANNING_CTXT* ctxt)
{
   if (ctxt && ctxt->index < ctxt->steps)
       return TRUE;
   return FALSE;
}

void panning_pages(PANNING_CTXT* ctxt)
{
    int x, y, w, h;

    if (!is_panning_pages(ctxt))
        return;
    ctxt->index++;

    x = ctxt->stepx*ctxt->index;
    y = ctxt->stepy*ctxt->index;
    w = GetGDCapability(ctxt->src->hdc, GDCAP_HPIXEL);
    h = GetGDCapability(ctxt->src->hdc, GDCAP_VPIXEL);

    /* FIXME: If the browser has the border, the argument dx, dy of the BitBlt
     * should substract the width of the border.
     * */
    if (ctxt->dir == MOVETO_RIGHT) {
        BitBlt(ctxt->src->hdc, x, 0, w-x, h, HDC_SCREEN, 0, BROWSER_TOP, 0);
        BitBlt(ctxt->dst->hdc, 0, 0, x,   h, HDC_SCREEN, w-x, BROWSER_TOP, 0);
    } else {
        if (w > x)
            BitBlt(ctxt->src->hdc, 0,  0, w-x, h, HDC_SCREEN, x, BROWSER_TOP, 0);
        BitBlt(ctxt->dst->hdc, w-x, 0,  x, h, HDC_SCREEN, 0, BROWSER_TOP, 0);
    }
}

void destroy_panning_pages_context(PANNING_CTXT** ctxt)
{
    if (!ctxt || !(*ctxt))
        return;
    end_panning_pages(*ctxt);
    free(*ctxt);
    *ctxt = 0;
}

#ifdef __cplusplus
}
#endif
