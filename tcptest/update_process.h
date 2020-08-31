#ifndef __UPDATE_PROCESS_
#define __UPDATE_PROCESS_

typedef enum
{
    FILE_UPDATED = 0,               /*Update file transfer complete*/
    FILE_NAME_ERR = 1,              /*Update file name invalid*/
    FILE_MD5_ERR = 2,               /*Update file md5 check error*/
    FILE_VERSION_ERR = 3,           /*Update file version invalid*/  
    FILE_CONTENT_ERR = 4,           /*Update file content error*/
    FILE_COPY_ERR = 5,              /*Update file copy error*/
    FILE_HEADER_ERR = 6,            /*Update file header error*/
    FILE_OTHER_ERR = 7,             /*other error*/
    FILE_RECV_TIMEOUT_ERR = 8,      /*recv file content timeout*/
    SD_SPACE_NOT_ENOUGH = 9,        /*recv file content timeout*/
    MODEL_NAME_NOT_FIND_ERR = 10,   /*Not write model*/
    MODEL_NAME_MATCH_ERR = 11,      /*model name does not match*/
} Updatecode;

typedef struct _MSG_HEAD
{
    char file_name[128];
    int filename_len;
    int file_size;
    unsigned char md5[32]; 
    unsigned char version[4];
    int flag;
    int check_sum;
}MSG_HEAD;

int Update_Process_Thread();
int Start_Update_SD_file();

#endif
