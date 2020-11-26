/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: 六 10/ 3 14:10:41 2020
 ************************************************************************/

#include "./lib/linklist.h"
#include "./lib/epoll.h"
#include "./lib/condition.h"
#include "./lib/threadpool.h"
#include "./lib/file.h"

struct shared {
    int cnt; // 检测次数
    int time; // 心跳次数
    pthread_mutex_t mutex; // 互斥变量
    pthread_cond_t ready; // 条件变量
};
struct shared *cond; // 共享内存状态结构体

char *Req_Id[N] = {"Req_Id0", "Req_Id1", "Req_Id2", "Req_Id3", "Req_Id4", "Req_Id5"};
char *Yes_Id[N] = {"Yes_Id0", "Yes_Id1", "Yes_Id2", "Yes_Id3", "Yes_Id4", "Yes_Id5"};
char *filename[N] = {"filename0", "filename1", "filename2", "filename3", "filename4", "filename5"};

threadpool_t pool;
LinkList LinkList = NULL;

struct timers print_timer; // 输出间隔时间
struct timers heart_timer; // 心跳间隔时间
struct timers ctrldata_timer; // 数据传输间隔时间

int login_listen
void user_ip_init(char *from, char *to) {
    struct in_addr from_addr;
    struct in_addr to_addr;
    inet_aton(from, &from_addr);
    inet_aton(to, &to_addr);
    unsigned int from_t = ntohl(from_addr.s_addr);
    unsigned int to_t = ntohl(to_addr.s_addr);
    for (unsigned int i = from_t; i <= to_t; i++) {
        struct in_addr q;
        q.s_addr = ntohl(i);
        struct sockaddr_in user_addr;
        user_addr.sin_addr = q;
        Node *node = node_create(user_addr);
        LinkList = insert(LinkList, node);
    }
    return ;
}

/* 遍历输出 */
void *print_pthread(void *argv) {
    LinkList p;
    p = LinkList;
    output(p);
    return NULL;
}

/* 用户登陆 */
void *login_pthread(void *argv) {
    do_epoll(LinkList, login_listen);
    return NULL;
}

