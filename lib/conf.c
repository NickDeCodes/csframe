/*************************************************************************
	> File Name: conf.c
	> Author: 
	> Mail: 
	> Created Time: 六 10/ 3 14:28:22 2020
 ************************************************************************/

#include "conf.h"

/* 配置文件类型初始化 */
void conf_type_init(ConfType type) {
	if (conf_type_check(type) < 0) {
		return ;
	}
	conf_type = type;
	return ;
}

/* 检查类型是否合法 */
int conf_type_check(ConfType type) {
	switch (type) {
	case CONF_EQUAL:
	case CONF_COLON:
	case CONF_DIRECT:
		conf_type = type;
		return 0;
	default:
		return -1;
	}
}

/* 返回当前类型值 */
ConfType conf_type_number() {
	return conf_type;
}

/* 返回当前类型名 */
const char *conf_type_name(ConfType type) {
	int i;

	for (i = 0; conf_types[i].name != NULL; i++)
		if (conf_types[i].val == type)
			return conf_types[i].name;
	return NULL;
}

/* 返回类型标记 */
const char *conf_type_flag(ConfType type) {
	int i;

	for (i = 0; conf_types[i].name != NULL; i++)
		if (conf_types[i].val == type)
			return conf_types[i].flag;
	return NULL;
}

/* 获取整型数据 */
int get_conf_int(const char *path, const char *name) {
	FILE *fd = NULL;
	char *line = NULL;
	char *substr = NULL;
	char *shift = NULL;
	ssize_t read = 0;
	size_t len = 0;
	int value;
	int tmp = 0;

	fd = fopen(path, "r");
	if (fd == NULL) {
		return -1;
	}
	while ((read = getline(&line, &len, fd)) != 1) {
        fflush(stdout);
        substr = strstr(line, name);
        if (substr == NULL) {
            continue;
        } else {
            tmp = strlen(name);
            if (line[tmp] == (char)conf_type_flag(conf_type)) {
                strncpy(shift, &line[tmp + 1], (int)read - tmp - 1);
                tmp = strlen(shift);
                *(shift + tmp - 1) = '\0';
				value = (int)shift;
                return value;
            } else {
                continue;
            }
        }
    }
	return -1;
}

/* 获取字符串数据 */
const char *get_conf_str(const char *path, const char *name) {
    FILE *fp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    char *value = (char *)calloc(sizeof(char), 100);

    fp = fopen(path, "r");
    if (fp == NULL) {
        return NULL;
    }
    while ((read = getline(&line, &len, fp)) > 0) {
        char *ptr = strstr(line, name);
        if (ptr == NULL) continue;
        ptr += strlen(name);
        if (*ptr != (char)conf_type_flag(conf_type)) continue;
        strncpy(value, (ptr+1), strlen(ptr+2));
        int tempvalue = strlen(value);
        value[tempvalue] = '\0';
    }
    fclose(fp);
    return value;
}


