#include "main_thread.h"
#include "SEGGER_RTT.h"

TX_THREAD       two_thread;
TX_TIMER        timer;
UINT            timer_count = 0;

void timer_expiry_function(ULONG timer_input);

/* New Thread entry function */
void main_thread_entry(void)
{
    UINT status;

    // Create the Application timer
    tx_timer_create(&timer, "timer", timer_expiry_function, 0xA1, 1000, 0, TX_NO_ACTIVATE);       // one shot timer

    status = tx_timer_activate(&timer);
    if(TX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fail to active timer\r\n");
        return;
    }

    while(1)
    {
        SEGGER_RTT_printf(0, "one thread loop\r\n");
        if(1 == timer_count)
        {
            tx_timer_change(&timer, 3000, 300);     // repeat timer
            tx_timer_activate(&timer);
        }
        else if(6 < timer_count)
        {
            tx_timer_delete(&timer);
            timer_count = 0;
        }
        tx_thread_sleep(1500);
    }
}

void timer_expiry_function(ULONG timer_input)
{
    SEGGER_RTT_printf(0, "timer function - param data [0x%X]\r\n", timer_input);
    timer_count++;
}
