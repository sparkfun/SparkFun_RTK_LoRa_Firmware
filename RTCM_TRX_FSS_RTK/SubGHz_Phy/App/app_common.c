#include "app_common.h"
#include "stm32wlxx_hal.h"
#include "queue.h"

void app_msg_queue_reset(QueueHandle_t que)
{
    if (NULL == que) { return; }
    xQueueReset(que);
}

int  app_msg_queue_recv(QueueHandle_t que, void *const pdata, int timeWait)
{
    if (NULL == que) { return false; }
    return xQueueReceive(que, pdata, timeWait);
}

void  app_msg_queue_send(QueueHandle_t que, const void *const pdata, int timeWait)
{
    if (NULL == que) { return; }
    int ret;
    ret = xQueueSend(que, pdata, timeWait);
    if (pdPASS != ret)
    {
        if (pdata != NULL)
        {
        	APP_TRANS_DATA_TYPE *msg = (APP_TRANS_DATA_TYPE*)pdata;
            if (msg->data != NULL)
            {
                free(msg->data);
                msg->data = NULL;
            }
        }
        APP_PRINTF("MSG send err:%d , p=%p",ret,que);
    }
}
