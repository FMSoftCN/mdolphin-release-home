/*
** $Id$
**
** softkeywindow.c.
**
** Copyright (C) 2008 Feynman Software.
**
** All right reserved by Feynman Software.
**
** Create date: 2008/7/11
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING 
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include "../mgiconfig.h"
#include "ime.h"
#include "mgpti.h"
#include "mgi.h"
#include "softkeyboard/softkeyboard.h"
#define HEIGHT_TASKBAR  0

#if KBD_ANIMATE 
#include "common_animates/common_animates.h"
#include "animate.h"
#endif

static void (*op_cb)(BOOL) = NULL;
static key_board_t* keyboard [SFKB_NUM];
static BOOL active = TRUE;
static PLOGFONT logfont1=0;
static PLOGFONT logfont2=0;
#define BUFFERSIZEMG 1024
/*ime status table*/
static int status_table [4][4] = { 
	{-1,  3,  1, -1},
	{ 0,  3, -1,  2},
	{ 0,  3,  1, -1},
	{ 0, -1,  1, -1}
};

static int convertgb2312toutf8(unsigned char *buffer, const unsigned char* characters, size_t mbs_length)
{
	int conved_mbs_len, ucs_len;
	char buffer2[BUFFERSIZEMG];
	const unsigned char* source = characters;
	if(!logfont1||!logfont2){
        fprintf(stderr, "[Error]mgi: can't convert gb2312 to utf8!\n");
		return 0;
    }
	ucs_len = MBS2WCSEx (logfont1, (void *)buffer2, FALSE,  source, mbs_length, BUFFERSIZEMG,  &conved_mbs_len); 
	ucs_len = WCS2MBSEx (logfont2, (unsigned char *)buffer,   (unsigned char *)buffer2, ucs_len, FALSE, BUFFERSIZEMG,  &conved_mbs_len); 
    return ucs_len; 
}
static void send_word(HWND target_hwnd, char *sendword, int type)
{
	char *word=sendword;
    int i = 0;
    int len = word?strlen(word):0;

	if(len<=0)
		return;
    
    switch (type) {
    case AC_SEND_EN_STRING:
        for (i=0; i<len; i++) {
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
            Send2ActiveWindow(mgTopmostLayer, MSG_CHAR, word[i], 0);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
            while(ERR_OK!=PostMessage(target_hwnd, MSG_CHAR, word[i], 0))
            {
            }

#endif
        }
        break;
    case AC_SEND_CN_STRING:
		{	
			unsigned char buffer[BUFFERSIZEMG];
			len =  convertgb2312toutf8(buffer, (unsigned char *)word, len);
			word=buffer;
			char *str = word;
			i =0;

			while(i<len)
			{
				WPARAM ch;
				//Process UTF8 flow 
				if(((Uint8)str[i]) <= 0x7F) //ascii code
				{
					ch = (WPARAM)str[i];
					i++;
				}
				else if(((Uint8)str[i]) <= 0xBF) //
				{
					i ++;
					continue;
				}
				else if(((Uint8)str[i]) <= 0xDF) //2 code
				{
					ch = ((Uint8) str[i])|(((Uint8)str[i+1])<<8);
					i += 2;
				}
				else if(((Uint8)str[i]) <= 0xEF) //three code
				{
					ch = ((Uint8)str[i])
						| (((Uint8)str[i+1])<<8)
						| (((Uint8)str[i+2])<<16);
					i += 3;
				}
				else
				{
					i ++;
					continue;
				}
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
				Send2ActiveWindow(mgTopmostLayer, MSG_CHAR, ch, 0);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
				PostMessage(target_hwnd, MSG_CHAR, ch, 0);
#endif
			}
		}


 
#if 0
        for (i=0; i<len; i += 2) {
            WORD wDByte;
            wDByte = MAKEWORD(word[i], word[i + 1]);
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
            Send2ActiveWindow(mgTopmostLayer, MSG_CHAR, wDByte, 0);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
            PostMessage(target_hwnd, MSG_CHAR, wDByte, 0);
#endif
        }
#endif 
        break;
    }

    return; 
}

