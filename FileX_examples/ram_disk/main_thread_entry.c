#include "SEGGER_RTT/SEGGER_RTT.h"

#include "main_thread.h"
#include "tx_api.h"
#include "fx_api.h"


/* Macros for FileX */
#define G_FX_MEDIA_HEADS                (1)
#define G_FX_MEDIA_SECTORS_PER_TRACK    (1)
#define OPERATION_TIME_OUT              (100)


static FX_MEDIA     g_fx_media;
static uint8_t      g_fx_media_media_memory[G_FX_MEDIA_MEDIA_MEMORY_SIZE];

void system_init(void);


/* Event flags for media operations */
TX_EVENT_FLAGS_GROUP g_media_event_flag;

void rm_filex_block_media_callback(rm_filex_block_media_callback_args_t *p_args)
{
    switch(p_args->event)
    {
        case RM_BLOCK_MEDIA_EVENT_MEDIA_INSERTED:
            break;

        case RM_BLOCK_MEDIA_EVENT_MEDIA_REMOVED:
            break;

        case RM_BLOCK_MEDIA_EVENT_WAIT_END:
            tx_event_flags_set(&g_media_event_flag, RM_BLOCK_MEDIA_EVENT_WAIT_END, TX_OR);
            break;

        case RM_BLOCK_MEDIA_EVENT_POLL_STATUS:
        case RM_BLOCK_MEDIA_EVENT_MEDIA_SUSPEND:
        case RM_BLOCK_MEDIA_EVENT_MEDIA_RESUME:
        case RM_BLOCK_MEDIA_EVENT_WAIT:
        default:
            break;
    }
}

