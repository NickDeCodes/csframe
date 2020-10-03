/*************************************************************************
	> File Name: file.c
	> Author: 
	> Mail: 
	> Created Time: 六 10/ 3 20:24:57 2020
 ************************************************************************/

#include "file.h"

/* 文件压缩函数 */
int file_backup(char *file, char *backfile) {
    FILE *ffile;
    unsigned long int flen, clen;
    unsigned char *fbuf = NULL;
    unsigned char *cbuf = NULL;
    
    // 通过命令行参数将file文件的数据压缩后存放到backfile文件中
    if ((ffile = fopen(file, "rb")) == NULL) {
        return -1;
    }
    // 装载源文件数据到缓冲区
    fseek(ffile, 0, SEEK_END); // 跳到文件末尾
    flen = ftell(ffile); // 获取文件长度
    fseek(ffile, 0, SEEK_SET);
    if ((fbuf = (unsigned char *)malloc(sizeof(unsigned char) * flen)) == NULL) {
        fclose(ffile);
        return -1;
    }
    fread(fbuf, sizeof(unsigned char), flen, ffile);
    // 压缩数据
    clen = compressBound(flen);
    if ((cbuf = (unsigned char *)malloc(sizeof(unsigned char) * clen)) == NULL) {
        fclose(ffile);
        return -1;
    }
    if (compress(cbuf, &clen, fbuf, flen) != Z_OK) {
        return -1;
    }
    fclose(ffile);
    if ((ffile = fopen(backfile, "a+")) == NULL) {
        return -1;
    }
    // 保存压缩后的数据到目标文件
    fwrite(&flen, sizeof(unsigned long int), 1, ffile); // 写入源文件长度
    fwrite(&clen, sizeof(unsigned long int), 1, ffile); // 写入目标数据长度
    fwrite(cbuf, sizeof(unsigned char), clen, ffile);
    fclose(ffile);
    free(fbuf);
    free(cbuf);
    return 1;
}

/* 文件解压函数 */
int file_unback(char *backfile, char *file) {
    // 解压函数
    FILE *ffile;
    unsigned long int flen, ulen;
    unsigned char *fbuf = NULL;
    unsigned char *ubuf = NULL;
    // 通过命令行参数将文件的数据解压缩后存放到文件中
    if ((ffile = fopen(backfile, "rb")) == NULL) {
        return -1;
    }
    // 装载源文件数据到缓冲区
    fread(&ulen, sizeof(unsigned long int), 1, ffile); // 获取缓冲区大小
    fread(&flen, sizeof(unsigned long int), 1, ffile); // 获取数据流大小
    
    if ((fbuf = (unsigned char*)malloc(sizeof(unsigned char) * flen)) == NULL) {
        fclose(ffile);
        return -1;
    }
    fread(fbuf, sizeof(unsigned char), flen, ffile);
    // 解压缩数据
    if ((ubuf = (unsigned char*)malloc(sizeof(unsigned char) * ulen)) == NULL) {
        fclose(ffile);
        return -1;
    }
    if (uncompress(ubuf, &ulen, fbuf, flen) != Z_OK) {
        return -1;
    }
    fclose(ffile);
    if ((ffile = fopen(file, "a+")) == NULL) {
        return -1;
    }
    // 保存解压缩后的数据到目标文件
    fwrite(ubuf, sizeof(unsigned char), ulen, ffile);
    fclose(ffile);
    free(fbuf);
    free(ubuf);
    return 1;
}

/* 获取文件大小 */
int file_size(char *file) {
    FILE *fp;
    if ((fp = fopen(file, "r")) == NULL) {
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    return len;
}

/* 发送文件 */
int send_file(int fd, char *file) {
    FILE *fp;
    if ((fp = fopen(file, "r")) == NULL) {
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *data = (char *)malloc(len + 1);
    fread(data, len, 1, fp);
    printf("%s\n", data);
    if (send(fd, data, strlen(data), 0) < 0) {
    }
    fclose(fp);
    memset(data, 0, sizeof(len + 1));
    return 1;
}

/*字符判断函数*/
int strtok_func(char *buff, char *option, char *flag) {
    char *p = strtok(buff, flag);
    p = strtok(NULL, flag);
    while (p) {
        if (strcmp(p, option) == 0) {
            //DBG("OK\n");
            return 1;
        }
        p = strtok(NULL, flag);
    }
    //DBG("NO\n");
    return 0;
}
/*提取字段函数*/
char *rss_func(char *buff, int ind, char *flag) {
    char *p = strtok(buff, flag);
    ind--;
    if (ind == 0) {
        //DBG("%s\n", p);
        return p;
    }
    p = strtok(NULL, flag);
    return 0;
}