#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "mdhome.h"
#include "clientcert.h"
#ifdef MINIGUI_V3
#include <mgutils/mgutils.h>
#else
#include <minigui/mgext.h>
#include <minigui/newfiledlg.h>
#endif

#include <mdolphin/mdolphin.h>

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

extern HWND g_current_hwnd;

#define IDC_INPUT_PASSWORD 610

static DLGTEMPLATE PopWinData =
{
     WS_CAPTION | WS_THINFRAME,
     WS_EX_NONE, 
     200, 200 , 250, 180,
     "Password Entry Dialog",
     0, 0,
     5,
     NULL, 0
};
static CTRLDATA CtrlInitProgress [] =
{
    {
        CTRL_STATIC,
        WS_VISIBLE,
        20, 20, 220, 40,
        0,
        "Please enter the password that was used to encrypt this certificate backup.\n",
        0, WS_EX_TRANSPARENT
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE ,
        20, 80, 60, 20,
        0,
        "Password:",
        0, WS_EX_TRANSPARENT
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_PASSWORD | WS_BORDER,
        80, 80, 160, 20,
        IDC_INPUT_PASSWORD,
        NULL,
        0
    },
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
        40, 120, 60, 20,
        IDOK, 
        "Ok",
        0
    },
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
        140, 120, 60, 20,
        IDCANCEL, 
        "Cancel",
        0
    }
};

static int inputBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    int len;
    switch (message) {
        case MSG_INITDIALOG: 
            SetWindowAdditionalData(hDlg, (DWORD)lParam);
            return 1;
        case MSG_COMMAND:
            switch (wParam) {
                case IDOK: 
                    len = GetWindowTextLength (GetDlgItem (hDlg, IDC_INPUT_PASSWORD));
                    if (len > 0) 
                    {
                        GetWindowText (GetDlgItem (hDlg, IDC_INPUT_PASSWORD), 
                                (char *)GetWindowAdditionalData(hDlg), len);
                        len = 0;
                    }
                case IDCANCEL:
                    EndDialog (hDlg, wParam);
                    break;
            }
            break;
        case MSG_CLOSE:
            EndDialog (hDlg, IDCANCEL);
            break;
    }

    return DefaultDialogProc (hDlg, message, wParam, lParam);
}
static void inputPassword( HWND hParent, char * pw)
{
    PopWinData.controls = CtrlInitProgress;
    DialogBoxIndirectParam (&PopWinData, hParent, inputBoxProc, (DWORD)pw);
}

/*
 * Save certificate with a hashed filename.
 * Return: 0 on success, 1 on failure.
 */

int saveCertificateHome(X509 * cert, bool isClient)
{
    char buf[4096];

    FILE * fp = NULL;
    unsigned int i = 0;
    int retval = 1;

    char *dir = get_current_dir_name();
    if (dir) {
        /*Attempt to create .dillo/certs blindly - check later*/
        snprintf(buf,4096,"%s/.mDolphin/", dir);
        if (opendir(buf)==NULL)
            mkdir(buf, 01777);
        snprintf(buf,4096,"%s/.mDolphin/certs/", dir);
        if (opendir(buf)==NULL)
            mkdir(buf, 01777);

        do{
            if (isClient){
                snprintf(buf,4096,"%s/.mDolphin/certs/%lx.%s",
                        dir, X509_issuer_name_hash(cert), "crt");
            } else
                snprintf(buf,4096,"%s/.mDolphin/certs/%lx.%u",
                        dir, X509_subject_name_hash(cert), i);

            fp=fopen(buf, "r");
            if (fp == NULL){
                /*File name doesn't exist so we can use it safely*/
                fp=fopen(buf, "w");
                if (fp == NULL){
                    printf("Unable to open cert save file in home dir\n");
                    break;
                } else {
                    PEM_write_X509(fp, cert);
                    fclose(fp);
                    retval = 0;
                    break;
                }
            } else {
                fclose(fp);
            }
            i++;
            /*Don't loop too many times - just give up*/
        } while( i < 1024 );

        free(dir);
    }
    return retval;
}

/*
 * Save Private Key with a hashed filename.
 * Return: 0 on success, 1 on failure.
 */