void system_init(void)
{
    UINT status = FX_SUCCESS;

    /* Open and configure FileX Block Media interface */
    status = (UINT)RM_FILEX_BLOCK_MEDIA_Open(&g_rm_filex_block_media_ctrl, &g_rm_filex_block_media_cfg);
    if(FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "RM_FILEX_BLOCK_MEDIA_Open failed\r\n");
    }

    /* Initialize the FileX system */
    fx_system_initialize();

    /* Set date for FileX system */
    status = fx_system_date_set (2025, 12, 25);
    if(FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fx_system_date_set failed\r\n");
    }

    /* Set time for FileX system */
    status = fx_system_time_set (12, 12, 12);
    if(FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fx_system_time_set failed\r\n");
    }

    /* Format the media */
    status = fx_media_format(&g_fx_media,                               // Pointer to FileX media control block.
                             RM_FILEX_BLOCK_MEDIA_BlockDriver,          // Driver entry
                             (void *) &g_rm_filex_block_media_instance,  // Pointer to LevelX NOR Driver
                             g_fx_media_media_memory,                   // Media buffer pointer
                             G_FX_MEDIA_MEDIA_MEMORY_SIZE,              // Media buffer size
                             (char *) G_FX_MEDIA_VOLUME_NAME,           // Volume Name
                             G_FX_MEDIA_NUMBER_OF_FATS,                 // Number of FATs
                             G_FX_MEDIA_DIRECTORY_ENTRIES,              // Directory Entries
                             G_FX_MEDIA_HIDDEN_SECTORS,                 // Hidden sectors
                             G_FX_MEDIA_TOTAL_SECTORS,                  // Total sectors
                             G_FX_MEDIA_BYTES_PER_SECTOR,               // Sector size
                             G_FX_MEDIA_SECTORS_PER_CLUSTER,            // Sectors per cluster
                             G_FX_MEDIA_HEADS,                          // Heads (disk media)
                             G_FX_MEDIA_SECTORS_PER_TRACK);             // Sectors per track (disk media)
    if(FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fx_media_format FileX failed\r\n");
    }

    /* Open media using the Azure FileX API */
    status = fx_media_open(&g_fx_media,
                           "g_fx_media",
                           RM_FILEX_BLOCK_MEDIA_BlockDriver,
                           (void *) &g_rm_filex_block_media_instance,
                           g_fx_media_media_memory,
                           G_FX_MEDIA_MEDIA_MEMORY_SIZE);
    if(FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fx_media_open failed\r\n");
    }

    /* Create directory using Azure FileX API */
    status = fx_directory_create(&g_fx_media, "temp");
    if (FX_ALREADY_CREATED == status)
    {
        SEGGER_RTT_printf(0, "Directory already exists\r\n");
    }
    if(FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fx_directory_create failed\r\n");
    }

    /* Flush data into the physical media */
    status = fx_media_flush(&g_fx_media);
    if(FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fx_media_flush failed\r\n");
    }

    /* Create a new file using the Azure FileX API */
    status = fx_file_create(&g_fx_media,  "myfile.txt");
    if (FX_ALREADY_CREATED == status)
    {
        SEGGER_RTT_printf(0, "File already exists\r\n");
    }

    if (FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fx_file_create failed\r\n");
    }

    FX_FILE file = {0};

    /* Open the file for writing by using the Azure FileX API */
    status = fx_file_open(&g_fx_media, &file, "myfile.txt", FX_OPEN_FOR_WRITE);
    if (FX_NOT_FOUND == status)
    {
        SEGGER_RTT_printf(0, "File does not exist\r\n");
    }

    if (FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fx_file_open failed\r\n");
    }

    /* Clean the file contents */
    status = fx_file_truncate(&file, 0);        // file size is 0 byte.
    if (FX_SUCCESS != status)
    {
        /* Return fx_file_extended_truncate failed status */
        SEGGER_RTT_printf(0, "fx_file_truncate failed\r\n");

        /* Close the file using the Azure FileX API */
        fx_file_close(&file);
    }

    /* Write fixed buffer to a file */
    status = fx_file_write(&file, "1234567890", 10);
    if (FX_SUCCESS != status)
    {
        /* Return fx_file_write failed status */
        SEGGER_RTT_printf(0, "fx_file_write failed\r\n");

        /* Close the file using the Azure FileX API */
        fx_file_close(&file);
    }

    ULONG actual_event = 0;
    /* Wait until the complete event flag is received */
    status = tx_event_flags_get(&g_media_event_flag,
                                RM_BLOCK_MEDIA_EVENT_WAIT_END,
                                TX_OR_CLEAR, &actual_event, OPERATION_TIME_OUT);
    if (TX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "tx_event_flags_get for write failed with status: %u\r\n", status);
        /* Close the file and return to avoid further issues */
        fx_file_close(&file);
    }
    /* Close the file using the Azure FileX API */
    status = fx_file_close(&file);
    if (FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fx_file_close failed\r\n");
    }

    /* Flush data into the physical media */
    status = fx_media_flush(&g_fx_media);
    if (FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fx_media_flush failed\r\n");
    }

    /* Open the file for reading by using the Azure FileX API */
    status = fx_file_open(&g_fx_media, &file, "myfile.txt", FX_OPEN_FOR_READ);
    if (FX_NOT_FOUND == status)
    {
        SEGGER_RTT_printf(0, "File does not exist\r\n");
    }
    if (FX_NOT_FOUND == status)
    {
        SEGGER_RTT_printf(0, "File does not exist\r\n");
    }

    /* Seek to the beginning of the file  */
    status = fx_file_seek(&file, 0);
    if (FX_SUCCESS != status)
    {
       /* Return fx_file_seek failed status */
       SEGGER_RTT_printf(0, "fx_file_extended_seek failed\r\n");

       /* Close the file using the Azure FileX API */
       fx_file_close(&file);
    }

    unsigned char           my_buffer[1024];
    ULONG                   actual_bytes;

    /* Read up to 1024 bytes into "my_buffer." */
    status = fx_file_read(&file, my_buffer, 1024, &actual_bytes);

    /* In case reading the file failed */
    if (FX_END_OF_FILE != status && FX_SUCCESS != status)
    {
        /* Return fx_file_read failed status */
        SEGGER_RTT_printf(0, "fx_file_read failed\r\n");

        /* Close the file using the Azure FileX API */
        fx_file_close(&file);
    }

    /* Wait until the complete event flag is received */
    status = tx_event_flags_get(&g_media_event_flag,
                                RM_BLOCK_MEDIA_EVENT_WAIT_END,
                                TX_OR_CLEAR, &actual_event, OPERATION_TIME_OUT);
    if (TX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "tx_event_flags_get for read failed with status: %u\r\n", status);
        /* Close the file and return to avoid further issues */
        fx_file_close(&file);
    }
    /* Close the file using the Azure FileX API */
    status = fx_file_close(&file);
    if (FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fx_file_close failed\r\n");
    }

    /* Delete the file using the Azure FileX API */
    status = fx_file_delete(&g_fx_media, "myfile.txt");
    if (FX_NOT_FOUND == status)
    {
        SEGGER_RTT_printf(0, "File does not exist\r\n");
    }
    if (FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fx_file_delete failed\r\n");
    }

    /* Flush data into the physical media */
    status = fx_media_flush(&g_fx_media);
    if (FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fx_media_flush failed\r\n");
    }


    /* Delete a directory using Azure FileX API */
    status = fx_directory_delete(&g_fx_media, "temp");
    if (FX_NOT_FOUND == status)
    {
        SEGGER_RTT_printf(0, "Directory not exists\r\n");
    }
    if (FX_DIR_NOT_EMPTY == status)
    {
        SEGGER_RTT_printf(0, "Directory not empty\r\n");
    }
    if (FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fx_directory_delete failed\r\n");
    }

    /* Flush data into the physical media */
    status = fx_media_flush(&g_fx_media);
    if (FX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "fx_media_flush failed\r\n");
    }

    /* Close media before format */
    status = fx_media_close(&g_fx_media);
    if (FX_SUCCESS != status && FX_MEDIA_NOT_OPEN != status)
    {
        SEGGER_RTT_printf(0, "fx_media_close failed\r\n");
    }

    RM_FILEX_BLOCK_MEDIA_Close (&g_rm_filex_block_media_ctrl);
}

/* New Thread entry function */
void main_thread_entry(void)
{
    UINT status = TX_SUCCESS;

    /* Create an event flags group for media operation synchronization. */
    status = tx_event_flags_create(&g_media_event_flag, "Media Events");
    if (TX_SUCCESS != status)
    {
        SEGGER_RTT_printf(0, "tx_event_flags_create failed\r\n");
    }
    system_init();

    SEGGER_RTT_printf(0, "Start application\r\n");

    /* TODO: add your own code here */
    while (1)
    {
        tx_thread_sleep (1);
    }
}
