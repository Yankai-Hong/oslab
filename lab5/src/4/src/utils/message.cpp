#include "message.h"

void initMessageQueue(MessageQueue* mq) {
    mq->head = mq->tail = mq->count = 0;
    initSemaphore(&mq->mutex, 1);
    initSemaphore(&mq->items, 0);
    initSemaphore(&mq->spaces, QUEUE_SIZE);
}
