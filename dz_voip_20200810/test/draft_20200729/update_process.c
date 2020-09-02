#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/vfs.h>

#include "update_process.h"
#include "update_server.h"
#include "env.h"

//#define DOWNLOAD_PATH "/tmp/mmc0/"
#define SHELL_FILE_NAME  "update.sh"
#define DOWNLOAD_FILE_NAME "update_x.x.x.x.tar"
#define AU_FIRMWARE	"u-boot"
#define AU_KERNEL	"uImage"
#define AU_ROOTFS	"uRootfs"
#define VERSION_NAME "firmware_ver"
#define MODEL_NAME "devmodel"
#define UPDATE_FLAG "update_flag"

unsigned char Download_Path[32] = {0};

int GetSDFirstPartitionDevNodeName(void)
{
    char pathname[32];
    int dev=0;
    for (dev; dev<10; dev++)
    {
        sprintf(pathname,"/dev/mmcblk%d", dev);
        if (0 == access(pathname,F_OK))
        {
           return dev;
        }
    }
    return -1;
}

int check_sum(unsigned char *p, int len)
{
    int i;
    int sum = 0;
    for(i = 0; i < len; i++)
    {
        sum = sum + *(p + i);
    }
    return sum;
}

int check_model_name(const char *dir_name)
{
    char *p;
    char file_path[128] = {0};
    char new_model_name[5] = {0};
    const unsigned char * model_name = "dev_model";

    sprintf(file_path, "%s%s/%s", Download_Path, dir_name, "upgrade_cfg");
    if(0 != get_ver(file_path, model_name, new_model_name))
        return -1;
    if(!ReadEnvData())
    {
        printf("READENV ERR\n");
        return -1;
    }
    p = GetEnv(MODEL_NAME);
    if(p)
    {
        if(!strncmp(p, new_model_name, strlen(p)))
        {
            return 0;
        }
        else
        {  
            return -2;
        }   
    }
    else
    {
        return -1;
    }
}
int check_version(char *new_ver)
{
    //���汾
    short new_version, cur_version;
    char cur_ver[5] = {0};
    char *p;
    if(!ReadEnvData())
    {
        printf("READENV ERR\n");
        return -1;
    }
    p = GetEnv(VERSION_NAME);
    if(p)
        strncpy(cur_ver, p, 4);
    new_version = strtol(new_ver, NULL, 16);
    cur_version = strtol(cur_ver, NULL, 16);
    if(new_version == 0)
    {
        return 1;
    }
    else if(new_version <= cur_version)
    {
        return 0;
    }
    return 2;
}

int Search_update_file(char * dir_name, char *file_name)
{
    DIR * dir;
    struct dirent * ptr;
    int i;
    dir =opendir(dir_name);
    while((ptr = readdir(dir))!=NULL)
    {
        if(!strncmp(ptr->d_name,DOWNLOAD_FILE_NAME,7))
        {
            if(strstr(ptr->d_name,".tar"))
            {
                if(strlen(ptr->d_name) == strlen(DOWNLOAD_FILE_NAME))
                {
                    strncpy(file_name, ptr->d_name, strlen(DOWNLOAD_FILE_NAME));
                    closedir(dir);
                    return 0;
                }
            }
        }
    }
    closedir(dir);
    return -1;
}

int Firmware_update_first_start()
{
    char cmd[256] = {0};
    int dev_node = -1;
    char *p;
    char update_flag[2] = {0};
    update_flag[0] = '0';
    if(!ReadEnvData())
    {
        printf("READENV ERR\n");
        return -1;
    }
    p = GetEnv(UPDATE_FLAG);
    if(p == NULL)
    {
        printf("not find env UPDATE_FLAG\n");
        return 0;
    }
    else
    {
        printf("UPDATE_FLAG = %c\n", *p);
        if(*p == '1')
        {
            printf("firmware update first start\n");
            dev_node = GetSDFirstPartitionDevNodeName();
            if(dev_node < 0)
            {      
                printf("Not find SD card\n");
                return -1;
            }
            sprintf(Download_Path, "/tmp/mmc%d/", dev_node);

            sprintf(cmd, "find %s -name 'update_*' -type d | xargs rm -rf", Download_Path);        //ɾ����ʷ������sd���е��ļ�
            system(cmd);
            
            memset(cmd, 0 ,sizeof(cmd));        //���������ļ�
            sprintf(cmd, "mv %ssystem.ini /root/", Download_Path);
            system(cmd);

            memset(cmd, 0 ,sizeof(cmd));        //���������ļ�
            sprintf(cmd, "mv %saccess_auth /root/", Download_Path);
            system(cmd);
            
            if(!ReadEnvData())                  //���ù̼�������־
            {
                printf("READENV ERR\n");
                return -1;
            }
            if (!SetEnv(UPDATE_FLAG,update_flag))
            {
                printf("SETENV ERR\n");
                return -1;
            }
            if (!SaveEnvData())
            {
                printf("SAVEENV ERR\n");
                return -1;
            }
        }
    }
    return 0;
}

