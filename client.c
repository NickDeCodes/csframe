/*************************************************************************
 > File Name: Client.c
 > Author:
 > Mail: 
 > Created Time: 六  3/16 14:09:59 2019
     Name        Path       Time
 0   Cpu       CpuLog.sh      5
 1   Mem       MemLog.sh      5
 2   Disk      DiskLog.sh     60
 3   Proc      ProcLog.sh     30
 4   SysInfo   SysInfo.sh     60
 5   Users      Users.sh      60
 ************************************************************************/

#include "./lib/conf.h"
#include "./lib/file.h"
#include "./lib/socket.h"
#include "./lib/threadpool.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/file.h>

#define config "./client.conf"
#define MAX_SIZE 1024
#define SIZE 128
#define N 6

struct Client_Conf{
    char *Master_Ip;//Master_Ip地址
    char *Client_Ip;//Client_Ip地址
    char *Log_Dir;//日志文件目录
    char *Log_Backup;//日志文件备份目录
    char *Sys_Log;//系统日志文件目录
    int Master_Port;//Master端口
    int Heart_Port;//心跳端口
    int Ctrl_Port;//控制端口
    int Data_Port;//数据端口
    int Warn_Port;//警告端口
    int R_W_Times;//读写次数
    int Self_Test;//自测次数
    int File_Size;//文件大小
}conf;

char *Name[N] = {"Name0", "Name1", "Name2", "Name3", "Name4", "Name5"};
char *Path[N] = {"Path0", "Path1", "Path2", "Path3", "Path4", "Path5"};
char *Time[N] = {"Time0", "Time1", "Time2", "Time3", "Time4", "Time5"};
char *Log[N] = {"Log0", "Log1", "Log2", "Log3", "Log4", "Log5"};
char *Back[N] = {"Back0", "Back1", "Back2", "Back3", "Back4", "Back5"};
char *BLog[N] = {"BLog0", "BLog1", "BLog2", "BLog3", "BLog4", "BLog5"};

typedef struct script{
    char *Name;//脚本名字
    char *Path;//脚本路径
    char *Log;//日志文件名字
    char *Back;//压缩文件名字
    char *BLog;//解压后的文件
    int Time;//执行时间
    int ID;//执行ID
}script;
script SC[N];//定义六个脚本结构体

struct shared{
    int cnt;     //检测次数
    int time;    //心跳次数
    pthread_mutex_t mutex;//互斥变量
    pthread_cond_t ready;//条件变量
};
struct shared *cond;//共享内存状态结构体

char *share_memory = NULL;//共享内存首地址
double DyAver = 0;//动态平均值
pthread_mutexattr_t m_attr;//共享互斥属性
pthread_condattr_t c_attr;//共享条件变量

void popen_script(int type) {
    char buffer[4 * MAX_SIZE] = {0};
    FILE *fstream = NULL;
    while(1) {
        for (int i = 0; i < conf.R_W_Times; i++) {
            SC[1].Path = (char *)get_conf_str(config, "Path1");
            if (type == 1) sprintf(SC[1].Path, "%s %f", SC[1].Path, DyAver);
            char buff[4 * SIZE] = {0};
            char test[4 * SIZE] = {0};
            if (NULL == (fstream = popen(SC[type].Path, "r"))) {
                exit(1);
            }
            if ((100 + type) == SC[1].ID) {
                if (NULL != fgets(buff, sizeof(buff), fstream)) {
                    strcat(buffer, buff);
                }
                if (NULL != fgets(buff, sizeof(buff), fstream)) {
                    DyAver = atof(buff);
                }else {
                    DyAver = 0;
                }
            } else {
                while (NULL != fgets(buff, sizeof(buff), fstream)) {
                    strcat(buffer, buff);
                    strcpy(test, buff);
                    if (strtok_func(test, "note\n", " ") || strtok_func(test, "warning\n", " ")) {
                        memset(test, 0, sizeof(test));
                        sprintf(test, "%d %s", 100 + type, buff);
                        if (socket_connect_udp(conf.Warn_Port, conf.Master_Ip, test) < 0) {
                        }
                        memset(buff, 0, sizeof(buff));
                    }
                }
            }
            sleep(SC[type].Time);
            pclose(fstream);
            if ((100 + type) == SC[0].ID) {
                fflush(stdout);
                pthread_mutex_lock(&cond->mutex);
                if (cond->cnt++ >= conf.Self_Test - 1) {
                    if (cond->time == 0) {
                        pthread_cond_signal(&cond->ready);
                    }
                    cond->cnt = 0;
                }
                pthread_mutex_unlock(&cond->mutex);
            }
        }
        FILE *file = fopen(SC[type].Log, "a+");
        if (NULL == file) {
            exit(1);
        }
        //建立文件锁
        if (flock(file->_fileno, LOCK_EX) < 0) {
        }
        fwrite(buffer, 1, strlen(buffer), file);
        fclose(file);
        int len;
        if ((len = file_size(SC[type].Log)) <  0) {
        }
        if (len >= (conf.File_Size * MAX_SIZE * MAX_SIZE)) {
            if (file_backup(SC[type].Log, SC[type].Back) < 0) {
            }
        }
    }
}

