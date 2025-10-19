#include <main_thread.h>

#define BYTE_POOL_SIZE     9120
#define STACK_SIZE         1024

void delete_three_thread(void);
void three_thread_entry(ULONG thread_input);

TX_THREAD       three_thread;
TX_BYTE_POOL    my_byte_pool;
UCHAR           byte_pool_memory[BYTE_POOL_SIZE];


void delete_three_thread(void)
{
    VOID *stack_ptr = three_thread.tx_thread_stack_start;

    if( (three_thread.tx_thread_state != TX_READY) &&
        (three_thread.tx_thread_state != TX_SUSPENDED) &&
        (three_thread.tx_thread_state != TX_TERMINATED) )
    {
        // 삭제 가능 여부
        tx_thread_suspend(&three_thread);
    }

    // 스레드 삭제
    tx_thread_delete(&three_thread);
    tx_byte_release(stack_ptr);
}

/* New Thread entry function */
void main_thread_entry(void)
{
    UINT status;
    VOID *stack_ptr;
    int i = 0;

    // 1. 바이트 풀 생성
    status = tx_byte_pool_create(&my_byte_pool, "My Byte Pool", byte_pool_memory, BYTE_POOL_SIZE);
    if (status != TX_SUCCESS)
    {
        return;
    }

    // 2. 스택 메모리 할당
    status = tx_byte_allocate(&my_byte_pool, &stack_ptr, STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS)
    {
        return;
    }

    // 3. 스레드 생성
    status = tx_thread_create(&three_thread, "My Thread", three_thread_entry, 0,
                              stack_ptr, STACK_SIZE,
                              10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        return;
    }


    /* TODO: add your own code here */
    while (1)
    {
        R_IOPORT_PinWrite(&g_ioport_ctrl, LED1, BSP_IO_LEVEL_HIGH);
        tx_thread_sleep (400);
        R_IOPORT_PinWrite(&g_ioport_ctrl, LED1, BSP_IO_LEVEL_LOW);
        tx_thread_sleep (400);
        i++;
        if(5 == i)
        {
            delete_three_thread();
        }
    }
}

void three_thread_entry(ULONG thread_input)
{
    (void)thread_input;

    while (1)
    {
        R_IOPORT_PinWrite(&g_ioport_ctrl, LED3, BSP_IO_LEVEL_HIGH);
        tx_thread_sleep (250);
        R_IOPORT_PinWrite(&g_ioport_ctrl, LED3, BSP_IO_LEVEL_LOW);
        tx_thread_sleep (250);
    }
}

