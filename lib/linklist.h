/*************************************************************************
	> File Name: linklist.h
	> Author: 
	> Mail: 
	> Created Time: 六 10/ 3 14:24:40 2020
 ************************************************************************/

#ifndef _LINKLIST_H
#define _LINKLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#ifdef _DEBUG
#define DBG(fmt, args...) printf(fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

/*创建链表节点*/
typedef struct Node{
    int fd;
    struct sockaddr_in addr;
    struct Node *next;
}Node, *LinkList;

/*插入节点*/
LinkList insert(LinkList head, Node *node);
/*删除节点*/
LinkList delete_node(LinkList head, struct sockaddr_in addr);
/*释放链表*/
void clear(LinkList head);
/*输出链表*/
void output(LinkList head);
/*汇总数据*/
void client_ip(LinkList head, char *ips);
/*寻找节点*/
int look_ip(LinkList head, struct sockaddr_in addr);
/*核查节点*/
int check_ip(LinkList head, struct sockaddr_in addr);
/*链表反转*/
LinkList reverse(LinkList head);
/*创建节点*/
Node *node_create(struct sockaddr_in addr);
/*初始化头*/
Node *head_node_init(struct sockaddr_in addr, int port);

#endif