#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;

void third_thread(void *arg) {
    for (int i = 0; i < 5; ++i) {
        printf("Thread 3: %d\n", i);
        for (volatile int j = 0; j < 80000000; ++j); // 耗时
    }

    // Message msg;
    // msg.senderPid = programManager.running->pid;
    // msg.size = 26;
    // const char* data = "send a message to thread 1";

    // for(int i = 0; i < msg.size; i++)
    //     msg.data[i] = data[i];
    // msg.data[msg.size] = '\0';

    // ListItem* item = programManager.allPrograms.at(0);
    // PCB *receive_thread = ListItem2PCB(item, tagInGeneralList);
    // programManager.sendMessage(receive_thread->messageQueue, &msg);

    // printf("Message sent to thread 0!\n");
}

void second_thread(void *arg) {
    for (int i = 0; i < 5; ++i) {
        printf("Thread 2: %d\n", i);
        for (volatile int j = 0; j < 80000000; ++j); // 耗时

        if(i == 3)  programManager.executeThread(third_thread, nullptr, "third thread", 3);
    }
}

void first_thread(void *arg)
{
    for (int i = 0; i < 5; ++i) {
        printf("Thread 1: %d\n", i);
        for (volatile int j = 0; j < 80000000; ++j); // 耗时

        if(i == 2)  programManager.executeThread(second_thread, nullptr, "second thread", 2);
    }

    
    // Message recvMsg;
    // programManager.receiveMessage(programManager.running->messageQueue, &recvMsg);
    // printf("Thread %d received message from thread %d: \"%s\"\n", 
    //     programManager.running->pid, recvMsg.senderPid, recvMsg.data);

    program_exit();
}

extern "C" void setup_kernel()
{

    // 中断管理器
    interruptManager.initialize();
    interruptManager.enableTimeInterrupt();
    interruptManager.setTimeInterrupt((void *)asm_time_interrupt_handler);

    // 输出管理器
    stdio.initialize();

    // 进程/线程管理器
    programManager.initialize();

    // 创建第一个线程
    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }

    // second thread
    // pid = programManager.executeThread(second_thread, nullptr, "second thread", 1);

    // // third thread
    // pid = programManager.executeThread(third_thread, nullptr, "third thread", 1);


    ListItem *item = programManager.readyPrograms.front();
    PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
    firstThread->status = RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);

    asm_halt();
}
