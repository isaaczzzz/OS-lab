/*
利用POSIX API（文件操作也可以使用ANSI C标准I/O库）编程实现cp –r命令，
支持将源路径（目录）中的所有文件和子目录，以及子目录中的所有内容，全部拷贝到目标路径（目录）中。
使用多线程加快拷贝速度。
*/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>

struct file_paras
{
    char *spath;
    char *dpath;
};

// 实现文件到文件的拷贝
int files = 0;
struct file_paras *fileQueue[1024];
void appendFileQueue(struct file_paras *fp)
{
    fileQueue[files++] = fp;
}
void copyfile(void *arg)
{
    struct file_paras *fp = (struct file_paras *)arg;
    char *spath = fp->spath;
    char *dpath = fp->dpath;
    int sfd, dfd;
    if ((sfd = open(spath, O_RDONLY)) == -1) // 只读
    {
        printf("源文件不存在\n");
        exit(-1);
    }
    // 目标文件不存在则创建，存在则覆盖
    if ((dfd = open(dpath, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) // 只写，不存在则创建
    {
        printf("目标文件创建失败!\n");
        exit(-1);
    }
    char buf[1024]; // 缓冲区
    int n;
    while ((n = read(sfd, buf, 1024)) > 0) // 读取源文件,到达文件尾部时返回0
    {
        if (n == -1 || (write(dfd, buf, n)) == -1) // 写入目标文件
        {
            printf("Error copying file from %s to %s\n", spath, dpath);
            exit(-1);
        }
    }
    printf("%s 拷贝到 %s\n", spath, dpath);
    close(sfd);
    close(dfd);
    return;
}
// 实现文件属性的判断
int isdir(char *path)
{
    struct stat buf;
    if (stat(path, &buf) == -1)
    {
        printf("获取文件属性错误\n");
        exit(-1);
    }
    return S_ISDIR(buf.st_mode);
}
// 目录的遍历及复制
int walkdir(char *spath, char *dpath)
{
    DIR *sdir;
    struct dirent *sdp;
    if ((sdir = opendir(spath)) == NULL)
    {
        printf("源目录不存在!\n");
        exit(-1);
    }
    while ((sdp = readdir(sdir)) != NULL)
    {
        if (strcmp(sdp->d_name, ".") == 0 || strcmp(sdp->d_name, "..") == 0) // 忽略.和..
            continue;
        char sfile[1024], dfile[1024];
        // 拼接源文件和目标文件的路径
        strcpy(sfile, spath);
        strcat(sfile, "/");
        strcat(sfile, sdp->d_name);
        strcpy(dfile, dpath);
        strcat(dfile, "/");
        strcat(dfile, sdp->d_name);
        if (isdir(sfile))
        {
            // 如果目录不存在则创建
            DIR *temp = opendir(dfile);
            if (temp != NULL)
                closedir(temp);
            else if (mkdir(dfile, 0777) == -1) // 创建目录
            {
                printf("创建目录 %s 失败\n", dfile);
                exit(-1);
            }
            if (walkdir(sfile, dfile) == -1) // 递归遍历子目录
                exit(-1);
        }
        else
        {
            // 如果是文件则直接拷贝
            struct file_paras *fp = (struct file_paras *)malloc(sizeof(struct file_paras));
            char *sp = (char *)malloc(sizeof(char) * 1024);
            char *dp = (char *)malloc(sizeof(char) * 1024);
            strcpy(sp, sfile);
            strcpy(dp, dfile);
            fp->spath = sp;
            fp->dpath = dp;
            appendFileQueue(fp);
        }
    }
    closedir(sdir);
    return 0;
}
// 主函数实现参数读取
int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        printf("用法: %s source_path destination_path\n", argv[0]);
        exit(-1);
    }
    // 计算拷贝时间
    struct timeval start, end;
    gettimeofday(&start, NULL);
    if ((walkdir(argv[1], argv[2])) != 0)
    {
        printf("拷贝失败!\n");
        exit(-1);
    }
    // 多线程拷贝
    pthread_t tid[files];
    for (int i = 0; i < files; i++)
    {
        pthread_create(&tid[i], NULL, (void *)copyfile, (void *)fileQueue[i]);
    }
    // 等待线程结束
    for (int i = 0; i < files; i++)
    {
        pthread_join(tid[i], NULL);
    }
    gettimeofday(&end, NULL);
    long timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("time = %ld us\n", timeuse);
    return 0;
}