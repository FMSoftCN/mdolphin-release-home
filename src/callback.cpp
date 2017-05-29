#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <mgutils/mgutils.h>
#include <mdolphin/mdolphin.h>
#include "mdhome.h"
#include "toolbar.h"
#include "tabpage.h"
#include "savefile.h"
#include "tooltipwin.h"
#include "mdolphin_auth.h"

#include "clientcert.h"
#if ENABLE_SSLFILE
#include <openssl/x509v3.h>
#include <openssl/x509_vfy.h>
#include <openssl/crypto.h>
#include <openssl/lhash.h>
#include <openssl/objects.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/pkcs12.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

extern HWND g_current_hwnd;
extern int g_scale_width;

extern void my_owner_draw(HWND hWnd, HDC hdc, const RECT* prc);
//extern BOOL my_provide_auth_callback (const char* title, char *username, char *password);
extern BOOL my_confirm_callback (HWND parent, const char * text, const char * caption);
extern void my_error_code_callback(HWND hWnd, int errCode, const char* url);
extern void my_message_callback (HWND parent, const char * text, const char * caption);
//extern BOOL my_confirm_callback (HWND parent, const char * text, const char * caption);
extern char * my_prompt_callback (HWND parent, const char* text, const char* defaultval, const char* caption);

static void set_location_text (HWND hWnd, const char * text)
{
    if (!text)
        return;
	
	TAB_INFO *info = (TAB_INFO *)GetWindowAdditionalData(hWnd);
	if (info) {
		if (info->location)
			free(info->location);
		info->location = strdup(text);
	}

    // hide ime window
    show_ime_window(FALSE);
    // change ime status on toolbar
    change_ime_menu_status(FALSE);

}

static void set_title_text(HWND hWnd, const char * text)
{
    if (!text)
        return;

	TAB_INFO *info = (TAB_INFO *)GetWindowAdditionalData(hWnd);
	if (info) {
		if (info->title)
			free(info->title);
		info->title = strdup(text);

		if(hWnd == g_current_hwnd) 
			update_tab_bar_title();
	}
}

static void set_status_text (HWND hWnd, const char * text)
{
    if (!text)
        return;

	TAB_INFO *info = (TAB_INFO *)GetWindowAdditionalData(hWnd);
	if (info) {
		if (info->status)
			free(info->status);
		info->status = strdup(text);
	}
}

static void set_loading_status (HWND hWnd, BOOL load, unsigned int progress)
{
	TAB_INFO *info = (TAB_INFO *)GetWindowAdditionalData(hWnd);
	if (info) {
		info->load = load;
		info->progress = progress;
	}

    if (hWnd != g_current_hwnd)
        return;
    change_loading_menu_status(load, progress);
}

static void set_histroy_status (HWND hWnd, unsigned int bcount, unsigned int fcount, unsigned int capacity)
{
	
	TAB_INFO *info = (TAB_INFO *)GetWindowAdditionalData(hWnd);
	if (info) {
		info->back = bcount;
		info->forward = fcount;
	}

    if (hWnd != g_current_hwnd)
        return;
    change_history_menu_status(bcount>0?TRUE:FALSE, fcount>0?TRUE:FALSE);
}

#if ENABLE_SSLFILE
static BOOL my_provide_client_cert(CERT_DATA **x509, SSL_PKEY **pkey, const CERT_NAME **names, int count)
{
    int i;
    if (count) {
        for (i = 0; i < count; i++) {
            if (readCertificateHome(X509_NAME_hash((X509_NAME *)names[i]), (X509**)x509, (EVP_PKEY**)pkey))
                return TRUE;
        }
        MessageBox(GetParent(g_current_hwnd), 
                "No client certificate! Please select your certificate!", "Alert", MB_OK);

        if (importCertificate(GetParent(g_current_hwnd))) {
            if (count) {
                for (i = 0; i < count; i++) {
                    if (readCertificateHome(X509_NAME_hash((X509_NAME *)names[i]), (X509**)x509, (EVP_PKEY**)pkey))
                        return TRUE;
                }
            }
        }
    }
    return FALSE;
}
#endif
#if ENABLE_SSL
static CERT_RESULT my_verify_server_cert(CERT_RESULT result, CERT_DATA *x509)
{
    return verifyCertificate(GetParent(g_current_hwnd), result, (X509*)x509);
}
#endif

BOOL my_choosefile_callback (HWND hWnd, char* filename, size_t bufsize, BOOL IsSave)
{
    //printf("file: %s\n", filename);
	NEWFILEDLGDATA newfiledlg =
	{
#ifdef MINIGUI_V3
            FALSE, FALSE,
#else
	        FALSE,
#endif
			"",
			"",
			"./",
			"All file(*.*)",
			1
	};

	if(IsSave)
		newfiledlg.IsSave = 1;   // IsSave =0 show open file , =1 show save file
	else
		newfiledlg.IsSave = 0;   // IsSave =0 show open file , =1 show save file
	int retvalue = ShowOpenDialog (hWnd, 100, 100, 350, 250, &newfiledlg);
	if (retvalue == IDOK)
		strcpy (filename, newfiledlg.filefullname);
	else
		return false;

	return true;

}

