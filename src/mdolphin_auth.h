#ifndef __M_AUTH__
#define __M_AUTH__
BOOL my_provide_auth_callbackv3 (const char* title, char *username, size_t userNameLen,
        char *password, size_t passwordLen);
BOOL my_provide_auth_callback (const char* title, char *username, char *password);
#endif