/* 心跳检测 */
void *heart_pthread(void *argv) {
    LinkList head = (LinkList)argv;
    LinkList p, q;
    
    struct timeval tm; // 定义时间结构体
    tm.tv_sec = 0;
    tm.tv_usec = 500000;
    
    fd_set wset;
    FD_ZERO(&wset);
    int maxfd = 0;
    p = head;
    while (p != NULL && p->next != NULL) {
        int heart_fd = 0;
        struct sockaddr_in heart_addr;
        // 创建套接字
        if ((heart_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            return NULL;
        }
        // 设置套接字为非阻塞态
        int opt = 1;
        if (ioctl(heart_fd, FIONBIO, &opt) < 0) {
            return NULL;
        }
            
        // 设置服务器
        memset(&heart_addr, 0, sizeof(heart_addr)); // 数据初始化清零
        heart_addr.sin_family = AF_INET; // 设置协议族
        heart_addr.sin_port = htons(conf.Heart_Port); // 端口
        heart_addr.sin_addr.s_addr = inet_addr(inet_ntoa(p->next->addr.sin_addr)); // IP地址
        // 连接
        connect(heart_fd, (struct sockaddr *)&heart_addr, sizeof(heart_addr));
        FD_SET(heart_fd, &wset);
        // 判断最大
        if(maxfd < heart_fd) {
            maxfd = heart_fd;
        }
        // 放入集合
        p->next->fd = heart_fd;
        p = p->next;
    }
    usleep(500000);
    int nfd = select(maxfd + 1, NULL, &wset, NULL, &tm);
    if (nfd > 0) {
        q = head;
        while (q != NULL && q->next != NULL) {
            if (FD_ISSET(q->next->fd, &wset)) {
                int error = -1;
                int len = sizeof(int);
                getsockopt(q->next->fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len); //套接字，层次，获取错误状态并清除，缓冲区，长度值
                if (error == 0) {
                    FD_CLR(q->next->fd, &wset);
                    close(q->next->fd);
                    q = q->next;
                } else {
                    FD_CLR(q->next->fd, &wset);
                    close(q->next->fd);
                    q->next->fd = -1;
                    LinkList = delete_node(LinkList, q->next->addr);
                }
            } else {
                FD_CLR(q->next->fd, &wset);
                close(q->next->fd);
                LinkList = delete_node(LinkList, q->next->addr);
            }
        }
    } else {
        //DBG("heart_pthread->\033[31mselect失败: %s\033[0m\n", strerror(errno));
    }
    return NULL;
}

/* 心跳—数据检测 */
void *ctrl_data_pthread(void *argv) {
    struct sockaddr_in ctrl_data_addr = *(struct sockaddr_in *)argv;
    int ctrl_fd;
    if ((ctrl_fd = socket_connect_tcp(conf.Ctrl_Port, inet_ntoa(ctrl_data_addr.sin_addr), 0, 0.5)) < 0) {
        return NULL;
    }
    for (int i = 0; i < N; i++) {
        if (send(ctrl_fd, (const void*)&task_id[i].Req_Id, sizeof(task_id[i].Req_Id), 0) < 0) {
            return NULL;
        }
        int rec_id = 0;
        if (recv(ctrl_fd, (void *)&rec_id, sizeof(rec_id), 0) < 0) {
            continue;
        }
        if (rec_id == task_id[i].Yes_Id) {
            int data_fd;
            char buffer[4 * MAX_SIZE] = {0};
            conf.Dir = get_conf_value(config, "Dir");
            if ((data_fd = socket_connect_tcp(conf.Data_Port, inet_ntoa(ctrl_data_addr.sin_addr), 0, 0.5)) < 0) {
                return NULL;
            }
            while (recv(data_fd, buffer, sizeof(buffer), 0) > 0) {
                strcat(conf.Dir, inet_ntoa(ctrl_data_addr.sin_addr));
                if (NULL == opendir(conf.Dir)) {
                    mkdir(conf.Dir, 0777);
                }
                strcat(conf.Dir, task_id[i].filename);
                FILE *log_file = fopen(conf.Dir, "a+");
                flock(log_file->_fileno, LOCK_EX);
                fwrite(buffer, sizeof(char), strlen(buffer), log_file);
                fclose(log_file);
                conf.Dir = NULL;
                memset(buffer, 0, sizeof(buffer));
                close(data_fd);
            }
        }
    }
    close(ctrl_fd);
    return NULL;
}

/* 警告信息 */
/*void *warn_pthread(void *argv) {
    int warn_fd;
    if ((warn_fd = socket_create_udp(conf.Warn_Port)) < 0) {
        return NULL;
    }
    struct sockaddr_in warn_addr;
    int len = sizeof(struct sockaddr_in), warn_udp;
    char buff[4 * SIZE] = {0};
    char test[4 * SIZE] = {0};
    char ttime[4 * SIZE] = {0};
    while (1) {
		warn_udp = recvfrom(warn_fd, buff, sizeof(buff), 0, (void *)&warn_addr, (socklen_t *)&len);
        printf("%s\n", buff);
        strcpy(test, buff);
        int types = 0;
        if (strtok_func(test, "note\n", " ")) {
            DBG("\033[31mnote:\033[0m %s\n", buff);
            printf("%d\n", atoi(test));
            printf("\033[31m%s\n\033[0m", inet_ntoa(warn_addr.sin_addr));
            time_t dm = time(NULL);
            struct tm *dmt = localtime(&dm);
            sprintf(ttime, "%d-%02d-%02d %02d:%02d:%02d", dmt->tm_year + 1900, dmt->tm_mon + 1, dmt->tm_mday, dmt->tm_hour, dmt->tm_min, dmt->tm_sec);
            printf("%s\n", ttime);
        } else {

        }
		printf("\033[31m%s\033[0m\n", buff);
    }
    return NULL;
}*/

/* 定时器管理者 */
void timers_manager() {
    print_timer.interval--;
    if (print_timer.interval == 0) {
        print_timer.interval = SET.print_time;
        print_timer.handler();
    }
    heart_timer.interval--;
    if (heart_timer.interval == 0) {
        heart_timer.interval = SET.heart_time;
        heart_timer.handler();
    }
    ctrldata_timer.interval--;
    if (ctrldata_timer.interval == 0) {
        ctrldata_timer.interval = SET.ctrldata_time;
        ctrldata_timer.handler();
    }
}

/*执行函数*/
void print() {
    threadpool_add_task(&pool, print_pthread, 0);
}

void heart() {
    threadpool_add_task(&pool, heart_pthread, LinkList);
}

void ctrldata() {
    LinkList p;
    p = LinkList;
    while (p != NULL && p->next != NULL) {
        threadpool_add_task(&pool, ctrl_data_pthread, &p->next->addr);
        p = p->next;
    }
}

/* 定时器初始化 */
void timers_init(int sec)  {
    struct itimerval base_tm;
    base_tm.it_value.tv_sec = sec;
    base_tm.it_value.tv_usec = 0;
    base_tm.it_interval = base_tm.it_value;
    setitimer(ITIMER_REAL, &base_tm, NULL);
    
    struct sigaction task;
    task.sa_handler = timers_manager;
    task.sa_flags = 0;
    sigemptyset(&task.sa_mask);
    sigaction(SIGALRM, &task, NULL);
    
    print_timer.interval = SET.print_time * sec;
    print_timer.handler = print;
    
    heart_timer.interval = SET.heart_time * sec;
    heart_timer.handler = heart;
    
    ctrldata_timer.interval = SET.ctrldata_time * sec;
    ctrldata_timer.handler = ctrldata;
}

int main() {
    /*获取配置参数*/
    for (int i = 0; i < N; i++) {
        task_id[i].Req_Id = atoi(get_conf_value(config, Req_Id[i]));
        task_id[i].Yes_Id = atoi(get_conf_value(config, Yes_Id[i]));
        task_id[i].filename = get_conf_value(config, filename[i]);
    }
    conf.Master_Port = atoi(get_conf_value(config, "Master_Port"));
    conf.Heart_Port = atoi(get_conf_value(config, "Heart_Port"));
    conf.Ctrl_Port = atoi(get_conf_value(config, "Ctrl_Port"));
    conf.Data_Port = atoi(get_conf_value(config, "Data_Port"));
    conf.Warn_Port = atoi(get_conf_value(config, "Warn_Port"));
    conf.From = get_conf_value(config, "From");
    conf.To = get_conf_value(config, "To");
    conf.Sys_Log = get_conf_value(config, "Sys_Log");
    conf.INS = atoi(get_conf_value(config, "INS"));
    
    DB.database = get_conf_value(config, "database");
    DB.host = get_conf_value(config, "host");
    DB.user = get_conf_value(config, "user");
    DB.passwd = get_conf_value(config, "passwd");

    SET.print_time = atoi(get_conf_value(config, "print_time"));
    SET.heart_time = atoi(get_conf_value(config, "heart_time"));
    SET.ctrldata_time = atoi(get_conf_value(config, "ctrldata_time"));

    /* 初始化头节点 */
    struct sockaddr_in init_addr;
    memset(&init_addr, 0, sizeof(struct sockaddr_in));
    LinkList = head_node_init(init_addr, conf.Heart_Port);

	/* 用户IP初始化 */
    user_ip_init(conf.From, conf.To);
    if ((login_listen = socket_create_tcp(conf.Master_Port)) < 0) {
    }

    /* 线程池初始化 */
    threadpool_init(&pool, 2, conf.INS);

    /* 添加任务 */
    threadpool_add_task(&pool, login_pthread, 0);
    //threadpool_add_task(&pool, warn_pthread, 0);
    
    /* 定时队列 */
    timers_init(1);
    while(1);
    // 摧毁线程池
    threadpool_destroy(&pool);
    return 0;
}