int Start_Update_SD_file()
{
    int dev_node = -1;
    char new_ver[5] = {0};
    char file_name[128] = {0};
    char cmd[256] = {0};
    char dir_name[128] = {0};
    int ver_status;
    int ret;
    int filename_len;
    int status;
    int shell_status;
    int status1, status2;
    unsigned char cal_md5[32];
    unsigned char get_md5[32];
    unsigned char md5_path[128] = {0};
    unsigned char cal_md5_path[128] = {0};
    FILE *fp;

    printf("start check sd update file\n");
    //��sd��Ѱ��������
    dev_node = GetSDFirstPartitionDevNodeName();
    if(dev_node < 0)
    {      
        printf("Not find SD card\n");
        return -1;
    }
    sprintf(Download_Path, "/tmp/mmc%d/", dev_node);

    ret = Search_update_file(Download_Path, file_name);
    if(ret)
    {
        printf("Not find update_x.x.x.x.tar\n");
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "rm %smd5.txt", Download_Path);
        system(cmd);
        return -1;
    }
    filename_len = strlen(file_name);

    sprintf(md5_path, "%smd5.txt", Download_Path);
    status = access(md5_path, F_OK);
    if(status)
    {
        printf("Not find md5.txt\n");
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "rm %s%s", Download_Path, file_name);
        system(cmd);
        return -1;
    }   
    //�汾�ż��
    new_ver[0] = file_name[7];
    new_ver[1] = file_name[9];
    new_ver[2] = file_name[11];
    new_ver[3] = file_name[13];
    ver_status = check_version(new_ver);
    if(!ver_status)                         //��Ч�汾��������
    {
        printf("Invalid software version number\n");
        memset(cmd, 0 ,sizeof(cmd));
        sprintf(cmd, "rm %s%s", Download_Path, file_name);
        system(cmd);
        return -1;   
    }
    
    //sprintf(cmd, "md5sum %s%s > %smd5.txt", Download_Path, file_info->file_name,Download_Path);
    //system(cmd);
    //sprintf(md5_path, "%smd5.txt", Download_Path);
    fp = fopen(md5_path, "r");
    if(NULL == fp)
    {
        printf("File:can not open file for read\n");
        return -1;
    }
    fread(get_md5, sizeof(cal_md5), 1, fp);
    fclose(fp);
    memset(cmd, 0, sizeof(cmd));
    //sprintf(cmd, "rm %smd5.txt", Download_Path);
    //system(cmd);

    sprintf(cal_md5_path, "%scal_md5.txt", Download_Path);
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "md5sum %s%s > %scal_md5.txt", Download_Path, file_name,Download_Path);
    system(cmd);
    fp = fopen(cal_md5_path, "r");
    if(NULL == fp)
    {
        printf("File:can not open file for read\n");
        return -1;
    }
    fread(cal_md5, sizeof(cal_md5), 1, fp);
    fclose(fp);
    memset(cmd, 0 ,sizeof(cmd));
    sprintf(cmd, "rm %scal_md5.txt", Download_Path);
    system(cmd);

    if(strncmp(cal_md5, get_md5, sizeof(cal_md5)))
    {
        printf("md5sum check error\n");
        memset(cmd, 0 ,sizeof(cmd));
        sprintf(cmd, "rm %s%s %smd5.txt", Download_Path, file_name, Download_Path);
        system(cmd);
        return -1;   
    }
    printf("md5sum check ok!\n");
    
    //��ѹ
    strncpy(dir_name, file_name, (filename_len - 4));
    sprintf(cmd, "tar -vxf %s%s -C %s", Download_Path, file_name, Download_Path);
    status = system(cmd);
    if(status == -1)
    {
        printf("tar system error\n");
        memset(cmd, 0 ,sizeof(cmd));
        sprintf(cmd, "rm %s%s", Download_Path, dir_name);
        system(cmd);
        sync();
        return -1;
    }
    sync();

    memset(cmd, 0 ,sizeof(cmd));
    sprintf(cmd, "%s%s", Download_Path, dir_name);
    status = access(cmd, F_OK);
    if(status == 0)   
    {
        memset(cmd, 0 ,sizeof(cmd));
        sprintf(cmd, "%s%s/%s", Download_Path, dir_name, SHELL_FILE_NAME);
        status = access(cmd, F_OK);
        if(status == 0)
        {
            system("killall -10 merecord");     //��������������������
            
            memset(cmd, 0 ,sizeof(cmd));
            sprintf(cmd, "sh %s%s/%s %s", Download_Path, dir_name, SHELL_FILE_NAME, dir_name);
            shell_status = system(cmd);
            
            if (-1 == shell_status)  
            {  
                printf("system error!");  
                return -1;
            }  
            else  
            {  
                printf("exit status value = [0x%x]\n", shell_status);  
                if (WIFEXITED(shell_status))  
                {  
                    if (0 == WEXITSTATUS(shell_status))  
                    {  
                        sync();
                        printf("run shell script successfully.\n");  
                    }  
                    else  
                    {  
                        printf("run shell script fail, script exit code: %d\n", WEXITSTATUS(shell_status)); 
                        return -1;
                    }  
                }  
                else  
                {  
                    printf("exit status = [%d]\n", WEXITSTATUS(shell_status));  
                    return -1;
                }  
            }  
            
            if(ver_status == 2)     //���¹̼��汾
            {
                if(!ReadEnvData())
                {
                    printf("READENV ERR\n");
                    return -1;
                }
                if (!SetEnv(VERSION_NAME,new_ver))
                {
                    printf("SETENV ERR\n");
                    return -1;
                }
                if (!SaveEnvData())
                {
                    printf("SAVEENV ERR\n");
                    return -1;
                }
            }
            printf("update success\n");
            memset(cmd, 0 ,sizeof(cmd));
            sprintf(cmd, "rm -rf %s%s %s%s %smd5.txt",Download_Path, dir_name, Download_Path, file_name, Download_Path);
            system(cmd);
            msleep(4000);
            sync();
            system("reboot");
            return 0;
        }
        else            //sd������
        {
            //model name check
            ret = check_model_name(dir_name);
            if(ret)
            {
                printf("model name dose not match\n");
                memset(cmd, 0 ,sizeof(cmd));
                sprintf(cmd, "rm -rf %s%s %s%s %smd5.txt",Download_Path, dir_name, Download_Path, file_name, Download_Path);
                system(cmd);
                return -1;   
            }
        
            memset(cmd, 0 ,sizeof(cmd));
            sprintf(cmd, "%s%s/%s", Download_Path, dir_name, AU_FIRMWARE);
            status = access(cmd, F_OK);
            memset(cmd, 0 ,sizeof(cmd));
            sprintf(cmd, "%s%s/%s", Download_Path, dir_name, AU_KERNEL);
            status1 = access(cmd, F_OK);
            memset(cmd, 0 ,sizeof(cmd));
            sprintf(cmd, "%s%s/%s", Download_Path, dir_name, AU_ROOTFS);
            status2 = access(cmd, F_OK);
            if(!(status && status1 && status2))
            {
                char update_flag[2] = {0};
                update_flag[0] = '1';
                memset(cmd, 0 ,sizeof(cmd));        //���������ļ�
                sprintf(cmd, "mv /root/system.ini %s", Download_Path);
                system(cmd);

                memset(cmd, 0 ,sizeof(cmd));        //���������ļ�
                sprintf(cmd, "mv /root/access_auth %s", Download_Path);
                system(cmd);
                if(!ReadEnvData())                  //���ù̼�������־
                {
                    printf("READENV ERR\n");
                    return -1;
                }
                if (!SetEnv(UPDATE_FLAG,update_flag))
                {
                    printf("SETENV ERR\n");
                    return -1;
                }
                if (!SaveEnvData())
                {
                    printf("SAVEENV ERR\n");
                    return -1;
                }

                printf("reboot for update\n");
                system("killall -10 merecord");     //��������������������
                memset(cmd, 0 ,sizeof(cmd));
                sprintf(cmd, "rm -rf %s%s %smd5.txt", Download_Path, file_name, Download_Path);
                system(cmd);
                sync();
                msleep(4000);
                system("reboot");
                return 0;
            }
            else
            {
                printf("no bin file to update\n");
                memset(cmd, 0 ,sizeof(cmd));
                sprintf(cmd, "rm -rf %s%s %s%s %smd5.txt",Download_Path, dir_name, Download_Path, file_name, Download_Path);
                system(cmd);
                sync();
                return -1;
            }
        }
    }
    else
    {
        printf("no dir to update\n");
        memset(cmd, 0 ,sizeof(cmd));
        sprintf(cmd, "rm -rf %s%s", Download_Path, file_name);
        system(cmd);
        sync();
        return -1;
    }
    return 0;
}

