#include "program.h"
#include "stdlib.h"
#include "interrupt.h"
#include "asm_utils.h"
#include "stdio.h"
#include "thread.h"
#include "os_modules.h"
#include "message.h"

const int PCB_SIZE = 4096;                   // PCB的大小，4KB。
char PCB_SET[PCB_SIZE * MAX_PROGRAM_AMOUNT]; // 存放PCB的数组，预留了MAX_PROGRAM_AMOUNT个PCB的大小空间。
bool PCB_SET_STATUS[MAX_PROGRAM_AMOUNT];     // PCB的分配状态，true表示已经分配，false表示未分配。

ProgramManager::ProgramManager()
{
    initialize();
}

void ProgramManager::initialize()
{
    allPrograms.initialize();
    readyPrograms.initialize();
    running = nullptr;

    for (int i = 0; i < MAX_PROGRAM_AMOUNT; ++i)
    {
        PCB_SET_STATUS[i] = false;
    }
}

int ProgramManager::executeThread(ThreadFunction function, void *parameter, const char *name, int priority)
{
    // 关中断，防止创建线程的过程被打断
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    // 分配一页作为PCB
    PCB *thread = allocatePCB();

    if (!thread)
        return -1;

    // 初始化分配的页
    memset(thread, 0, PCB_SIZE);

    for (int i = 0; i < MAX_PROGRAM_NAME && name[i]; ++i)
    {
        thread->name[i] = name[i];
    }

    thread->status = ProgramStatus::READY;
    thread->priority = priority;
    thread->ticks = priority * 10;
    thread->ticksPassedBy = 0;
    thread->pid = ((int)thread - (int)PCB_SET) / PCB_SIZE;

    // 线程栈
    thread->stack = (int *)((int)thread + PCB_SIZE);
    thread->stack -= 7;
    thread->stack[0] = 0;
    thread->stack[1] = 0;
    thread->stack[2] = 0;
    thread->stack[3] = 0;
    thread->stack[4] = (int)function;
    thread->stack[5] = (int)program_exit;
    thread->stack[6] = (int)parameter;

    allPrograms.push_back(&(thread->tagInAllList));
    readyPrograms.push_back(&(thread->tagInGeneralList));

    // initiaze Message queue
    initMessageQueue(thread->messageQueue);

    // 恢复中断
    interruptManager.setInterruptStatus(status);

    schedule_priority_first(thread);

    return thread->pid;
}

void ProgramManager::schedule()
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    if (readyPrograms.size() == 0)
    {
        interruptManager.setInterruptStatus(status);
        return;
    }

    if (running->status == ProgramStatus::RUNNING)
    {
        running->status = ProgramStatus::READY;
        running->ticks = running->priority;
        readyPrograms.push_back(&(running->tagInGeneralList));
    }
    else if (running->status == ProgramStatus::DEAD)
    {
        releasePCB(running);
    }

    ListItem *item = readyPrograms.front();
    PCB *next = ListItem2PCB(item, tagInGeneralList);
    PCB *cur = running;
    next->status = ProgramStatus::RUNNING;
    running = next;
    readyPrograms.pop_front();

    asm_switch_thread(cur, next);

    interruptManager.setInterruptStatus(status);
}

void ProgramManager::schedule_FCFS(){
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    if (readyPrograms.size() == 0)
    {
        interruptManager.setInterruptStatus(status);
        return;
    }

    if (running->status == ProgramStatus::DEAD)
    {
        releasePCB(running);
    }

    ListItem *item = readyPrograms.front();
    PCB *next = ListItem2PCB(item, tagInGeneralList);
    PCB *cur = running;
    next->status = ProgramStatus::RUNNING;
    running = next;
    readyPrograms.pop_front();

    asm_switch_thread(cur, next);

    interruptManager.setInterruptStatus(status);
}

