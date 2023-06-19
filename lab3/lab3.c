/**
* lab3.c
* 基于管道的进程间数据传输
* 父进程首先使用系统调用pipe()建立一个管道，然后使用系统调用fork() 创建子进程1
* 子进程1关闭管道读文件
* 子进程1通过文件I/O操作向管道写文件写一句话（向文件中写入字符串）：Child process 1 is sending a message!
* 然后子进程1调用exit（）结束运行
* 父进程再次使用系统调用fork() 创建子进程2
* 子进程2关闭管道读文件
* 子进程2通过文件I/O操作向管道写文件写一句话（向文件中写入字符串）：Child process 2 is sending a message!
* 然后子进程2调用exit（）结束运行
* 父进程关闭管道写文件
* 父进程通过文件I/O操作从管道读文件中读出来自于两个子进程的信息，通过printf语句打印输出在屏幕上
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MSG_SIZE 100

int main() {
    int fd[2]; // fd[0] 读数据, fd[1] 写数据
    pid_t pid1, pid2;

    // 创建管道
    if (pipe(fd) == -1) {
        printf("创建管道失败\n");
        exit(EXIT_FAILURE);
    }

    // 创建子进程1
    pid1 = fork();
    if (pid1 == -1) {
        printf("创建子进程失败\n");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        // 子进程1关闭管道读文件
        close(fd[0]);

        // 子进程1向管道写文件写一句话
        printf("子进程1正在写入管道\n");
        char message[MSG_SIZE] = "Child process 1 is sending a message!\n";
        write(fd[1], message, strlen(message) + 1);

        // 结束子进程1
        exit(EXIT_SUCCESS);
    }

    // 创建子进程2
    pid2 = fork();
    if (pid2 == -1) {
        printf("创建子进程失败\n");
        exit(EXIT_FAILURE);
    } else if (pid2 == 0) {
        // 子进程2关闭管道读文件
        close(fd[0]);

        // 子进程2向管道写文件写一句话
        printf("子进程2正在写入管道\n");
        char message[MSG_SIZE] = "Child process 2 is sending a message!\n";
        write(fd[1], message, strlen(message) + 1);

        // 结束子进程2
        exit(EXIT_SUCCESS);
    }

    // 父进程关闭管道写文件
    close(fd[1]);

    // 父进程从管道读文件中读出来自于两个子进程的信息
    char buf1[MSG_SIZE], buf2[MSG_SIZE];
    read(fd[0], buf1, MSG_SIZE);
    read(fd[0], buf2, MSG_SIZE);

    // 打印输出在屏幕上
    printf("父进程从管道读文件中读出来自于两个子进程的信息\n");
    printf("%s", buf1);
    printf("%s", buf2);

    // 结束父进程
    exit(EXIT_SUCCESS);
}