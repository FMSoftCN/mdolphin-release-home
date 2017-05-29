#ifndef _SAVEFILE_H
#define _SAVEFILE_H

BOOL  my_saveas_callback(char * filename);
BOOL my_savefile_callback (int id, const char * url, const void* data, size_t datasize, unsigned long totalsize,const char *FileName, const char *MimeType,  BOOL isFinish);

BOOL my_notsavefile_callback (int id, const char * url, const void* data, size_t datasize, unsigned long totalsize,const char *FileName, const char *MimeType,  BOOL isFinish);
#endif
