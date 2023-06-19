/**
 * lab1.c
 * 实现哲学家就餐问题，要求不能出现死锁
*/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define N 5
#define LEFT (i + N - 1) % N
#define RIGHT (i + 1) % N

// 信号量
sem_t room;
sem_t chopsticks[N];
int phil_num[N] = {0, 1, 2, 3, 4};

void philosopher(void *arg)
{
    int i = *(int *)arg;
    while (1)
    {
        printf("Philosopher %d is thinking.\n", i);
        sleep(1);
        printf("Philosopher %d is hungry.\n", i);
        sem_wait(&room);
        sem_wait(&chopsticks[LEFT]);
        sem_wait(&chopsticks[RIGHT]);
        printf("Philosopher %d is eating.\n", i);
        sem_post(&room);
        sleep(1);
        sem_post(&chopsticks[LEFT]);
        sem_post(&chopsticks[RIGHT]);
    }
}

int main() {
    // 初始化信号量
    sem_init(&room, 0, N - 1);
    for (int i = 0; i < N; i++)
    {
        sem_init(&chopsticks[i], 0, 1);
    }
    // 创建线程
    pthread_t thread_id[N];
    for (int i = 0; i < N; i++)
    {
        pthread_create(&thread_id[i], NULL, (void *)philosopher, &phil_num[i]);
    }
    // 等待线程结束
    for (int i = 0; i < N; i++)
    {
        pthread_join(thread_id[i], NULL);
    }
    // 销毁信号量
    sem_destroy(&room);
    for (int i = 0; i < N; i++)
    {
        sem_destroy(&chopsticks[i]);
    }
    return 0;
}


