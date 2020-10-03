/*************************************************************************
	> File Name: conf.h
	> Author: 
	> Mail: 
	> Created Time: 六 10/ 3 14:28:56 2020
 ************************************************************************/

#ifndef _CONF_H
#define _CONF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 配置方式 */
typedef enum {
	CONF_EQUAL, // name= value
	CONF_COLON, // name: value
	CONF_DIRECT, // name value
	CONF_NOT_SET = -1
} ConfType;

static struct {
	const char *name;
	const char *flag;
	ConfType val;
} conf_types[] = {
	{ "equal", "=",	CONF_EQUAL },
	{ "colon", ":",	CONF_COLON },
	{ "direct", " ", CONF_DIRECT },
	{ NULL,	NULL, CONF_NOT_SET }
};

static ConfType conf_type = CONF_EQUAL;

/* 配置文件类型初始化 */
void conf_type_init(ConfType type);
/* 检查类型是否合法 */
int conf_type_check(ConfType type);
/* 返回当前类型值 */
ConfType conf_type_number();
/* 返回当前类型名 */
const char *conf_type_name(ConfType type);
/* 返回类型标记 */
const char *conf_type_flag(ConfType type);
/* 获取整型数据 */
int get_conf_int(const char *path, const char *name);
/* 获取字符串数据 */
const char *get_conf_str(const char *path, const char *name);

#endif
