#include <string.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <mgutils/mgutils.h>
#include "clientcert.h"

#if ENABLE_SSL
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

#if ENABLE_SSLFILE
BOOL setVerifyCAPath ()
{
    char buf[4096];
    memset(&buf, 0, sizeof(buf));
    char * dir = get_current_dir_name();
    if (dir) {
        snprintf(buf,4096,"%s/.mDolphin/certs/", dir);
        free(dir);
    }
    return mdolphin_set_caPath(buf);
}
#endif

#define MAXSIZE 64
static unsigned long hash[MAXSIZE];
static int count = MAXSIZE+1;

static bool verifyCert(unsigned long subjectHash)
{
    int i;
    for ( i = 0; i<count; i++){
        if (hash[i] == subjectHash)
            return true;
    }

    return false;
}

static void setVerifyCert(unsigned long subjectHash)
{
    if (count == (MAXSIZE+1)) {
        memset(hash, 0, MAXSIZE*sizeof(unsigned long));
        count = 0;
    }
    
    if (count == MAXSIZE)
        count = 0;
    hash[count] = subjectHash;
    count++;
}

CERT_RESULT verifyCertificate(HWND hParent, CERT_RESULT result, X509 *x509)
{
    unsigned long sHash = X509_subject_name_hash(x509);
    if ( verifyCert (sHash)) {
        return X509_V_OK;
    }
    
    int id;
    switch (result) {
        case X509_V_OK:      /*Everything is Kosher*/
            id = IDOK;
            break;
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
        case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
        case X509_V_ERR_UNABLE_TO_GET_CRL:
            id = MessageBox(hParent,
                    "The issuer for the remote certificate cannot be found\n"
                    "The authenticity of the remote certificate cannot be trusted", 
                    "Verify certificate",
                    MB_OKCANCEL);
            break;
        case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
        case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
        case X509_V_ERR_CERT_SIGNATURE_FAILURE:
        case X509_V_ERR_CRL_SIGNATURE_FAILURE:
            id = MessageBox(hParent,
                    "The remote certificate signature could not be read\n"
                    "or is invalid and should not be trusted", 
                    "Verify certificate",
                    MB_OKCANCEL);
            break;
        case X509_V_ERR_CERT_NOT_YET_VALID:
        case X509_V_ERR_CRL_NOT_YET_VALID:
            id = MessageBox(hParent,
                    "Part of the remote certificate is not yet valid\n"
                    "Certificates usually have a range of dates over which\n"
                    "they are to be considered valid, and the certificate\n"
                    "presented has a starting validity after today's date\n"
                    "You should be cautious about using this site", 
                    "Verify certificate",
                    MB_OKCANCEL);
            break;
        case X509_V_ERR_CERT_HAS_EXPIRED:
        case X509_V_ERR_CRL_HAS_EXPIRED:
            id = MessageBox(hParent,
                    "The remote certificate has expired.  The certificate\n"
                    "wasn't designed to last this long. You should avoid \n"
                    "this site.",
                    "Verify certificate",
                    MB_OKCANCEL);
            break;
        case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
        case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
        case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
        case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
        case X509_V_ERR_OUT_OF_MEM:
            id = MessageBox(hParent,
                    "There was an error in the certificate presented.\n"
                    "Some of the certificate data was improperly formatted\n"
                    "making it impossible to determine if the certificate\n"
                    "is valid. You should not trust this certificate.",
                    "Verify certificate",
                    MB_OKCANCEL); 
            break;
        case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
        case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
            id = MessageBox(hParent,
                    "The remote certificate is self-signed.\n"
                    "Do you want continue?",
                    "Verify certificate",
                    MB_OKCANCEL);
            break;
        case X509_V_ERR_INVALID_CA:
        case X509_V_ERR_INVALID_PURPOSE:
        case X509_V_ERR_CERT_UNTRUSTED:
        case X509_V_ERR_CERT_REJECTED:
        case X509_V_ERR_KEYUSAGE_NO_CERTSIGN:
            id =  MessageBox(hParent,
                    "One of the certificates in the chain is being used\n"
                    "incorrectly (possibly due to configuration problems\n"
                    "with the remote system.  The connection should not", 
                    "Verify certificate",
                    MB_OKCANCEL);
            break;
            /* These are 'informational' when looking for issuer cert */
        case X509_V_ERR_SUBJECT_ISSUER_MISMATCH:
        case X509_V_ERR_AKID_SKID_MISMATCH:
        case X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH:
            id = MessageBox(hParent,
                    "Some of the information presented by the remote system\n"
                    "does not match other information presented\n"
                    "This may be an attempt to evesdrop on communications",
                    "Verify certificate", 
                    MB_OKCANCEL);
            break;
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
        case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
        case X509_V_ERR_CERT_CHAIN_TOO_LONG:
        case X509_V_ERR_CERT_REVOKED:
        case X509_V_ERR_PATH_LENGTH_EXCEEDED:
        case X509_V_ERR_UNABLE_TO_GET_CRL_ISSUER:
        case X509_V_ERR_UNHANDLED_CRITICAL_EXTENSION:
        default: 
            id = MessageBox(hParent,
                    "The remote certificate cannot be verified. Do you want continue?", 
                    "Verify certificate",
                    MB_OKCANCEL);
    }
    if (id == IDOK) {
#if ENABLE_SSLFILE
        if (saveCertificateHome(x509, false))
            printf("Save Certificate is error!\n");
#endif
        setVerifyCert(sHash);
        return X509_V_OK;
    }

    return result;
}
#endif