static key_board_t* init_keyboard_data(HWND hWnd)
{
    /* for english key board */
    if (!(keyboard[0] = (key_board_t*)calloc(1, sizeof (key_board_t)))) {
        _MY_PRINTF("calloc keyboard data for EN failed\n");
        return NULL;
    }
    if (-1 == init_en_keyboard(hWnd, keyboard[0])) {
        _MY_PRINTF("error for initalize En-keyboard.\n");
        return NULL;
    }

    /* for num key board */
    if (!(keyboard[1] = (key_board_t*)calloc(1, sizeof (key_board_t)))) {
        _MY_PRINTF("calloc keyboard data for NUM failed\n");
        return NULL;
    }
    if (-1 == init_num_keyboard(hWnd, keyboard[1])) {
        _MY_PRINTF("error for initalize num-keyboard.\n");
        return NULL;
    }

    /* for punct key board */
    if (!(keyboard[2] = (key_board_t*)calloc(1, sizeof (key_board_t)))) {
        _MY_PRINTF("calloc keyboard data for punctuation failed\n");
        return NULL;
    }
    if (-1 == init_punct_keyboard(hWnd, keyboard[2])) {
        _MY_PRINTF("error for initalize Puctuation-keyboard.\n");
        return NULL;
    }

    /* for py key board */
    if (!(keyboard[3] = (key_board_t*)calloc(1, sizeof (key_board_t)))) {
        _MY_PRINTF("calloc keyboard data for EN failed\n");
        return NULL;
    }
    if (-1 == init_py_keyboard(hWnd, keyboard[3])) {
        _MY_PRINTF("error for initalize PinYin-keyboard.\n");
        return NULL;
    }

    //return the default keyboard at beginning.
    return keyboard[0];
}

static void destroy_key_win(void)
{
	//FIXME:
    destroy_en_keyboard(keyboard[0]);
    free(keyboard[0]);
    destroy_num_keyboard(keyboard[1]);
    free(keyboard[1]);
    destroy_punct_keyboard(keyboard[2]);
    free(keyboard[2]);
	destroy_py_keyboard(keyboard[3]);
    free(keyboard[3]);
}

#if KBD_ANIMATE
static void on_imewnd_jmp_finished(ANIMATE_SENCE* as)
{
    SOFTKBD_DATA* pdata;

    pdata = (SOFTKBD_DATA*)as->param;
    if (pdata->is_opened) {
		pdata->is_opened = 0;
        ShowWindow((HWND)as->normal->img, SW_HIDE);
	} else {
		pdata->is_opened = 1;
	}
}
#endif

static void show_ime_window(HWND hWnd, SOFTKBD_DATA* pdata, BOOL show, WPARAM wParam)
{
    int x, y;

	if(!pdata)
		return ;

    if (pdata->is_opened == show)
        return;
	
	softkey_reset();
	if(!show) //hide
	{
		if(pdata && pdata-> keyboard && pdata->keyboard->clear)
			pdata->keyboard->clear(pdata->keyboard);
	}
#if KBD_ANIMATE 
    x = (RECTW(g_rcDesktop) - SKB_WIN_W)/2;
    y = RECTH(g_rcDesktop)-HEIGHT_TASKBAR;
    if (show) {
        SetInterval(50);
        MoveWindow(hWnd, x, y-1, SKB_WIN_W, SKB_WIN_H, FALSE);
        ShowWindow(hWnd, SW_SHOW);
        RunJumpWindow(hWnd, x, y-1, x, y - SKB_WIN_H,
                SKB_WIN_W, SKB_WIN_H, on_imewnd_jmp_finished, pdata);
    } else {
        if (wParam == 0) {
            // use animates
            RunJumpWindow(hWnd, x, y - SKB_WIN_H, x, y,
                    SKB_WIN_W, SKB_WIN_H, on_imewnd_jmp_finished, pdata);
        } else {
            pdata->is_opened = 0;
            ShowWindow(hWnd, SW_HIDE);
        }
    }
#else
    ShowWindow(hWnd, show?SW_SHOW: SW_HIDE);
    pdata->is_opened = show;
    if (op_cb)
        op_cb(pdata->is_opened);
#endif
}