int Check_update_file(MSG_HEAD *file_info)
{
    int status, status1, status2;
    int filename_len;
    char cmd[256] = {0};
    unsigned char cal_md5[32];
    unsigned char md5_path[128] = {0};
    FILE *fp;
    char new_ver[5] = {0};
    int ver_status;
    int ret;
    
    //����ļ�����
    filename_len = ntohl(file_info->filename_len);
    printf("file_info->file_name = %s\n", file_info->file_name);
    printf("file_info->filename_len = %d\n", filename_len);
    if(strncmp(file_info->file_name,DOWNLOAD_FILE_NAME,7)||(NULL == strstr(file_info->file_name,".tar"))||(filename_len != strlen(DOWNLOAD_FILE_NAME)))
    {
        file_info->flag = htonl(FILE_NAME_ERR);
        file_info->check_sum = htonl(check_sum((unsigned char *)file_info, (sizeof(MSG_HEAD) - 4)));
        Send_Internet_Msg(&tcp_info, (unsigned char *)&file_info, sizeof(MSG_HEAD));
        memset(&file_info, 0, sizeof(MSG_HEAD));
        printf("file name error\n");
        memset(cmd, 0 ,sizeof(cmd));
        sprintf(cmd, "rm %s%s", Download_Path, file_info->file_name);
        system(cmd);
        return -1;
    }
    //���md5sum
    sprintf(cmd, "md5sum %s%s > %smd5.txt", Download_Path, file_info->file_name,Download_Path);
    system(cmd);
    sprintf(md5_path, "%smd5.txt", Download_Path);
    fp = fopen(md5_path, "r");
    if(NULL == fp)
    {
        file_info->flag = htonl(FILE_NAME_ERR);
        file_info->check_sum = htonl(check_sum((unsigned char *)file_info, (sizeof(MSG_HEAD) - 4)));
        Send_Internet_Msg(&tcp_info, (unsigned char *)&file_info, sizeof(MSG_HEAD));
        memset(&file_info, 0, sizeof(MSG_HEAD));
        printf("File:can not open file for read\n");
        return -1;
    }
    fread(cal_md5, sizeof(cal_md5), 1, fp);
    fclose(fp);
    //system("rm /tmp/mmc0/md5.txt");
    
    if(strncmp(file_info->md5, cal_md5, sizeof(cal_md5)))
    {
        printf("md5sum check error\n");
        file_info->flag = htonl(FILE_MD5_ERR);
        file_info->check_sum = htonl(check_sum((unsigned char *)file_info, (sizeof(MSG_HEAD) - 4)));
        Send_Internet_Msg(&tcp_info, (unsigned char *)file_info, sizeof(MSG_HEAD));
        memset(cmd, 0 ,sizeof(cmd));
        sprintf(cmd, "rm %s%s %smd5.txt", Download_Path, file_info->file_name, Download_Path);
        printf("cmd = %s\n", cmd);
        system(cmd);
        return -1;   
    }
    printf("md5sum check ok!\n");

    //�汾�ż��
    new_ver[0] = file_info->file_name[7];
    new_ver[1] = file_info->file_name[9];
    new_ver[2] = file_info->file_name[11];
    new_ver[3] = file_info->file_name[13];
    ver_status = check_version(new_ver);
    printf("ver_status = %d\n", ver_status);
    if(!ver_status)                         //��Ч�汾��������
    {
        printf("Invalid software version number\n");
        file_info->flag = htonl(FILE_VERSION_ERR);
        file_info->check_sum = htonl(check_sum((unsigned char *)file_info, (sizeof(MSG_HEAD) - 4)));
        Send_Internet_Msg(&tcp_info, (unsigned char *)file_info, sizeof(MSG_HEAD));
        memset(cmd, 0 ,sizeof(cmd));
        //sprintf(cmd, "rm %s%s", Download_Path, file_info->file_name);
        sprintf(cmd, "rm %s%s %smd5.txt", Download_Path, file_info->file_name, Download_Path);
        system(cmd);
        return -1;   
    }
    #if 0
    file_info->flag = htonl(FILE_UPDATED);
    file_info->check_sum = htonl(check_sum((unsigned char *)file_info, (sizeof(MSG_HEAD) - 4)));
    Send_Internet_Msg(&tcp_info, (unsigned char *)file_info, sizeof(MSG_HEAD));
    #endif
    return ver_status;
}

