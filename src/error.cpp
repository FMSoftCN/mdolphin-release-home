
#include <string.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <mdolphin/mdolphin.h>
#include <mdolphin/mdolphin_errcode.h>
#include "mdhome.h"
#include "prompt_dialog.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

static char buf[32];
static PLOGFONT errFont;
static char prompt_text [256];


enum {
    /* id of control */
    ERR_WARNING = 0, 
    ERR_CONFIRM,
    ERR_ERROR,
};



bool g_err_hwnd_show;

static const char *get_error_description(HWND hWnd, int errCode, const char* failedUrl)
{
    switch (errCode) {
        case MDEC_NET_URL_ERROR:
            return "The URL was not properly formatted.";
        case MDEC_NET_UNSUPPORTED_PROTOCOL:
            return "unsupported protocol.";
        case MDEC_NET_DNS_ERROR:
            return "couldn't resolve the host.";
        case MDEC_NET_COULDNT_CONNECT:
            return "Failed to connect() to host or proxy.";
        case MDEC_NET_UNKNOWN_ERROR:
            return "It's an unknown network error.";
        case MDEC_NET_FTP_421:
        case MDEC_NET_FTP_425:
        case MDEC_NET_FTP_426:
        case MDEC_NET_FTP_450:
        case MDEC_NET_FTP_451:
        case MDEC_NET_FTP_452:
        case MDEC_NET_FTP_500:
        case MDEC_NET_FTP_501:
        case MDEC_NET_FTP_502:
        case MDEC_NET_FTP_503:
        case MDEC_NET_FTP_504:
        case MDEC_NET_FTP_530:
        case MDEC_NET_FTP_532:
        case MDEC_NET_FTP_550:
        case MDEC_NET_FTP_551:
        case MDEC_NET_FTP_552:
        case MDEC_NET_FTP_553:
            {
                memset(buf, 0 ,32);
                snprintf(buf, 31, "FTP ERROR %d !", errCode - MDEC_NET_FTP_BASE);
                buf[31]='\0';
                return buf;
            }
        case MDEC_NET_FTP_UNKNOWN_ERROR:
            return "It's an unknown FTP error.";
        case MDEC_NET_FILE_READ_ERROR:
            return "It's a FILE error. A file given with FILE:// couldn't be opened.";
        case MDEC_NET_SSL_CONNECT_ERROR:
            return "It's an SSL error which occurred somewhere in the SSL/TLS handshake.";
        case MDEC_NET_SSL_PEER_CERTIFICATE:
            return "The remote server's SSL certificate was deemed not OK.";
        case MDEC_NET_SSL_ENGINE_NOTFOUND:
            return "The specified crypto engine wasn't found.";
        case MDEC_NET_SSL_ENGINE_SETFAILED:
            return "Failed setting the selected SSL crypto engine as default.";
        case MDEC_NET_SSL_CERTPROBLEM:
            return "Problem with the local client certificate.";
        case MDEC_NET_SSL_CIPHER:
            return "Couldn't use specified cipher.";
        case MDEC_NET_SSL_CACERT:
            return "Peer certificate cannot be authenticated with known CA certificates.";
        case MDEC_NET_SSL_FTP_ERROR:
            return "Requested FTP SSL level failed.";
        case MDEC_NET_SSL_ENGINE_INITFAILED:
            return "Initiating the SSL Engine failed.";
        case MDEC_NET_SSL_CACERT_BADFILE:
            return "Problem with reading the SSL CA cert (path? access rights?)";
        case MDEC_NET_PROXY_ERROR:
            return "It's a PROXY error.";
        default:
            return NULL;
    }
}
#define ERR_W	236
#define ERR_H	124

static DLGTEMPLATE DlgInitProgress =
{
    WS_THINFRAME | WS_VISIBLE, 
    WS_EX_TOPMOST,
    0, 0, ERR_W, ERR_H, 
    "mDophin Error",
    0, 0,
    0, NULL,
    0
};

static BITMAP errBk;
static BITMAP errMark;
static BITMAP errButton;

static void draw_err_background(HDC hdc)
{
	FillBoxWithBitmap(hdc, 0, 0, -1, -1, &errBk);
}

