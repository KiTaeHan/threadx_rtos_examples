#include "main_thread.h"
#include "SEGGER_RTT.h"

#define BYTE_POOL_SIZE     9120
#define STACK_SIZE         1024

void two_thread_entry(ULONG thread_input);

TX_BYTE_POOL    my_byte_pool;
UCHAR           byte_pool_memory[BYTE_POOL_SIZE];

TX_THREAD       two_thread;
TX_MUTEX        mutex;

/* New Thread entry function */
void main_thread_entry(void)
{
    UINT status;
    VOID *stack_ptr;

    // Create a byte pool
    status = tx_byte_pool_create(&my_byte_pool, "My Byte Pool", byte_pool_memory, BYTE_POOL_SIZE);
    if (status != TX_SUCCESS)
    {
        return;
    }

    // Create the mutex
    tx_mutex_create(&mutex, "mutex 0", TX_NO_INHERIT);

    // Stack memory allocation
    status = tx_byte_allocate(&my_byte_pool, &stack_ptr, STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS)
    {
        return;
    }

    // Create a thread
    status = tx_thread_create(&two_thread, "My Thread", two_thread_entry, 0,
                              stack_ptr, STACK_SIZE,
                              10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        return;
    }

    /* TODO: add your own code here */
    while (1)
    {
        status = tx_mutex_get(&mutex, TX_WAIT_FOREVER);
        if(TX_SUCCESS != status) break;

        SEGGER_RTT_printf(0, "one thread loop\r\n");
        tx_thread_sleep(1500);

        status = tx_mutex_put(&mutex);
        if(TX_SUCCESS != status) break;
    }
}

void two_thread_entry(ULONG thread_input)
{
    (void)thread_input;

    UINT status;

    while (1)
    {
        status = tx_mutex_get(&mutex, TX_WAIT_FOREVER);
        if(TX_SUCCESS != status) break;

        SEGGER_RTT_printf(0, "two thread loop\r\n");
        tx_thread_sleep(1500);

        status = tx_mutex_put(&mutex);
        if(TX_SUCCESS != status) break;
    }
}