void ProgramManager::schedule_priority_first(PCB* insert_PCB){
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    if (readyPrograms.size() == 0 && !insert_PCB)
    {
        interruptManager.setInterruptStatus(status);
        return;
    }


    ListItem* highest_item = nullptr;
    PCB* highest_pcb = nullptr;

    for(ListItem* item = readyPrograms.front(); item != nullptr; item = item->next){
        PCB* tmp = ListItem2PCB(item, tagInGeneralList);

        if(highest_item==nullptr || tmp->priority > highest_pcb->priority){
            highest_item = item;
            highest_pcb = tmp;
        }
    }

    if(!insert_PCB && running->status == ProgramStatus::DEAD){
        PCB* next = highest_pcb;
        PCB* cur = running;
        next->status = ProgramStatus::RUNNING;
        running = next;

        releasePCB(cur);
        readyPrograms.erase(highest_item);

        asm_switch_thread(cur, next);

        interruptManager.setInterruptStatus(status);

        return;
    }

    //只有当running为nullptr，刚插入线程的优先级大于running的优先级，running->status为DEAD的时候才执行调度
    if(!running || insert_PCB->priority > running->priority || running->status == ProgramStatus::DEAD){
        if(running != nullptr && running->status == ProgramStatus::RUNNING){
            running->status = ProgramStatus::READY;
            readyPrograms.push_back(&programManager.running->tagInGeneralList);
        }

        PCB *next = insert_PCB;
        PCB *cur = running;
        next->status = ProgramStatus::RUNNING;
        running = next;
        readyPrograms.erase(&insert_PCB->tagInGeneralList);
    
        asm_switch_thread(cur, next);
    
        interruptManager.setInterruptStatus(status);        
    }
    

}



void program_exit()
{
    PCB *thread = programManager.running;
    thread->status = ProgramStatus::DEAD;

    programManager.schedule_priority_first();
    asm_halt();  // 防止 DEAD 线程“死而复生”

}

PCB *ProgramManager::allocatePCB()
{
    for (int i = 0; i < MAX_PROGRAM_AMOUNT; ++i)
    {
        if (!PCB_SET_STATUS[i])
        {
            PCB_SET_STATUS[i] = true;
            return (PCB *)((int)PCB_SET + PCB_SIZE * i);
        }
    }

    return nullptr;
}

void ProgramManager::releasePCB(PCB *program)
{
    int index = ((int)program - (int)PCB_SET) / PCB_SIZE;
    PCB_SET_STATUS[index] = false;
}

void ProgramManager::wait(Semaphore* S) {
    interruptManager.disableInterrupt(); // 原子操作关键，防止竞态
    S->value--;
    if (S->value < 0) {
        // 将当前线程加入等待队列
        PCB* current = programManager.running;
        S->waitQueue.push_back(&current->tagInGeneralList);
        current->status = BLOCKED;

        programManager.schedule_FCFS();  // 进行线程切换
    }
    interruptManager.enableInterrupt();
}

void ProgramManager::signal(Semaphore* S) {
    interruptManager.disableInterrupt();
    S->value++;
    if (S->value <= 0) {
        // 唤醒等待队列中的一个线程
        ListItem* item = S->waitQueue.front();
        S->waitQueue.pop_front();
        PCB* thread = ListItem2PCB(item, tagInGeneralList);
        thread->status = READY;
        programManager.readyPrograms.push_back(&thread->tagInGeneralList);
    }
    interruptManager.enableInterrupt();
}

void ProgramManager::sendMessage(MessageQueue* mq, Message* msg) {
    wait(&mq->spaces);       // 等待空位
    wait(&mq->mutex);        // 加锁互斥访问

    mq->messages[mq->tail] = *msg;
    mq->tail = (mq->tail + 1) % QUEUE_SIZE;
    mq->count++;

    signal(&mq->mutex);      // 解锁
    signal(&mq->items);      // 增加可读消息
}

void ProgramManager::receiveMessage(MessageQueue* mq, Message* outMsg) {
    wait(&mq->items);        // 等待消息到达
    wait(&mq->mutex);        // 加锁互斥访问

    *outMsg = mq->messages[mq->head];
    mq->head = (mq->head + 1) % QUEUE_SIZE;
    mq->count--;

    signal(&mq->mutex);      // 解锁
    signal(&mq->spaces);     // 增加空位
}