int Update_process(MSG_HEAD *file_info, int ver_flag)
{
    int status, status1, status2;
    int filename_len;
    char cmd[256] = {0};
    char new_ver[5] = {0};
    char dir_name[128] = {0};
    int shell_status;
    int ret;

    sprintf(cmd, "find %s -name 'update_*' -type d | xargs rm -rf", Download_Path);        //ɾ����ʷ������sd���е�����Ŀ¼
    system(cmd);
    
    filename_len = ntohl(file_info->filename_len);
    new_ver[0] = file_info->file_name[7];
    new_ver[1] = file_info->file_name[9];
    new_ver[2] = file_info->file_name[11];
    new_ver[3] = file_info->file_name[13];
     //��ѹ
    strncpy(dir_name, file_info->file_name, (filename_len - 4));
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "tar -vxf %s%s -C %s", Download_Path, file_info->file_name, Download_Path);
    status = system(cmd);
    if(status == -1)
    {
        printf("tar system error\n");
        memset(cmd, 0 ,sizeof(cmd));
        //sprintf(cmd, "rm %s%s", Download_Path, file_info->file_name);
        sprintf(cmd, "rm %s%s", Download_Path, dir_name);
        system(cmd);
        sync();
        file_info->flag = htonl(FILE_OTHER_ERR);
        file_info->check_sum = htonl(check_sum((unsigned char *)file_info, (sizeof(MSG_HEAD) - 4)));
        Send_Internet_Msg(&tcp_info, (unsigned char *)file_info, sizeof(MSG_HEAD));
        return -1;
    }
    sync();

    memset(cmd, 0 ,sizeof(cmd));
    sprintf(cmd, "%s%s", Download_Path, dir_name);
    status = access(cmd, F_OK);
    if(status == 0)
    {
        memset(cmd, 0 ,sizeof(cmd));
        sprintf(cmd, "%s%s/%s", Download_Path, dir_name, SHELL_FILE_NAME);
        status = access(cmd, F_OK);
        if(status == 0)
        {
            system("killall -10 merecord");     //��������������������
            
            memset(cmd, 0 ,sizeof(cmd));
            sprintf(cmd, "sh %s%s/%s %s", Download_Path, dir_name, SHELL_FILE_NAME, dir_name);
            shell_status = system(cmd);
            sync();
            if (-1 == shell_status)  
            {  
                printf("system error!");  
                file_info->flag = htonl(FILE_COPY_ERR);
                file_info->check_sum = htonl(check_sum((unsigned char *)file_info, (sizeof(MSG_HEAD) - 4)));
                Send_Internet_Msg(&tcp_info, (unsigned char *)file_info, sizeof(MSG_HEAD));
                return -1;
            }  
            else  
            {  
                printf("exit status value = [0x%x]\n", shell_status);  
                if (WIFEXITED(shell_status))  
                {  
                    if (0 == WEXITSTATUS(shell_status))  
                    {  
                        printf("run shell script successfully.\n");  
                    }  
                    else  
                    {  
                        printf("run shell script fail, script exit code: %d\n", WEXITSTATUS(shell_status)); 
                        file_info->flag = htonl(FILE_COPY_ERR);
                        file_info->check_sum = htonl(check_sum((unsigned char *)file_info, (sizeof(MSG_HEAD) - 4)));
                        Send_Internet_Msg(&tcp_info, (unsigned char *)file_info, sizeof(MSG_HEAD));
                        return -1;
                    }  
                }  
                else  
                {  
                    printf("exit status = [%d]\n", WEXITSTATUS(shell_status));  
                    file_info->flag = htonl(FILE_COPY_ERR);
                    file_info->check_sum = htonl(check_sum((unsigned char *)file_info, (sizeof(MSG_HEAD) - 4)));
                    Send_Internet_Msg(&tcp_info, (unsigned char *)file_info, sizeof(MSG_HEAD));
                    return -1;
                }  
            }  
            
            if(ver_flag == 2)     //���¹̼��汾
            {
                if(!ReadEnvData())
                {
                    printf("READENV ERR\n");
                    return -1;
                }
                if (!SetEnv(VERSION_NAME,new_ver))
                {
                    printf("SETENV ERR\n");
                    return -1;
                }
                if (!SaveEnvData())
                {
                    printf("SAVEENV ERR\n");
                    return -1;
                }
            }
            printf("update success\n");
            //file_info->flag = htonl(FILE_UPDATED);
            //Send_Internet_Msg(&tcp_info, (unsigned char *)file_info, sizeof(MSG_HEAD));
            memset(cmd, 0 ,sizeof(cmd));
            sprintf(cmd, "rm -rf %s%s %s%s %smd5.txt",Download_Path, dir_name, Download_Path, file_info->file_name, Download_Path);
            system(cmd);
            sync();
            msleep(4000);
            system("reboot");
            return 0;
        }
        else            //sd������
        {
            //model name check
            ret = check_model_name(dir_name);
            if(ret)
            {
                if(ret == -1)
                {
                    printf("model name not find\n");
                    file_info->flag = htonl(MODEL_NAME_NOT_FIND_ERR);
                }
                else if(ret == -2)
                {
                    printf("model name dose not match\n");
                    file_info->flag = htonl(MODEL_NAME_MATCH_ERR);
                }
                file_info->check_sum = htonl(check_sum((unsigned char *)file_info, (sizeof(MSG_HEAD) - 4)));
                Send_Internet_Msg(&tcp_info, (unsigned char *)file_info, sizeof(MSG_HEAD));
                memset(cmd, 0 ,sizeof(cmd));
                sprintf(cmd, "rm -rf %s%s %s%s %smd5.txt",Download_Path, dir_name, Download_Path, file_info->file_name, Download_Path);
                system(cmd);
                return -1;   
            }
            file_info->flag = htonl(FILE_UPDATED);
            file_info->check_sum = htonl(check_sum((unsigned char *)file_info, (sizeof(MSG_HEAD) - 4)));
            Send_Internet_Msg(&tcp_info, (unsigned char *)file_info, sizeof(MSG_HEAD));
            printf("model name check ok!\n");
                
            memset(cmd, 0 ,sizeof(cmd));
            sprintf(cmd, "%s%s/%s", Download_Path, dir_name, AU_FIRMWARE);
            status = access(cmd, F_OK);
            memset(cmd, 0 ,sizeof(cmd));
            sprintf(cmd, "%s%s/%s", Download_Path, dir_name, AU_KERNEL);
            status1 = access(cmd, F_OK);
            sprintf(cmd, "%s%s/%s", Download_Path, dir_name, AU_ROOTFS);
            status2 = access(cmd, F_OK);
            if(!(status && status1 && status2))
            {
                char update_flag[2] = {0};
                update_flag[0] = '1';
                memset(cmd, 0 ,sizeof(cmd));        //���������ļ�
                sprintf(cmd, "mv /root/system.ini %s", Download_Path);
                system(cmd);

                memset(cmd, 0 ,sizeof(cmd));        //���������ļ�
                sprintf(cmd, "mv /root/access_auth %s", Download_Path);
                system(cmd);
                if(!ReadEnvData())                  //���ù̼�������־
                {
                    printf("READENV ERR\n");
                    return -1;
                }
                if (!SetEnv(UPDATE_FLAG,update_flag))
                {
                    printf("SETENV ERR\n");
                    return -1;
                }
                if (!SaveEnvData())
                {
                    printf("SAVEENV ERR\n");
                    return -1;
                }
                system("killall -10 merecord");     //��������������������
                printf("reboot for update\n");
                memset(cmd, 0 ,sizeof(cmd));
                sprintf(cmd, "rm -rf %s%s %smd5.txt", Download_Path, file_info->file_name, Download_Path);
                system(cmd);
                sync();
                msleep(4000);
                system("reboot");
                return 0;
            }
            else
            {
                printf("no bin file to update\n");
                file_info->flag = htonl(FILE_CONTENT_ERR);
                file_info->check_sum = htonl(check_sum((unsigned char *)file_info, (sizeof(MSG_HEAD) - 4)));
                Send_Internet_Msg(&tcp_info, (unsigned char *)file_info, sizeof(MSG_HEAD));
                memset(cmd, 0 ,sizeof(cmd));
                sprintf(cmd, "rm -rf %s%s %s%s %smd5.txt",Download_Path, dir_name, Download_Path, file_info->file_name, Download_Path);
                system(cmd);
                sync();
                return -1;
            }
        }
    }
    else
    {
        printf("no dir to update\n");
        memset(cmd, 0 ,sizeof(cmd));
        sprintf(cmd, "rm -rf %s%s", Download_Path, file_info->file_name);
        system(cmd);
        return -1;
    }
}