static RECT YRECT ={35,  98, 42+35,  18+98};
static RECT NRECT ={161, 98, 42+161, 18+98};
static RECT ORECT ={98,  98, 42+98,  18+98};
static RECT MRECT ={9,   26, 30+9,   37+26};
static RECT CRECT ={5,   5,  236-5,  20-5};
static RECT TRECT ={52,  33, 195,    87};

static void draw_err_text(HDC hdc, const char *caption, const char *msg)
{
    LOGFONT oldFont;
	int oldColor;
	GetLogFontInfo(hdc, &oldFont);
	SelectFont(hdc, errFont);
	SetBkMode(hdc, BM_TRANSPARENT); 

	oldColor = SetTextColor(hdc, COLOR_lightwhite);
	if(caption) 
		DrawText(hdc, caption, -1, &CRECT, DT_LEFT |  DT_VCENTER | DT_SINGLELINE);

	SetTextColor(hdc, oldColor);
	if (msg) 
		DrawText(hdc, msg, -1, &TRECT, DT_LEFT |  DT_VCENTER | DT_CHARBREAK);

	SetBkMode(hdc, BM_OPAQUE); 
	SelectFont(hdc, &oldFont);
}
	
static void draw_err_mark_button(HDC hdc, ERR_INFO *info)
{
	int hl1, hl2;
	if (info->hl1)
		hl1 = 18;
	else
		hl1 = 0;
	if (info->hl2)
		hl2 = 18;
	else
		hl2 = 0;

	if (info->err == ERR_CONFIRM) {
		FillBoxWithBitmapPart(hdc, YRECT.left, YRECT.top, RECTW(YRECT), RECTH(YRECT), 
				errButton.bmWidth, errButton.bmHeight, &errButton, 0, hl1);
		FillBoxWithBitmapPart(hdc, NRECT.left, NRECT.top, RECTW(NRECT), RECTH(NRECT), 
				errButton.bmWidth, errButton.bmHeight, &errButton, 42*2, hl2);
	} else{
		FillBoxWithBitmapPart(hdc, ORECT.left, ORECT.top, RECTW(ORECT), RECTH(ORECT), 
				errButton.bmWidth, errButton.bmHeight, &errButton, 42, hl1);
	}
	FillBoxWithBitmapPart(hdc, MRECT.left, MRECT.top, RECTW(MRECT), RECTH(MRECT), 
			errMark.bmWidth, errMark.bmHeight, &errMark, info->err*30, 0);
}

static void my_paint(HDC hdc, ERR_INFO *info)
{
	draw_err_background(hdc);
	draw_err_mark_button(hdc, info);
	draw_err_text(hdc, info->caption, info->msg);
}

static void deal_with_mouse_up(HWND hDlg, int x, int y, ERR_INFO *info)
{
	if (info->err == ERR_CONFIRM) {
		if (PtInRect(&YRECT, x, y)) 
		{
			EndDialog (hDlg, 1);
			g_err_hwnd_show = false;
		}

		if (PtInRect(&NRECT, x, y)) 
		{
			EndDialog (hDlg, 0);
			g_err_hwnd_show = false;
		}

	} else {
		if (PtInRect(&ORECT, x, y)) 
		{
			EndDialog (hDlg, 1);
		    g_err_hwnd_show = false;
		}

	}

	info->hl1 = FALSE;
	info->hl2 = FALSE;

	if (info->err == ERR_CONFIRM) {
		InvalidateRect(hDlg, &YRECT, FALSE);
		InvalidateRect(hDlg, &NRECT, FALSE);
	} else
		InvalidateRect(hDlg, &ORECT, FALSE);
}

static void deal_with_mouse_down(HWND hDlg, int x, int y, ERR_INFO *info)
{
	if (info->err == ERR_CONFIRM) {
		if (PtInRect(&YRECT, x, y)) {
			info->hl1 = TRUE;
			InvalidateRect(hDlg, &YRECT, FALSE);
		}
		if (PtInRect(&NRECT, x, y)) {
			info->hl2 = TRUE;
			InvalidateRect(hDlg, &NRECT, FALSE);
		}
	} else {
		if (PtInRect(&ORECT, x, y)) {
			info->hl1 = TRUE;
			InvalidateRect(hDlg, &ORECT, FALSE);
		}
	}

    //g_err_hwnd_show = false;
}

