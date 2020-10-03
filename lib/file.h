/*************************************************************************
	> File Name: file.h
	> Author: 
	> Mail: 
	> Created Time: 六 10/ 3 20:25:06 2020
 ************************************************************************/

#ifndef _FILE_H
#define _FILE_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <zlib.h>

/* 文件压缩函数 */
int file_backup(char *file, char *backfile);
/* 文件解压函数 */
int file_unback(char *backfile, char *file);
/* 获取文件大小 */
int file_size(char *file);
/* 发送文件 */
int send_file(int fd, char *file);
/*字符判断函数*/
int strtok_func(char *buff, char *option, char *flag);
/*提取字段函数*/
char *rss_func(char *buff, int ind, char *flag);
#endif
