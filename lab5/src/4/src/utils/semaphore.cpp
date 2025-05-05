#include "semaphore.h"

void initSemaphore(Semaphore* s, int initialValue){
    s->value = initialValue;
    s->waitQueue.initialize();
}