static int savePKeyHome( X509* cert, EVP_PKEY * pkey)
{
    char buf[4096];

    FILE * fp = NULL;
    unsigned int i = 0;
    int retval = 1;
    
    char *dir = get_current_dir_name();
    if (dir) {
        /*Attempt to create .dillo/certs blindly - check later*/
        snprintf(buf,4096,"%s/.mDolphin/", dir);
        if (opendir(buf)==NULL)
            mkdir(buf, 01777);
        snprintf(buf,4096,"%s/.mDolphin/certs/", dir);
        if (opendir(buf)==NULL)
            mkdir(buf, 01777);

        do{
            snprintf(buf,4096,"%s/.mDolphin/certs/%lx.%s",
                    dir, X509_issuer_name_hash(cert), "key");

            fp=fopen(buf, "r");
            if (fp == NULL){
                /*File name doesn't exist so we can use it safely*/
                fp=fopen(buf, "w");
                if (fp == NULL){
                    printf("Unable to open key save file in home dir\n");
                    break;
                } else {
                    PEM_write_PrivateKey(fp, pkey, NULL, NULL, 0, 0, NULL);
                    fclose(fp);
                    retval = 0;
                    break;
                }
            } else {
                fclose(fp);
            }
            i++;
            /*Don't loop too many times - just give up*/
        } while( i < 1024 );

        free(dir);
    }
    return retval;
}


static int loadPkcs12( const char *fn, const char* pw)
{
    /* you are lost in a maze of twisty crypto algorithms... */
    PKCS12 *p12;
    X509 *x509;
    EVP_PKEY *pkey;
    FILE *fp;
    int ret;
    fp = fopen(fn, "r");
    if (fp == NULL) {
        printf("Could not open file `%s\n", fn);
        return -1;
    }

    p12 = d2i_PKCS12_fp(fp, NULL);

    fclose(fp);

    if (p12 == NULL) {
        printf("Could not read certificate from file `%s\n",fn);
        return -1;
    }

    /* we need some more for the P12 decoding */
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_digests();
    ERR_load_crypto_strings();

    ret = PKCS12_parse(p12, getenv(pw),&pkey, &x509, NULL);
    saveCertificateHome(x509, true);
    savePKeyHome( x509, pkey);
    PKCS12_free(p12);

    if (ret != 1) {
        printf("Error parsing certificate (incorrect password?): %s\n", 
                ERR_reason_error_string(ERR_get_error()));
        return -1;
    }
    return 0;
}
static BOOL mdolphin_add_certificate(const char* fn, const char* passwd)
{
    if ( !loadPkcs12(fn, passwd))
        return true;
    else
        return false;
}

BOOL importCertificate(HWND hParent)
{
    char pw[128];
    char *fullname = NULL;
    BOOL result = false;
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
        "All file(*.*)|Pkcs12(*.p12)",
        1
    };
    int ret = ShowOpenDialog (hParent, 100, 100, 350, 250, &newfiledlg);
    if(ret == IDOK){
        int len = strlen(newfiledlg.filefullname)+1;
        fullname = (char *)malloc(len*sizeof(char));
        if (!fullname)
            return result;
        strcpy(fullname, newfiledlg.filefullname);
        inputPassword( hParent, pw);
        if (mdolphin_add_certificate( fullname, pw)) {
            MessageBox( hParent, "Successfully restored your security certificate(s) and private key(s)",
                    "Alert", MB_OK);
            result = true;
        } else {
            MessageBox( hParent, "Error parsing certificate (incorrect password?)", 
                    "Alert", MB_OK);
            result = false;
        }
        if (fullname)
            free(fullname);
    }
    return result;
}


BOOL readCertificateHome(unsigned long hash, X509 **x509, EVP_PKEY **pkey)
{
    FILE * fcrt = NULL;
    FILE * fkey = NULL;
    bool result = false;

    char *buf1 = new char[4096];
    char *buf2 = new char[4096];
    if (! buf1 && buf2)
        return false;

    char* dir = get_current_dir_name();
    if(dir) {
        snprintf(buf1, 4095, "%s/.mDolphin/certs/%lx.%s", dir, hash, "crt");
        snprintf(buf2, 4095, "%s/.mDolphin/certs/%lx.%s", dir, hash, "key");
        free(dir);

        fcrt = fopen(buf1, "r");
        fkey = fopen(buf2, "r");
        if (fcrt == NULL || fkey == NULL){
            printf("No certificate or private key file!\n");
            goto err;
        } 

        if (PEM_read_X509(fcrt, x509, NULL, NULL) && PEM_read_PrivateKey(fkey, pkey, NULL, NULL))
            result = true;
        else
            ERR_clear_error();
err: 
        if (fcrt)  
            fclose(fcrt);
        if (fkey)  
            fclose(fkey);
    }
    delete[] buf1;
    delete[] buf2;
    return result;
}
#endif
