#include "main_thread.h"
#include "SEGGER_RTT.h"

#define BYTE_POOL_SIZE     9120
#define STACK_SIZE         1024
#define QUEUE_SIZE         100
#define SIZE_IN_WORDS       1

void two_thread_entry(ULONG thread_input);

TX_BYTE_POOL    my_byte_pool;
UCHAR           byte_pool_memory[BYTE_POOL_SIZE];

TX_THREAD       two_thread;
TX_QUEUE    message_queue;

// Memory for the queue
ULONG queue_storage[QUEUE_SIZE * SIZE_IN_WORDS];

/* New Thread entry function */
void main_thread_entry(void)
{
    UINT status;
    VOID *stack_ptr;

    // 1. 바이트 풀 생성
    status = tx_byte_pool_create(&my_byte_pool, "My Byte Pool", byte_pool_memory, BYTE_POOL_SIZE);
    if (status != TX_SUCCESS)
    {
        return;
    }

    // Create the message queue
    status = tx_queue_create(&message_queue, "message queue", SIZE_IN_WORDS,    // 4bytes
                             queue_storage, QUEUE_SIZE*sizeof(ULONG));
    if (status != TX_SUCCESS)
    {
        // Handle error
    }


    // 2. 스택 메모리 할당
    status = tx_byte_allocate(&my_byte_pool, &stack_ptr, STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS)
    {
        return;
    }

    // 3. 스레드 생성
    status = tx_thread_create(&two_thread, "My Thread", two_thread_entry, 0,
                              stack_ptr, STACK_SIZE,
                              10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        return;
    }


    /* TODO: add your own code here */
    ULONG message_sent = 0;

    while (1)
    {
        // Increment message count
        message_sent++;

        // Send a message to the queue. Wait forever if the queue is full.
        status = tx_queue_send(&message_queue, &message_sent, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)
        {
            // Handle error
            break;
        }

        tx_thread_sleep(500);
    }
}

void two_thread_entry(ULONG thread_input)
{
    (void)thread_input;

    UINT status;
    ULONG received_message;

    while (1)
    {
        // Receive a message from the queue. Wait forever if the queue is empty.
        status = tx_queue_receive(&message_queue, &received_message, TX_WAIT_FOREVER);
        if (status == TX_SUCCESS)
        {
            // Process the received message
            SEGGER_RTT_printf(0, "Received message: %lu\n", received_message);
        }
        else
        {
            // Handle error
            break;
        }
    }
}

