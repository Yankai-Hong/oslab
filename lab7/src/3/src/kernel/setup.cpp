#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"
#include "memory.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;
// 内存管理器
MemoryManager memoryManager;

void first_thread(void *arg)
{
    printf("\n");
    printf("Start best-fit memory allocation test\n");

    // 分配 4 个不同大小的块
    char* p1 = (char*)memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 20);
    char* p2 = (char*)memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 30);
    char* p3 = (char*)memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 40);
    char* p4 = (char*)memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 50);

    printf("Allocated blocks:\n");
    printf("p1: %x\n", p1);
    printf("p2: %x\n", p2);
    printf("p3: %x\n", p3);
    printf("p4: %x\n", p4);

    // 释放其中两个，制造不同大小的空闲块
    memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, (int)p1, 20);
    memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, (int)p3, 40);

    printf("Freed p1 (20 pages) and p3 (40 pages).\n");

    // 申请18页，应选择p1对应位置
    char* p5 = (char*)memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 18);
    printf("Allocated p5 (18 pages): %x (expected near p1)\n", p5);

    // 申请35页，应选择p3对应位置
    char* p6 = (char*)memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 35);
    printf("Allocated p6 (35 pages): %x (expected near p3)\n", p6);

    memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, (int)p2, 30);
    memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, (int)p4, 50);
    memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, (int)p5, 18);
    memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, (int)p6, 35);

    printf("Best-fit memory allocation test complete.\n");

    asm_halt();
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

    // 内存管理器
    memoryManager.initialize();

    // 创建第一个线程
    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }

    ListItem *item = programManager.readyPrograms.front();
    PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
    firstThread->status = RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);

    asm_halt();
}
