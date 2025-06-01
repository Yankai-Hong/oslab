#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"

#define E820_BUFFER_ADDR 0x500
#define E820_LENGTH_ADDR 0x7c00

struct E820Entry {
    uint baseAddr;
    uint length;
    uint32 type;
} __attribute__((packed));

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;

void first_thread(void *arg)
{
    // 第1个线程不可以返回
    stdio.moveCursor(0);
    for (int i = 0; i < 25 * 80; ++i)
    {
        stdio.print(' ');
    }
    stdio.moveCursor(0);

    uint32 memory = *((uint32 *)MEMORY_SIZE_ADDRESS);
    // ax寄存器保存的内容
    uint32 low = memory & 0xffff;
    // bx寄存器保存的内容
    uint32 high = (memory >> 16) & 0xffff;
    memory = low * 1024 + high * 64 * 1024;
    printf("total memory: %d bytes (%d MB)\n", memory, memory / 1024 / 1024);

    // // 从 0x7c00 获取写入的 E820 总长度
    // uint totalLength = *(uint16 *)E820_LENGTH_ADDR;
    // int entryCount = totalLength / sizeof(E820Entry);

    // E820Entry *entries = (E820Entry *)E820_BUFFER_ADDR;

    // uint32 usableMemory = 0;
    // for (int i = 0; i < entryCount; ++i)
    // {
    //     if (entries[i].type == 1) // 可用内存
    //     {
    //         usableMemory += entries[i].length;
    //     }
    // }

    // printf("\nUsable memory: %llu bytes (%llu MB)\n",
    //        usableMemory, usableMemory / 1024 / 1024);
    
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
