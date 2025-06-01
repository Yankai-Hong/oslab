#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "list.h"

struct Semaphore {
    int value;
    List waitQueue;
};

void initSemaphore(Semaphore* s, int initialValue);

#endif