static int SoftKeyWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    SOFTKBD_DATA* pdata = NULL;

	if (!active)
		//return DefaultMainWinProc (hWnd, message, wParam, lParam);
		return 0;

    if (message != MSG_CREATE)
	{
        pdata = (SOFTKBD_DATA*)GetWindowAdditionalData(hWnd);
	}

    switch (message) {
    case MSG_NCCREATE:
        RegisterIMEWindow(hWnd);
        break;
    case MSG_CREATE: {
		if(!logfont1)
			logfont1 = CreateLogFont (NULL, "arial", "gb2312",
							 FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
							 FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
							 12, 0); 
		if(!logfont2)
			logfont2 = CreateLogFont (NULL, "arial", "utf8",
				FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
				FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
				12, 0); 

        if (!(pdata = (SOFTKBD_DATA*)calloc(1, sizeof (SOFTKBD_DATA)))) {
            _MY_PRINTF("Fail to calloc SOFTKBD data.\n");
            return -1;
        }

        pdata->current_board_idx = 0;
        if (NULL == (pdata->keyboard = init_keyboard_data(hWnd))) {
            _MY_PRINTF("Soft Key Window init failed\n");
            return -1;
        }
#ifdef KBD_TOOLTIP
        pdata->tooltip_win = CreateToolTip(hWnd);
#endif
        SetWindowAdditionalData(hWnd, (DWORD)pdata);
        break;
    }
    case MSG_IME_OPEN:
#ifndef MGDESKTOP_VERSION
        show_ime_window(hWnd, pdata, TRUE, wParam);
#endif
        // FIXME: Restore the case mode by sending MSG_LBUTTONDOWN about the LEFTSHIFT key.
      /*  if (get_pti_case_mode() == PTI_CASE_ABC) {
            md_key_t* mk = get_en_key_by_code(SCANCODE_LEFTSHIFT);
            int x = mk->bound.left + RECTW(mk->bound)/2;
            int y = mk->bound.top + RECTH(mk->bound)/2;
            SendMessage(hWnd, MSG_LBUTTONDOWN, 0, MAKELONG(x, y));
        }*/
        break;
    case MSG_IME_CLOSE:
#ifndef MGDESKTOP_VERSION
        show_ime_window(hWnd, pdata, FALSE, wParam);
#endif
		break;
    case MSG_IME_SETTARGET:
        if ((HWND)wParam != hWnd)
            pdata->target_hwnd = (HWND)wParam;
        return 0;
    case MSG_IME_GETTARGET:
        return (int)pdata->target_hwnd;
    case MSG_IME_GETSTATUS:
        return (int)pdata->is_opened;
    case MSG_KEYDOWN: 
    case MSG_KEYUP: 
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
        Send2ActiveWindow (mgTopmostLayer, message, wParam, lParam);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
        PostMessage(pdata->target_hwnd, message, wParam, lParam);
#endif
	    return 0;
    case MSG_LBUTTONDOWN:
    case MSG_LBUTTONUP:
    case MSG_MOUSEMOVE:
    case MSG_NCMOUSEMOVE:
#ifdef KBD_TOOLTIP

		if(MSG_LBUTTONUP == message)
			HideToolTip((HWND)((SOFTKBD_DATA *)GetWindowAdditionalData(hWnd))->tooltip_win);

#endif
	
		if(MSG_LBUTTONDOWN == message || MSG_LBUTTONUP == message)
			reset_mouse_state(hWnd);

       switch (pdata->keyboard->proceed_msg(pdata->keyboard, hWnd, message, wParam, lParam)) {
        case AC_CHANGE_KBD: {
            int y;
            POINT p; 

            p.x = LOSWORD(lParam);
            p.y = HISWORD(lParam);

            md_key_t* key = pdata->keyboard->key_window->get_key(pdata->keyboard->key_window, p);
            // FIXME: If get_key return NULL, I think dispatch this message again.
            if (!key)
                break;

            /* english */
            if (key->scan_code == SCANCODE_TOEN)     y = 0;   
            /* pinyin */ 
            if (key->scan_code == SCANCODE_TOPY)     y = 1;   
            /* 123 */ 
            if (key->scan_code == SCANCODE_TONUM)    y = 2;   
            /* oprator */ 
            if (key->scan_code == SCANCODE_TOOP)     y = 3;  

			/*clear old state */
			pdata->keyboard->clear(pdata->keyboard);
            pdata->current_board_idx = status_table[pdata->current_board_idx][y];
            pdata->keyboard = keyboard[pdata->current_board_idx];

            SendMessage(hWnd, MSG_ERASEBKGND, 0, 0L);
            softkey_reset();
            return 0;
        }
        case AC_SEND_EN_STRING:
            send_word(pdata->target_hwnd, pdata->keyboard->action.str, AC_SEND_EN_STRING);
            break;
        case AC_SEND_CN_STRING:
            send_word(pdata->target_hwnd, pdata->keyboard->action.str, AC_SEND_CN_STRING);
            break;
        case AC_SEND_MSG:
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
            Send2ActiveWindow(mgTopmostLayer, 
                pdata->keyboard->action.message,
                pdata->keyboard->action.wParam, 
                pdata->keyboard->action.lParam);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
            PostMessage(pdata->target_hwnd, 
                pdata->keyboard->action.message,
                pdata->keyboard->action.wParam, 
                pdata->keyboard->action.lParam);
#endif
            return 0;
        }
        break;
        case MSG_PAINT: {
            HDC hdc = BeginPaint(hWnd);
            do {
                view_window_t*   view_window   = pdata->keyboard->view_window;
                stroke_window_t* stroke_window = pdata->keyboard->stroke_window; 
                if (view_window == NULL || stroke_window == NULL)
                    break;
                view_window->style |= VW_DRAW_ELMTS;
                view_window->style &= ~VW_EL_PRESSED;
                view_window->update(view_window, hWnd, NULL);
                stroke_window->update(stroke_window, hWnd);
            } while(FALSE);

            EndPaint(hWnd, hdc);
            return 0;
        }
        case MSG_ERASEBKGND: 
            if (pdata && pdata->keyboard)
                pdata->keyboard->update(pdata->keyboard, hWnd, wParam, (RECT*)lParam);
            return 0;
        case MSG_CLOSE:
            UnregisterIMEWindow(hWnd);
            //SendMessage(HWND_DESKTOP, MSG_IME_UNREGISTER, (WPARAM) hWnd, 0);
            destroy_key_win();
#ifdef KBD_TOOLTIP
            DestroyMainWindow(pdata->tooltip_win);
#endif
            free(pdata);
            DestroyMainWindow(hWnd);
            PostQuitMessage(hWnd);
            return 0;
		case MSG_DESTROY:
            if(logfont1){
                DestroyLogFont(logfont1);
                logfont1=0;
            }
            if(logfont2){
                DestroyLogFont(logfont2);
                logfont2=0;
            }
            break;
    }
    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