int GetStorageInfo(char * MountPoint,  //SD�����һ������
                                 int *Capacity,  //  ��Ҫ��ȡ�Ŀռ��С
                                 int type) //��ȡʲô���͵Ŀռ�
{
    struct statfs statFS; //ϵͳstat�Ľṹ��
    uint64 usedBytes = 0;
    uint64 freeBytes = 0;
    uint64 totalBytes = 0;
    uint64 endSpace = 0;

    if (statfs(MountPoint, &statFS) == -1){   //��ȡ������״̬
        printf("statfs failed for path->[%s]\n", MountPoint);
        return(-1);
    }

    totalBytes = (uint64)statFS.f_blocks * (uint64)statFS.f_frsize; //��ϸ�ķ����������� ���ֽ�Ϊ��λ
    freeBytes = (uint64)statFS.f_bfree * (uint64)statFS.f_frsize; //��ϸ��ʣ��ռ����������ֽ�Ϊ��λ
    usedBytes = (uint64)(totalBytes - freeBytes); //��ϸ��ʹ�ÿռ����������ֽ�Ϊ��λ

    switch( type )
    {
        case 1:
        endSpace = totalBytes/1024; //��KBΪ��λ��������
        break;

        case 2:
        endSpace = usedBytes/1024; //��KBΪ��λ��ʹ�ÿռ�
        break;

        case 3:
        endSpace = freeBytes/1024; //��KBΪ��λ��ʣ��ռ�
        break;

        default:
        return ( -1 );
    }
    *Capacity = endSpace; //�������˵�˰�
    return 0;
}

