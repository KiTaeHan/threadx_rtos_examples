#include "two_thread.h"

/* Tow Thread entry function */
void two_thread_entry(void)
{
    /* TODO: add your own code here */
    while (1)
    {
        R_IOPORT_PinWrite(&g_ioport_ctrl, LED2, BSP_IO_LEVEL_HIGH);
        tx_thread_sleep (300);
        R_IOPORT_PinWrite(&g_ioport_ctrl, LED2, BSP_IO_LEVEL_LOW);
        tx_thread_sleep (300);
    }
}
