#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;

Semaphore semaphore;
int msg;

int cheese_burger;

int apple;
int banana;
Semaphore apple_sema;
Semaphore banana_sema;

const int count = 5;
Semaphore chopsticks[count];

unsigned int seed = 19888889;  // 任意初始种子

// 简单的伪随机数生成器，返回 0 或 1
int rand_bit() {
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 1;
}

int rand_int(){
    seed = seed * 1103515245 + 1234;
    return (seed >> 16) % 11;
}

void a_mother(void *arg)
{
    semaphore.P();
    int delay = 0;

    printf("mother: start to make cheese burger, there are %d cheese burger now\n", cheese_burger);
    // make 10 cheese_burger
    cheese_burger += 10;

    printf("mother: oh, I have to hang clothes out.\n");
    // hanging clothes out
    delay = 0xfffffff;
    while (delay)
        --delay;
    // done

    printf("mother: Oh, Jesus! There are %d cheese burgers\n", cheese_burger);
    semaphore.V();
}

void a_naughty_boy(void *arg)
{
    semaphore.P();
    printf("boy   : Look what I found!\n");
    // eat all cheese_burgers out secretly
    cheese_burger -= 10;
    // run away as fast as possible
    semaphore.V();
}

void send_msg(void *arg){
    semaphore.P();

    // 发送一条消息
    msg++;

    printf("Send a message! \n");
    printf("Now have %d message. \n", msg);

    semaphore.V();
}

void rec_msg(void *arg){
    semaphore.P();

    msg--;
    printf("Receive a message! \n");
    printf("Npw have %d message. \n", msg);

    semaphore.V();
}

void produce_new(void *arg){
    int count = 10;

    while (count--)
    {
        if(rand_bit()){
            printf("Produce an apple. \n");
            apple_sema.V();
        }else{
            printf("Produce a banana. \n");
            banana_sema.V();
        }

        int delay = 0xfffffff;
        while (delay)
            --delay;
        
        printf("Have %d apple(s) and %d banana(s). \n", apple_sema.get_counter(), banana_sema.get_counter());
        printf("\n");

        programManager.schedule();
    }

    printf("Over \n");
    
}

void consume_new(void *arg){
    int count = 10;

    while (count--)
    {
        if(rand_bit()){
            apple_sema.P();
            printf("Eat an apple. \n");
        }else{
            banana_sema.P();
            printf("Eat a banana. \n");
        }
        printf("Have %d apple(s) and %d banana(s). \n", apple_sema.get_counter(),
         banana_sema.get_counter());
        printf("\n");

        programManager.schedule();
    }
    
}

void produce(void *arg){
    int count = 10;

    while (count--)
    {
        if(rand_bit()){
            apple++;
            printf("Produce an apple. \n");
        }else{
            banana++;
            printf("Produce a banana. \n");
        }
        
        int delay = 0xfffffff;
        while (delay)
            --delay;

        printf("Have %d apple(s) and %d banana(s). \n", apple, banana);
        printf("\n");

        programManager.schedule();
    }

    printf("Over \n");
    
}

void consume(void *arg){
    int count = 10;

    while (count--)
    {
        if(rand_bit()){
            apple--;
            printf("Eat an apple. \n");
        }else{
            banana--;
            printf("Eat a banana. \n");
        }
        printf("Have %d apple(s) and %d banana(s). \n", apple, banana);
        printf("\n");

        programManager.schedule();
    }
}

void philosopher_1(void* arg){
    int index = 1;
    int left = (index - 1) % 5;
    int right = (index + 1) % 5;

    int first = left < right ? left : right;
    int second = left < right ? right : left;

    while(true){
        if(rand_int() > 8){
            chopsticks[first].P();
            chopsticks[second].P();
    
            printf("Philosopher %d finish meal! \n", index);
            printf("\n");
    
            chopsticks[first].V();
            chopsticks[second].V();
            break;
        }else{
            programManager.schedule();
        }
    }
}
void philosopher_2(void* arg){
    int index = 2;
    int left = (index - 1) % 5;
    int right = (index + 1) % 5;

    int first = left < right ? left : right;
    int second = left < right ? right : left;

    while(true){
        if(rand_int() > 8){
            chopsticks[first].P();
            chopsticks[second].P();
    
            printf("Philosopher %d finish meal! \n", index);
            printf("\n");
    
            chopsticks[first].V();
            chopsticks[second].V();
            break;
        }else{
            programManager.schedule();
        }
    }
}
void philosopher_3(void* arg){
    int index = 3;
    int left = (index - 1) % 5;
    int right = (index + 1) % 5;

    int first = left < right ? left : right;
    int second = left < right ? right : left;

    while(true){
        if(rand_int() > 8){
            chopsticks[first].P();
            chopsticks[second].P();
    
            printf("Philosopher %d finish meal! \n", index);
            printf("\n");
    
            chopsticks[first].V();
            chopsticks[second].V();
            break;
        }else{
            programManager.schedule();
        }
    }
}
void philosopher_4(void* arg){
    int index = 4;
    int left = (index - 1) % 5;
    int right = (index + 1) % 5;

    int first = left < right ? left : right;
    int second = left < right ? right : left;

    while(true){
        if(rand_int() > 8){
            chopsticks[first].P();
            chopsticks[second].P();
    
            printf("Philosopher %d finish meal! \n", index);
            printf("\n");
    
            chopsticks[first].V();
            chopsticks[second].V();
            break;
        }else{
            programManager.schedule();
        }
    }
}
void philosopher_5(void* arg){
    int index = 5;
    int left = (index - 1) % 5;
    int right = (index + 1) % 5;

    int first = left < right ? left : right;
    int second = left < right ? right : left;

    while(true){
        if(rand_int() > 8){
            chopsticks[first].P();
            chopsticks[second].P();
    
            printf("Philosopher %d finish meal! \n", index);
            printf("\n");
    
            chopsticks[first].V();
            chopsticks[second].V();
            break;
        }else{
            programManager.schedule();
        }
    }
}


void first_thread(void *arg)
{
    // 第1个线程不可以返回
    stdio.moveCursor(0);
    for (int i = 0; i < 25 * 80; ++i)
    {
        stdio.print(' ');
    }
    stdio.moveCursor(0);

    msg = 0;
    semaphore.initialize(1);
    

    apple = 0;
    banana = 0;

    apple_sema.initialize(0);
    banana_sema.initialize(0);

    // programManager.executeThread(produce_new, nullptr, "produce", 1);
    // programManager.executeThread(consume_new, nullptr, "consume", 1);

    for (int i = 0; i < count; i++)
    {
        chopsticks[i].initialize(1);
    }
    programManager.executeThread(philosopher_1, nullptr, "philosopher_1", 1);
    programManager.executeThread(philosopher_2, nullptr, "philosopher_2", 1);
    programManager.executeThread(philosopher_3, nullptr, "philosopher_3", 1);
    programManager.executeThread(philosopher_4, nullptr, "philosopher_4", 1);
    programManager.executeThread(philosopher_5, nullptr, "philosopher_5", 1);

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
