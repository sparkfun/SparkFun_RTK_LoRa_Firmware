#ifndef __APP_COMMON_H
#define __APP_COMMON_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "main.h"
#include "cmsis_os.h"
#include "queue.h"
#include "sys_app.h"
#include "utilities_def.h"
#include "utilities_conf.h"

#define VERSION "0.0.6"

#define MIN(i, j) (((i) < (j)) ? (i) : (j))
#define MAX(i, j) (((i) > (j)) ? (i) : (j))
typedef struct
{
    uint32_t evt;
    uint32_t len;
    uint8_t *data;
} APP_TRANS_DATA_TYPE;

/*All tasks PRIORITIES define*/
#define DRV_UART_CMD_EVENT_TASK_PRI (configMAX_PRIORITIES - 1)
#define DRV_UART_DATA_EVENT_TASK_PRI (configMAX_PRIORITIES - 1)

#define DRV_UHF_RX_RTCM_TASK_PRI (configMAX_PRIORITIES - 2)
#define DRV_UHF_TX_RTCM_TASK_PRI (configMAX_PRIORITIES - 2)
#define DRV_UHF_SIM_RTCM_TASK_PRI (configMAX_PRIORITIES - 4)
#define OS_DELAY_MS(time_ms) (vTaskDelay(pdMS_TO_TICKS(time_ms)))

#define app_msg_queue_create(queSize, dataSize) xQueueCreate((queSize), (dataSize))
#define app_msg_queue_delete(que) vQueueDelete(que)
extern void app_msg_queue_reset(QueueHandle_t que);
extern int app_msg_queue_recv(QueueHandle_t que, void *const pdata, int timeWait);
extern void app_msg_queue_send(QueueHandle_t que, const void *const pdata, int timeWait);

#endif
