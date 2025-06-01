#ifndef MESSAGE_H
#define MESSAGE_H

#include "semaphore.h"
#define QUEUE_SIZE 128  //length of each queue

struct Message {
    int senderPid;
    int size;
    char data[128]; // 可根据需要调整
};

struct MessageQueue {
    Message messages[QUEUE_SIZE];
    int head;
    int tail;
    int count;
    Semaphore mutex;     // 互斥访问消息队列
    Semaphore items;     // 表示当前有多少条消息可读
    Semaphore spaces;    // 表示当前有多少空位可写
};


void initMessageQueue(MessageQueue* mq);

#endif