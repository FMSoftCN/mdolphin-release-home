#ifndef _ERROR_H
#define _ERROR_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct _ERR_INFO {
    char* caption;
    char* msg;
	int err;
	BOOL hl1;
	BOOL hl2;
	HWND hParent;
} ERR_INFO;
void my_message_callback (HWND parent, const char * text, const char * caption);
#ifdef __cplusplus
}
#endif

#endif
