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
    printf("\n Memory Allocation Test Begin\n");

    // 分配内存
    char *p1 = (char *)memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 5);
    char *p2 = (char *)memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 8);
    char *p3 = (char *)memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 3);

    printf("Allocated:\n");
    printf("    p1: 0x%x\n", (int)p1);
    printf("    p2: 0x%x\n", (int)p2);
    printf("    p3: 0x%x\n", (int)p3);

    // 释放 p2
    memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, (int)p2, 8);
    printf("Released p2: 0x%x (8 pages)\n", (int)p2);

    // 再次尝试分配一段相同大小的页
    char *p4 = (char *)memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 8);
    printf("Reallocated p4 (8 pages): 0x%x\n", (int)p4);

    if ((int)p2 == (int)p4)
        printf("Reused p2's space correctly\n");
    else
        printf("Did not reuse p2's space\n");

    // 释放所有分配
    memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, (int)p1, 5);
    memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, (int)p3, 3);
    memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, (int)p4, 8);

    printf("Memory Allocation Test Complete\n");
    

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
    memoryManager.openPageMechanism();
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
