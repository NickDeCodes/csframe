/*************************************************************************
	> File Name: socket.h
	> Author: 
	> Mail: 
	> Created Time: 六 10/ 3 14:26:04 2020
 ************************************************************************/

#ifndef _SOCKET_H
#define _SOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>

/* 创建一个socket 类型TCP */
int socket_create_tcp(int port);
/* 进行 socket 连接 类型TCP */
int socket_connect_tcp(int port, char *host, int sec, double usec);
/* 创建一个socket 类型UDP */
int socket_create_udp(int port);
/* 进行 socket 连接 类型UDP */
int socket_connect_udp(int port, char *host, char *buff);

#endif
