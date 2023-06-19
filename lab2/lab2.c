/**
 * lab2.c
 * 实现生产者消费者问题，创建3个消费者线程，4个生产者线程
 * 生产者向缓冲区中放入产品，消费者从缓冲区中取出产品
 * 生产者和消费者任务都具有相同的优先级
*/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#define BUFFER_SIZE 5
#define PRODUCER_NUM 4
#define CONSUMER_NUM 3

char buffer[BUFFER_SIZE][1000]; // 缓冲区
pthread_mutex_t mutex; // 互斥信号量
sem_t full, empty; // 同步信号量

int in = 0, out = 0; // 缓冲区的读写指针
int producer_id[PRODUCER_NUM] = {0, 1, 2, 3};
int consumer_id[CONSUMER_NUM] = {0, 1, 2};

struct param
{
    int *id;
    FILE *fp;
};

void producer(void *arg)
{
    char data[1005];
    while (1) 
    {   
        struct param *p = (struct param *)arg;
        int *id = p->id;
        FILE *fp = p->fp;
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        memset(data, 0, sizeof(data));
        if (fscanf(fp, "%s", data) == EOF) // 读取到文件结束
        {
            fseek(fp, 0, SEEK_SET);
            fscanf(fp, "%s", data);
        }
        printf("producer %d put %s into buffer[%d]\n", *id, data, in);
        memset(buffer[in], 0, sizeof(buffer[in]));
        strcpy(buffer[in], data);
        in = (in + 1) % BUFFER_SIZE; // in指针后移（循环）
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
        sleep(1);
    }
}

void consumer(void *arg)
{
    while (1)
    {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        printf("consumer %d get %s from buffer[%d]\n", *(int *)arg, buffer[out], out);
        out = (out + 1) % BUFFER_SIZE; // out指针后移（循环）
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
        sleep(1);
    }
}

int main()
{
    // 打开文件
    FILE *fp = fopen("lab2_data.txt", "r");
    // 初始化互斥锁与信号量
    pthread_mutex_init(&mutex, NULL);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);
    // 初始化缓冲区
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        memset(buffer[i], 0, sizeof(buffer[i]));
    }
    // 创建生产者线程
    pthread_t producers[PRODUCER_NUM];
    for (int i = 0; i < PRODUCER_NUM; i++)
    {
        struct param p = {&producer_id[i], fp};
        pthread_create(&producers[i], NULL, (void *)producer, (void *)&p);
    }
    // 创建消费者线程
    pthread_t consumers[CONSUMER_NUM];
    for (int i = 0; i < CONSUMER_NUM; i++)
    {
        pthread_create(&consumers[i], NULL, (void *)consumer, (void *)&consumer_id[i]);
    }
    // 等待生产者线程结束
    for (int i = 0; i < PRODUCER_NUM; i++)
    {
        pthread_join(producers[i], NULL);
    }
    // 等待消费者线程结束
    for (int i = 0; i < CONSUMER_NUM; i++)
    {
        pthread_join(consumers[i], NULL);
    }
    // 销毁信号量
    pthread_mutex_destroy(&mutex);
    sem_destroy(&full);
    sem_destroy(&empty);
    return 0;
}