static int InitDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	ERR_INFO *info;
	RECT rc;
    switch (message) {
    case MSG_INITDIALOG:


		if (LoadBitmap(HDC_SCREEN, &errBk, "res/err_bk.png")) 
			return -1;
		if (LoadBitmap(HDC_SCREEN, &errMark, "res/err_mark.png")) 
			return -1;
		if (LoadBitmap(HDC_SCREEN, &errButton, "res/err_button.png")) 
			return -1;

		GetWindowRect(((ERR_INFO *)lParam)->hParent, &rc);
		MoveWindow(hDlg, (RECTW(rc)-ERR_W)/2, (RECTH(rc)-ERR_H)/2, ERR_W+2, ERR_H+2, TRUE);

		errFont = CreateLogFont(NULL, "fmsong", "UTF-8",
				FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL, FONT_OTHER_NIL,
				FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 12, 0);

		SetWindowAdditionalData(hDlg, (DWORD)lParam);

        g_err_hwnd_show = true;

		return 1;
    case MSG_PAINT:
		{
			HDC hdc = BeginPaint(hDlg);
			info = (ERR_INFO *)GetWindowAdditionalData(hDlg);
			my_paint(hdc, info);
			EndPaint(hDlg, hdc);
			return 0;
		}
	case MSG_LBUTTONUP: 
		{
			int x = LOSWORD (lParam);
			int y = HISWORD (lParam);
			info = (ERR_INFO *)GetWindowAdditionalData(hDlg);
			deal_with_mouse_up(hDlg, x, y, info);
		}
		break;
	case MSG_LBUTTONDOWN:
		{
			int x = LOSWORD (lParam);
			int y = HISWORD (lParam);
			info = (ERR_INFO *)GetWindowAdditionalData(hDlg);
		    printf("enter MSG_LBUTTONDOWN ok\n");
			deal_with_mouse_down(hDlg, x, y, info);
		}
		break;
	case MSG_CLOSE:
		UnloadBitmap(&errBk);
		UnloadBitmap(&errMark);
		UnloadBitmap(&errButton);
		DestroyLogFont(errFont);
        g_err_hwnd_show = false;
		break;
	}
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int error_dialog_box (HWND hWnd, const char* caption, const char *msg, int err)
{
	ERR_INFO *err_info = (ERR_INFO *)malloc(sizeof(ERR_INFO));
	if (!err_info) {
		fprintf(stderr, "malloc is error!\n");
		return 0;
	}
	memset(err_info, 0, sizeof(ERR_INFO));
	if (caption)
		err_info->caption = strdup(caption);
	if (msg)
		err_info->msg = strdup(msg);
	err_info->err = err;
	err_info->hParent = hWnd;
 
	DlgInitProgress.controls = NULL;
	int ret = DialogBoxIndirectParam (&DlgInitProgress, hWnd, InitDialogBoxProc, (LPARAM)err_info);

	if (err_info->caption)
		free(err_info->caption);
	if (err_info->msg)
		free(err_info->msg);
	free(err_info);
	return ret;
}

void my_error_code_callback (HWND hWnd, int errCode, const char* failedUrl)
{
	const char *err = get_error_description(hWnd, errCode, failedUrl);
	if (!err || !failedUrl)
		return; 
	error_dialog_box (hWnd, failedUrl, err, ERR_ERROR);
}

void my_message_callback (HWND parent, const  char * text, const  char * caption)
{
	if(!text || !caption)
		return;
	error_dialog_box (parent, caption, text, ERR_WARNING);
}

BOOL my_confirm_callback (HWND parent, const char * text, const char * caption)
{
	if(!text || !caption)
		return FALSE;

	if (error_dialog_box (parent, caption, text, ERR_CONFIRM)) 
		return TRUE;
	return FALSE;
}

char * my_prompt_callback (HWND parent, const char* text, const char* defaultval, const char* caption)
{
    memset (prompt_text, 0 , sizeof(prompt_text));
    if (prompt_box(parent, text, defaultval, prompt_text, sizeof(prompt_text)))
        return prompt_text;
    return NULL;
}
#ifdef __cplusplus
}
#endif
