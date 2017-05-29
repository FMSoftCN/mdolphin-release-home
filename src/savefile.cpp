#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <mgutils/mgutils.h>
#include <mdolphin/mdolphin.h>

#include "progress_dialog.h"
#include "savefile.h"
#include "error.h"
#include "mdhomeconfig.h"

//extern HWND g_current_hwnd;
//extern HWND g_tb_hwnd;
extern HWND  g_current_hwnd;
extern HWND g_tb_hwnd;

extern BOOL my_choosefile_callback (HWND hWnd, char* filename, size_t bufsize, BOOL IsSave);

struct nlist {
    BOOL inuse;
    char *url;
    char *filename;
    unsigned long long total;
    unsigned long long remain;
    HWND hwnd;
    BOOL canceled;
    int  id;
    int  percent;
};

static struct nlist dl_tab[64];

static struct nlist *lookup(int id)
{
    for (size_t i = 0; i < TABLESIZE(dl_tab); i++)
        if (id == dl_tab[i].id)
            return &(dl_tab[i]);

    return NULL;
}

static struct nlist *new_nlist(const char *url, const char *filename, unsigned long long total, int id)
{
    struct nlist *np = NULL;

    if (!url || !filename)
        return NULL;

    for (size_t i = 0; i < TABLESIZE(dl_tab); i++)
        if (!dl_tab[i].inuse){
            dl_tab[i].inuse = TRUE;
            np = &(dl_tab[i]);
            break; 
        }
    
    if (!np)
        return NULL;

    np->url = strdup(url);
    np->filename = strdup(filename);
    np->total = total;
    np->remain = total;
//    np->hwnd = newHwnd;
    np->canceled = FALSE;
    np->id = id;
    np->percent = 0;
    return np;
}

static void free_nlist(struct nlist* np)
{
    free (np->url);
    free (np->filename);
    np->inuse = FALSE;
    np->url = NULL;
    np->filename = NULL;
    np->total = np->remain = 0;
    np->hwnd = HWND_INVALID;
    np->canceled = FALSE;
    np->id = 0;
    np->percent = 0;
}

static void destroy_save_process(int id)
{
    struct nlist * np = NULL;

    if (!(np = lookup(id))) 
        return;

//    destroy_save_window (np->hwnd);

    free_nlist(np); 
}

static void start_save_process (int id)
{
}

static BOOL saveDownloadFile(const void* data, size_t datasize, 
    unsigned long remain, unsigned long totalsize, int id, BOOL overExist)
{
    struct nlist *np = NULL;
    FILE * fp = NULL;
    int percent = 0;
    char mes[512] = {0};

    if (!(np = lookup(id))) 
        return FALSE;

    {
        struct stat filestat;
        int status;    
        status = stat(np->filename, &filestat); 
        if (S_ISDIR(filestat.st_mode)) 
            return FALSE;

        if (!S_ISREG(filestat.st_mode)) 
        {
            int len = strlen(np->filename);
            int i;
            for (i=len-1; i>=0; i-- )
            {
                if (np->filename[i] == '/')     
                    break;
            }

            char dir[512];
            strncpy(dir, np->filename, i);
            dir[i]='\0';

            char cmd[512]="mkdir -p ";
            strcat(cmd, dir);
            system(cmd);
        }
    }

    fp = fopen(np->filename, (overExist==TRUE)?"w+": "a+");
    if (fp == NULL){
        return FALSE;
    } else {
        fwrite(data, 1, datasize, fp);
        fclose(fp);
        if (totalsize) {
            percent = 100 - int (remain*100/totalsize);
//            SendMessage(np->hwnd, PBM_SETPOS, percent, 0);
        } else {
            np->percent +=10;
            if (np->percent > 100)
                np->percent = 0;
//            SendMessage(np->hwnd, PBM_SETPOS, np->percent, 0);
        }
        return TRUE;
    }
}

