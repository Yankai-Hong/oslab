#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"
#include "memory.h"
#include "syscall.h"
#include "tss.h"
#include "shell.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;
// 内存管理器
MemoryManager memoryManager;
// 系统调用
SystemService systemService;
// Task State Segment
TSS tss;

ByteMemoryManager kernelByteMemoryManager;
SpinLock kernelLock;

int syscall_0(int first, int second, int third, int forth, int fifth)
{
    printf("systerm call 0: %d, %d, %d, %d, %d\n",
           first, second, third, forth, fifth);
    return first + second + third + forth + fifth;
}

void first_process()
{
    int pid = fork();
    
    if(pid == -1) {
        printf("error\n");
        asm_halt();
    } else {
        if(pid) {
            while((pid = wait(nullptr)) != -1) {

            }
            asm_halt();
        } else {
            Shell shell;
            shell.initialize();
            shell.run();
        }
    }

}

void test_allocate() {
    const int BLOCK_SIZE = 16;

    printf("=== Testing %dB fixed-block allocation ===\n", BLOCK_SIZE);
    printf("Expected block interval: %d bytes\n", BLOCK_SIZE);

    // 分配多个块并记录地址
    void *ptrs[10];
    for (int i = 0; i < 10; ++i) {
        ptrs[i] = malloc(BLOCK_SIZE);
        printf("Block %d: addr=%d\n", i, ptrs[i]);
    }

    bool res = true;
    // 验证地址间隔
    for (int i = 1; i < 10; ++i) {
        uint32 prev_addr = (uint32)ptrs[i-1];
        uint32 curr_addr = (uint32)ptrs[i];
        uint32 expected_addr = prev_addr + BLOCK_SIZE;

        if (curr_addr != expected_addr) {
            printf("ERROR: Block %d addr %p != expected %p (offset %ld)\n",
                   i, (void*)curr_addr, (void*)expected_addr, curr_addr - prev_addr);
            res = false;
        }
    }
    if(res){
        printf("Allocation Succeed!. \n");
    }

    // 释放所有块
    for (int i = 0; i < 10; ++i) {
        free(ptrs[i]);
    }
    printf("All blocks freed. Test passed.\n");
}

void first_thread(void *arg)
{

    printf("start process\n");
    // programManager.executeProcess((const char *)first_process, 1);
    programManager.executeProcess((const char*)test_allocate, 1);
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

    // 初始化系统调用
    systemService.initialize();
    // 设置0号系统调用
    systemService.setSystemCall(0, (int)syscall_0);
    // 设置1号系统调用
    systemService.setSystemCall(1, (int)syscall_write);
    // 设置2号系统调用
    systemService.setSystemCall(2, (int)syscall_fork);
    // 设置3号系统调用
    systemService.setSystemCall(3, (int)syscall_exit);
    // 设置4号系统调用
    systemService.setSystemCall(4, (int)syscall_wait);
    // 设置5号系统调用
    systemService.setSystemCall(5, (int)syscall_move_cursor);

    //设置6号系统调用
    systemService.setSystemCall(6, (int)syscall_malloc);

    //设置7号系统调用
    systemService.setSystemCall(7, (int)syscall_free);


    // 内存管理器
    memoryManager.initialize();
    
    //内核字节内存管理器初始化
    kernelByteMemoryManager.initialize();

    // 创建第一个线程
    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }

    ListItem *item = programManager.readyPrograms.front();
    PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
    firstThread->status = ProgramStatus::RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);

    asm_halt();
}