static void my_capture_image(HWND hWnd, HDC hdc, const RECT* prc)
{
	TAB_INFO *info = (TAB_INFO *)GetWindowAdditionalData(hWnd);
    if (info && prc)
        info->hdc = hdc;

	RENDERING_MODE mode;
	mdolphin_get_rendering_mode(g_current_hwnd, &mode);
	if (mode == MD_VIRTUALVIEW_MODE) {
		int width, height;
		mdolphin_get_contents_size(g_current_hwnd, &width, &height);
		if (width && width>BROWSER_WIDTH && g_scale_width != width) {
			g_scale_width = width;
			mdolphin_set_rendering_mode(g_current_hwnd, MD_VIRTUALVIEW_MODE, BROWSER_WIDTH*800/g_scale_width);
		}
	}
}
#if MDOLPHIN_V3
static BOOL my_set_ime_status(HWND hWnd, BOOL show){
    // set current tab window's IME status
	            
    set_tab_window_ime_stauts(hWnd, show);
    // update IME status on toolbar
    change_ime_menu_status(show);

    show_ime_window(FALSE);

    return FALSE;
}
#else
static BOOL my_set_ime_status(HWND hWnd, BOOL show, EditorElementInfo){
    // set current tab window's IME status
	            
    set_tab_window_ime_stauts(hWnd, show);
    // update IME status on toolbar
    change_ime_menu_status(show);

    show_ime_window(FALSE);

    return FALSE;
}
#endif
static const char *my_useragent(const char *url)
{
        if (strstr(url, ".google.com"))
                    return "Mozilla/5.0 (MiniGUI/2.04; U; Linux i686; en) AppleWebKit/525.1+ (KHTML, like Gecko, Safari/525.1+) mDolphin";
            return "Mozilla/5.0 (MiniGUI/2.04; U; linux-i686; en; rv:1.8.1.12) mDolphin/2.00 Compatible Gecko/20070601";
}

void set_callback_info(HWND hWnd)
{
    CB_INFO cb_info;
    memset (&cb_info, 0, sizeof(CB_INFO));
    
    cb_info.CB_MESSAGE_BOX = my_message_callback;
    cb_info.CB_CONFIRM_BOX = my_confirm_callback;
    cb_info.CB_PROMPT_BOX = my_prompt_callback ;
    cb_info.CB_SET_TITLE = set_title_text;
    cb_info.CB_SET_LOCATION = set_location_text;
    cb_info.CB_SET_STATUS = set_status_text;
    cb_info.CB_SET_LOADING_STATUS = set_loading_status;
    cb_info.CB_SET_HISTORY_STATUS = set_histroy_status;
 //   cb_info.CB_CHOOSEFILE_BOX = NULL;
 //   cb_info.CB_SAVE_FILE_DATA = NULL;
    
 //Caijun.Lee add 
    cb_info.CB_SAVE_FILE_DATA = my_savefile_callback;
  //  cb_info.CB_CHOOSEFILE_BOX = my_choosefile_callback; 

    cb_info.CB_SET_IME_STATUS = my_set_ime_status;
    cb_info.CB_ERROR = my_error_code_callback;
#if MDOLPHIN_V3
    cb_info.CB_PROVIDE_AUTH = my_provide_auth_callbackv3;
#else
    cb_info.CB_PROVIDE_AUTH = my_provide_auth_callback;
#endif
    cb_info.CB_CREATE_POPUP_MENU = NULL;
    cb_info.CB_OPEN_WINDOW = my_create_new_window;
    cb_info.CB_CLOSE_WINDOW = my_close_tab_window;
    cb_info.CB_SET_TOOLTIP = my_tooltip;
    cb_info.CB_CUSTOM_USERAGENT = my_useragent;
    cb_info.CB_CAPTURE_IMAGE = my_capture_image;
    cb_info.CB_OWNER_DRAW = my_owner_draw;

//Caijun.Lee add   
//    cb_info.CB_SAVE_AS_FILE_DATA = my_saveas_callback;


//Caijun.Lee add   
#if ENABLE_SSL
#if ENABLE_SSLFILE
    cb_info.CB_PROVIDE_CLIENT_CERT = my_provide_client_cert;
#else
    cb_info.CB_PROVIDE_CLIENT_CERT = NULL;
#endif
    cb_info.CB_VERIFY_SERVER_CERT = my_verify_server_cert;
#endif

    if (g_current_hwnd)
        mdolphin_set_callback_info(g_current_hwnd, &cb_info);
}

#ifdef __cplusplus
}
#endif