BOOL my_choosefile_dia (HWND hWnd, const char *url, const char* defaultfilename, const char *MimeType , char *filename ,size_t bufsize, BOOL IsSave)
{
    char file_name[MY_NAMEMAX] = {0};
    char file_path[128] = {0};
/*    
	NEWFILEDLGDATA newfiledlg =
	{
#ifdef MINIGUI_V3
            FALSE, FALSE,
#else
	        FALSE,
#endif
			"",
			"",
			"/tmp",
			"All file(*.*)",
			1
	};
*/
    int len = strlen(defaultfilename);
    if (len > 0)
    {
        len = (MY_NAMEMAX > len)? len :MY_NAMEMAX;
//        strncpy(newfiledlg.filename, defaultfilename, len);
        strncpy(file_name, defaultfilename, len);
    }
    else
    {
       int i, j = -1;   
       i = strlen(url);
       i = i -2;
       for(; i> 0; i--)
       {
            if (url[i] == '/') 
                break;
            else if (j == -1 && url[i] == '?')
            {
                j=i; 
            }
       }
       
       j = (j==-1)?(strlen(url)):j;
//       strncpy(newfiledlg.filename, &(url[i+1]), j-i);
       strncpy(file_name, &(url[i+1]), j-i);
    }
/*
	if(IsSave)
		newfiledlg.IsSave = 1;   // IsSave =0 show open file , =1 show save file
	else
		newfiledlg.IsSave = 0;   // IsSave =0 show open file , =1 show save file
	int retvalue = ShowOpenDialog (hWnd, 100, 100, 350, 250, &newfiledlg);
	if (retvalue == IDOK)
		strcpy (filename, newfiledlg.filefullname);
	else
		return false;
*/
    GetValueFromEtcFile("widget.cfg", "global", "base_path", file_path, 128);
    sprintf(filename, "%s/%s", file_path,file_name);
	return true;

}

BOOL my_savefile_callback(int id, const char *url, const void* data, size_t datasize, unsigned long totalsize, const char *FileName, const char *MimeType ,BOOL isFinish)
{
#ifndef ENABLE_DOWNLOAD
    char * text = "This version don't support download!";
    char * caption = "WARNING";
  
     my_message_callback (g_current_hwnd, text, caption);
     //my_message_callback (g_current_hwnd, const_cast<char*>("This version don't support download!"), const_cast<char*>("This version don't support download!"));

    return FALSE;
#else 
//    ShowWindow(g_tb_hwnd, SW_HIDE);
    if (isFinish) {
        destroy_save_process(id);
        return FALSE;
    }

    struct nlist *np = NULL;
    char mes[512] = {0};
    char path[MAX_PATH+1] = {0};
//    HWND newHwnd = HWND_INVALID;
    np = lookup(id);

    if (!np) {
//        snprintf(mes, 510,"Do you want to save  url %s ?\n",url);
//        if (MessageBox(GetParent(g_current_hwnd), mes, "Alert", MB_OKCANCEL) == IDOK) {
            if (!my_choosefile_dia (GetParent(g_current_hwnd), url, FileName, MimeType ,path, MAX_PATH, true))
                return FALSE;

            np = new_nlist(url, path, totalsize, id);
            start_save_process(id); 
            if (totalsize)
                np->remain -= (unsigned long long )datasize;
//            return saveDownloadFile(data, datasize, np->remain, totalsize, id, TRUE);

            if(saveDownloadFile(data, datasize, np->remain, totalsize, id, TRUE) == TRUE)
            {
                if(np->remain == 0)
                {    
                sprintf(mes, "Download  url:%s finished!\n",url);
                MessageBox(GetParent(g_current_hwnd), mes, "Alert", MB_OKCANCEL);
                }
                return TRUE;
            }
            else
                return FALSE;
    } else {
        if (np->canceled) {
            destroy_save_process(id);
            return FALSE; 
        }
        if (np->total)
            np->remain -= (unsigned long long )datasize;
//        return saveDownloadFile(data, datasize, np->remain, totalsize, id, FALSE);
            if(saveDownloadFile(data, datasize, np->remain, totalsize, id, FALSE) == TRUE)
            {
                if(np->remain == 0)
                {    
                sprintf(mes, "Download  url:%s finished!\n",url);
                MessageBox(GetParent(g_current_hwnd), mes, "Alert", MB_OKCANCEL);
                }
                return TRUE;
            }
            else
                return FALSE;

    }
#endif
}

BOOL  my_saveas_callback(char * filename)
{
    ShowWindow(g_tb_hwnd, SW_HIDE);
    if (!my_choosefile_dia (GetParent(g_current_hwnd), NULL, filename, NULL ,filename, 0, true))
        return false;
    return true;
}
void cancel_save_progress(int id)
{
    struct nlist *np = NULL;
    np = lookup(id);
    np->canceled = TRUE;
}