int Update_Process_Thread()
{
    int ret;
    MSG_HEAD Msg_header;
    char filename[256];
    int file_size;
    int filename_len;
    int last_size;
    int i;
    int write_length;
    char databuf[4096] = {0};
    FILE *fp;
    int status;
    pid_t pid;
    int cal_sum = 0;
    int get_sum = 0;
    int dev_node = -1;
    int recv_size = 0;
    while(1)
    {
        if(connect_state == 1)      //�ͻ���������
        {
            recv_size = 0;
            //status = access("/dev/mmcblk0", F_OK);
            dev_node = GetSDFirstPartitionDevNodeName();
            if(dev_node >= 0)
            {      
                sprintf(Download_Path, "/tmp/mmc%d/", dev_node);
                //step1 �ȴ�����������Ϣͷ��Ϣ
                ret = Recv_Internet_Msg(&tcp_info, (unsigned char *)&Msg_header, sizeof(MSG_HEAD), 0);
                if(ret != sizeof(MSG_HEAD))     //�Ƿ���Ϣͷ
                {
                    memset(&Msg_header, 0, sizeof(MSG_HEAD));
                    continue;
                }   
                cal_sum = check_sum((unsigned char *)&Msg_header, (sizeof(MSG_HEAD) - 4));
                get_sum = ntohl(Msg_header.check_sum);
#if 1
                printf("cal_sum = %d\n", cal_sum);
                printf("*****************head msg*******************\n");
                printf("file_name = %s\n", Msg_header.file_name);
                printf("filename_len = %d\n", ntohl(Msg_header.filename_len));
                printf("file_size = %d\n", ntohl(Msg_header.file_size));
                unsigned char buffer[64] = {0};
                memcpy(buffer, Msg_header.md5, 32);
                printf("md5 = %s\n", buffer);
                memset(buffer, 0, sizeof(buffer));
                memcpy(buffer, Msg_header.version, 4);
                printf("version = %s\n", buffer);
                printf("flag = %d\n", ntohl(Msg_header.flag));
                printf("check_sum = %d\n", ntohl(Msg_header.check_sum));
                printf("*****************************************\n");
#endif
                if(cal_sum != get_sum)
                {
                    printf("msg head checksum error\n");
                    Msg_header.flag = htonl(FILE_HEADER_ERR);
                    Msg_header.check_sum = htonl(check_sum((unsigned char *)&Msg_header, (sizeof(MSG_HEAD) - 4)));
                    Send_Internet_Msg(&tcp_info, (unsigned char *)&Msg_header, sizeof(MSG_HEAD));
                    memset(&Msg_header, 0, sizeof(MSG_HEAD));
                    continue;
                }
                //T��ʣ��ռ��ж�
                int freespace = 0;

                GetStorageInfo(Download_Path, &freespace, 3);
                printf("sd_free_space = %d\n", freespace);
                if(freespace < 50000)       //�洢�ռ�С��50M
                {
                    printf("sd card space not enough\n");
                    Msg_header.flag = htonl(SD_SPACE_NOT_ENOUGH);
                    Msg_header.check_sum = htonl(check_sum((unsigned char *)&Msg_header, (sizeof(MSG_HEAD) - 4)));
                    Send_Internet_Msg(&tcp_info, (unsigned char *)&Msg_header, sizeof(MSG_HEAD));
                    memset(&Msg_header, 0, sizeof(MSG_HEAD));
                    continue;
                }
                
                filename_len = ntohl(Msg_header.filename_len);
                file_size = ntohl(Msg_header.file_size);
                sprintf(filename, "%s", Download_Path);
                i = strlen(Download_Path);
                strncpy(filename + i, Msg_header.file_name, filename_len);
                
                //step2 �������������ļ�
                
                fp = fopen(filename, "w+");
                if(NULL == fp)
                {
                    printf("File:can not creat file for write file_name = %s\n", filename);
                    continue;
                }
                last_size = file_size;
                while(last_size > 0)
                {
                    //printf("last_size = %d\n", last_size);
                    memset(databuf, 0, sizeof(databuf));
                    if(last_size > sizeof(databuf))
                    {
                        i = Recv_Internet_Msg(&tcp_info, databuf, sizeof(databuf), 1);
                        //printf("Byte recrived i = %d\n", i);
                        if(i <= 0)
                        {
                            Msg_header.flag = htonl(FILE_RECV_TIMEOUT_ERR);
                            Msg_header.check_sum = htonl(check_sum((unsigned char *)&Msg_header, (sizeof(MSG_HEAD) - 4)));
                            Send_Internet_Msg(&tcp_info, (unsigned char *)&Msg_header, sizeof(MSG_HEAD));
                            close(tcp_info.client_fd);
                            tcp_info.client_fd = -1;
                            connect_state = 0;
                            fclose(fp);
                            remove(filename);
                            printf("close connet, only recived = %d\n", recv_size);
                            goto LABLE;
                        }
                        recv_size += i;
                        write_length = fwrite(databuf,sizeof(char),i,fp);
                        if(write_length < i)
                        {
                            printf("File:write file failed\n");
                        }
                    }
                    else
                    {
                        //printf("I will recv %d byte only\n", last_size);
                        i = Recv_Internet_Msg(&tcp_info, databuf, last_size, 1);
                        if(i <= 0)
                        {
                            
                            Msg_header.flag = htonl(FILE_RECV_TIMEOUT_ERR);
                            Msg_header.check_sum = htonl(check_sum((unsigned char *)&Msg_header, (sizeof(MSG_HEAD) - 4)));
                            Send_Internet_Msg(&tcp_info, (unsigned char *)&Msg_header, sizeof(MSG_HEAD));
                            close(tcp_info.client_fd);
                            tcp_info.client_fd = -1;
                            connect_state = 0;
                            fclose(fp);
                            remove(filename);
                            printf("close connet, only recived = %d\n", recv_size);
                            goto LABLE;
                        }
                        //printf("Byte recrived i = %d\n", i);
                        int write_length = fwrite(databuf,sizeof(char),i,fp);
                        if(write_length < i)
                        {
                            printf("File:write file failed\n");
                        }
                    }
                    last_size = last_size - i;
                }
                fclose(fp);
                sync();
                printf("File receiving complete\n");
                ret = Check_update_file(&Msg_header);
                if(ret < 0)
                    continue;
                Update_process(&Msg_header, ret);
                //Check_update(&Msg_header);
            }
            else
            {
                msleep(1000);
            }
        }
        else
        {
LABLE:         msleep(200);
        }
    }
    return 0;
}

