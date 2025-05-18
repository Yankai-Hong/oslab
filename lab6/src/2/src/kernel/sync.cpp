#include "sync.h"
#include "asm_utils.h"
#include "stdio.h"
#include "os_modules.h"

extern "C" void lock_acquire(uint32* lock);

SpinLock::SpinLock()
{
    initialize();
}

void SpinLock::initialize()
{
    bolt = 0;
}

void SpinLock::lock()
{
    lock_acquire(&bolt);
}

void SpinLock::unlock()
{
    bolt = 0;
}