/*心跳检测*/
bool heart_test(int port, char *host) {
    bool ret = true;
    int fd;
    if ((fd = socket_connect_tcp(port, host, 0, 0.5)) < 0) {
        ret = false;
    }
    close(fd);
    return ret;
}

int main() {
    //获取配置参数
    //循环获取脚本参数
    for (int i = 0; i < N; i++) {
        SC[i].Name = (char *)get_conf_str(config, Name[i]);//获取脚本名字
        SC[i].Path = (char *)get_conf_str(config, Path[i]);//获取脚本路径
        SC[i].Time = get_conf_int(config, Time[i]);//获取脚本运行间隔时间
        SC[i].Log = (char *)get_conf_str(config, Log[i]);//获取脚本日志名字
        SC[i].Back = (char *)get_conf_str(config, Back[i]);//获取压缩日志文件名字
        SC[i].BLog = (char *)get_conf_str(config, BLog[i]);//获取解压后的文件
        SC[i].ID = 100 + i;//脚本执行ID
    }
    //获取其它配置参数
    conf.Master_Ip = (char *)get_conf_str(config, "Master_Ip");//获取Master_Ip
    conf.Client_Ip = (char *)get_conf_str(config, "Client_Ip");//获取Client_Ip
    conf.Log_Dir = (char *)get_conf_str(config, "Log_Dir");//获取日志目录
    conf.Log_Backup = (char *)get_conf_str(config, "Log_Backup");//获取日志备份目录
    conf.Sys_Log = (char *)get_conf_str(config, "Sys_Log");//获取系统日志;
    conf.Master_Port = get_conf_int(config, "Master_Port");//获取Master_Port
    conf.Heart_Port = get_conf_int(config, "Heart_Port");//获取心跳端口
    conf.Ctrl_Port = get_conf_int(config, "Ctrl_Port");//获取控制端口
    conf.Data_Port = get_conf_int(config, "Data_Port");//获取数据端口
    conf.Warn_Port = get_conf_int(config, "Warn_Port");//获取警告端口
    conf.R_W_Times = get_conf_int(config, "R_W_Times");//获取读写次数
    conf.Self_Test = get_conf_int(config, "Self_Test");//获取自检侧次数
    conf.File_Size = get_conf_int(config, "File_Size");//获取文件大小
    
    int shmid;//设置共享内存；
    char *share_memory = NULL;//分配的共享内存的原始首地址
    mkdir(conf.Log_Dir, 0755);
    mkdir(conf.Log_Backup, 0755);
    
    //创建共享内存
    if ((shmid = shmget(IPC_PRIVATE, sizeof(struct shared), 0666|IPC_CREAT)) == -1) {
        return -1;
    }
    //将共享内存连接到当前进程的地址空间
    if ((share_memory = (char *)shmat(shmid, 0, 0)) == NULL) {
        return -1;
    }
    cond = (struct shared*)share_memory;
    cond->cnt = 0;//初始化心跳次数
    cond->time = 0;//初始化检测次数
    pthread_mutexattr_init(&m_attr);//初始化共享互斥属性
    pthread_condattr_init(&c_attr);//初始化共享条件变量
    pthread_mutexattr_setpshared(&m_attr, PTHREAD_PROCESS_SHARED);//设置共享
    pthread_condattr_setpshared(&c_attr, PTHREAD_PROCESS_SHARED);//设置共享
    pthread_mutex_init(&cond->mutex, &m_attr);//初始化锁
    pthread_cond_init(&cond->ready, &c_attr);//初始化条件
    
    int pid_0;//父进程登陆
    int loginfd;
    char name[MAX_SIZE] = {0};
    char message[MAX_SIZE] = {0};
    loginfd = socket_connect_tcp(conf.Master_Port, conf.Master_Ip, 0, 0.5);
    gethostname(name, sizeof(name));
    send(loginfd, name, strlen(name), 0);
    if (recv(loginfd, (char *)&message, sizeof(message), 0) > 0) {
        printf("\033[35m服务端发送消息：\033[0m%s\n", message);
    }
    if((pid_0 = fork()) < 0) {
        perror("fork");
        return -1;
    }
    //父进程心跳监听
    if (pid_0 != 0) {
        int heart_listen;
        if ((heart_listen = socket_create_tcp(conf.Heart_Port)) < 0) {
            return -1;
        }
        while(1) {
            int heart_fd;
            if ((heart_fd = accept(heart_listen, NULL, NULL)) < 0) {
                close(heart_fd);
            }
            close(heart_fd);
        }
    } else {//儿子进程
        int pid_1;
        if ((pid_1 = fork()) < 0) {
            return -1;
        }
        if (pid_1 == 0) {
            while (1) {
                pthread_mutex_lock(&cond->mutex);
                pthread_cond_wait(&cond->ready, &cond->mutex);
                pthread_mutex_unlock(&cond->mutex);
                while(1) {
                    if (heart_test(conf.Master_Port, conf.Master_Ip)) {
                        pthread_mutex_lock(&cond->mutex);
                        cond->time = 0;
                        cond->cnt = 0;
                        pthread_mutex_unlock(&cond->mutex);
                        fflush(stdout);
                        break;
                    } else {
                        pthread_mutex_lock(&cond->mutex);
                        cond->time++;
                        pthread_mutex_unlock(&cond->mutex);
                        fflush(stdout);
                    }
                    sleep(6 * cond->time);
                    if (cond->time > conf.Self_Test) cond->time = conf.Self_Test;
                    pthread_mutex_unlock(&cond->mutex);
                }
            }
        } else {//返回儿子进程
            int ID = 0;
            int pid_2;
            for(int i = 0; i < N; i++)  {
                ID = i;
                if((pid_2 = fork()) < 0) {
                    continue;
                }
                if(pid_2 == 0) break;
            }
            if (pid_2 == 0) {
                popen_script(ID);
            } else {
                int ctrl_listen;
                if ((ctrl_listen = socket_create_tcp(conf.Ctrl_Port)) < 0) {
                    return -1;
                }
                int data_listen;
                if ((data_listen = socket_create_tcp(conf.Data_Port)) < 0) {
                    return -1;
                }
                while(1) {
                    int ctrl_fd;
                    if ((ctrl_fd = accept(ctrl_listen, NULL, NULL)) < 0) {
                        continue;
                    }
                    fflush(stdout);
                    int ctrl_id = 0;
                    while(recv(ctrl_fd, (void *)&ctrl_id, sizeof(ctrl_id), 0) > 0) {
                        for (int i = 0; i < N; i++) {
                            if (ctrl_id == SC[i].ID) {
                                int flag = 0;
                                FILE *fp, *zp;
                                if ((zp = fopen(SC[i].Back, "r")) == NULL) {
                                    if ((fp = fopen(SC[i].Log, "r")) == NULL) {
                                        flag = 400 + i;
                                        send(ctrl_fd, (const char *)&flag, sizeof(flag), 0);
                                    } else {
                                        flag = 200 + i;
                                        send(ctrl_fd, (const char *)&flag, sizeof(flag), 0);
                                        int data_fd;
                                        if ((data_fd = accept(data_listen, NULL, NULL)) < 0) {
                                            continue;
                                        }
                                        if (send_file(data_fd, SC[i].Log) < 0) {
                                            continue;
                                        }
                                        if (remove(SC[i].Log) < 0) {
                                            continue;
                                        }
                                        close(data_fd);
                                    }
                                } else {
                                    if (file_unback(SC[i].Back, SC[i].BLog) < 0) {
                                        continue;
                                    }
                                    if (remove(SC[i].Back) < 0) {
                                        continue;
                                    }
                                    flag = 200 + i;
                                    send(ctrl_fd, (const void *)&flag, sizeof(flag), 0);
                                    int data_fd;
                                    if ((data_fd = accept(data_listen, NULL, NULL)) < 0) {
                                        continue;
                                    }
                                    if (send_file(data_fd, SC[i].BLog) < 0) {
                                        continue;
                                    }
                                    if (remove(SC[i].BLog) < 0) {
                                        continue;
                                    }
                                }
                            }
                        }
                    }
                    close(ctrl_fd);
                    pthread_mutex_lock(&cond->mutex);
                    cond->cnt = 0;
                    pthread_mutex_unlock(&cond->mutex);
                }
                close(ctrl_listen);
            }
        }
    }
    return 0;
}