static void init_createinfo(PMAINWINCREATE pCreateInfo)
{
    pCreateInfo->dwStyle = WS_ABSSCRPOS;
    pCreateInfo->dwExStyle = WS_EX_TOPMOST;
    pCreateInfo->spCaption = "Soft Key Window" ;
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor (0);
    pCreateInfo->hIcon = 0;
    pCreateInfo->MainWindowProc = SoftKeyWinProc;
    pCreateInfo->lx = (RECTW(g_rcDesktop) - SKB_WIN_W)/2;
    pCreateInfo->ty = RECTH(g_rcDesktop) - SKB_WIN_H - HEIGHT_TASKBAR;
    pCreateInfo->rx = (RECTW(g_rcDesktop) + SKB_WIN_W)/2;
    pCreateInfo->by = RECTH(g_rcDesktop);
    pCreateInfo->iBkColor = COLOR_lightwhite; 
    pCreateInfo->dwAddData = 0;
}

static HWND create_ime_win(HWND hosting)
{
    MAINWINCREATE CreateInfo;
    HWND hMainWnd;

    init_createinfo(&CreateInfo);
    CreateInfo.hHosting = hosting;

    hMainWnd = CreateMainWindow(&CreateInfo);
    if (hMainWnd == HWND_INVALID)
        return HWND_INVALID;

    return hMainWnd;
}

void mgiEnableSoftKeypad(BOOL e)
{
	active = e;
}

#ifdef _MGRM_PROCESSES
HWND mgiCreateSoftKeypad(void (*cb)(BOOL IsShown))
{
	op_cb = cb; 
	return create_ime_win(HWND_DESKTOP); 
}

#else

typedef struct ime_info {
    sem_t wait;
    HWND hwnd;
} IME_INFO;

static void* start_ime(void* data)
{
    MSG Msg;
    IME_INFO* ime_info = (IME_INFO*) data;
    HWND ime_hwnd;

    ime_hwnd = ime_info->hwnd = create_ime_win(HWND_DESKTOP);
    if (ime_hwnd == HWND_INVALID)
        return NULL;

    sem_post(&ime_info->wait);

    while (GetMessage(&Msg, ime_hwnd)) {
        TranslateMessage (&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (ime_hwnd);

    return NULL;
}

static pthread_t imethread;

/* the argument of 'hosting' is ignored. */
HWND mgiCreateSoftKeypad(void (*cb)(BOOL IsShown))
{
	op_cb = cb; 
    IME_INFO ime_info;
    pthread_attr_t new_attr;

    sem_init(&ime_info.wait, 0, 0);

    pthread_attr_init(&new_attr);
    pthread_attr_setdetachstate(&new_attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&imethread, &new_attr, start_ime, &ime_info);
    pthread_attr_destroy(&new_attr);

    sem_wait(&ime_info.wait);
    sem_destroy(&ime_info.wait);

    return ime_info.hwnd;
}

#endif
