#include "main_thread.h"
#include "SEGGER_RTT.h"

#define BYTE_POOL_SIZE     9120
#define STACK_SIZE         1024

void two_thread_entry(ULONG thread_input);

TX_BYTE_POOL    my_byte_pool;
UCHAR           byte_pool_memory[BYTE_POOL_SIZE];

TX_THREAD       two_thread;
TX_EVENT_FLAGS_GROUP event_flags;

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

    // Create the event flags group
    tx_event_flags_create(&event_flags, "event flags 0");

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
        // Set event flag
        status = tx_event_flags_set(&event_flags, 0x3, TX_OR);
        if(TX_SUCCESS != status) break;

        SEGGER_RTT_printf(0, "one thread loop\r\n");
        tx_thread_sleep(1500);
    }
}

void two_thread_entry(ULONG thread_input)
{
    (void)thread_input;

    UINT status;
    ULONG flags;

    while (1)
    {
        // Wait for event flag
        status = tx_event_flags_get(&event_flags, 0x2, TX_AND_CLEAR, &flags, TX_WAIT_FOREVER);
        if(TX_SUCCESS != status) break;
        SEGGER_RTT_printf(0, "two thread's flag data 1: 0x%X\r\n", flags);

        status = tx_event_flags_get(&event_flags, 0x1, TX_OR_CLEAR, &flags, TX_WAIT_FOREVER);
        if(TX_SUCCESS != status) break;
        SEGGER_RTT_printf(0, "two thread's flag data 2: 0x%X\r\n", flags);
    }
}

