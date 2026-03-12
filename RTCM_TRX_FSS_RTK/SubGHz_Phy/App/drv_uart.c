#include <string.h>
#include "main.h"
#include "cmsis_os.h"
#include "queue.h"
#include "event_groups.h"
#include "stm32_systime.h"
#include "queue.h"
#include "app_common.h"
#include "stm32wlxx_hal.h"
#include "radio.h"
#include "drv_radio.h"
#include "drv_uart.h"
#include "sys_app.h"
#include "usart.h"
#include "dma.h"
#include "stm32wlxx_hal_uart.h"
#include "utilities_conf.h"
#include "usr_cmd.h"
#define UART_DMA_IDLE_MODE  1
#define LOG_TAG "DRVUART"

#define RX_IRQ_SIZE (128)
#if (UART_DMA_IDLE_MODE==1)
#define RX_DMA_SIZE  (1536)
#else
#define RX_DMA_SIZE  (255)
#endif
#define RX_BUFF_SIZE (1024)
#define TX_BUFF_SIZE (1024)


// #define CMD_PORT 0x01
// #define DATA_PORT 0x02

typedef enum _DRV_EVT
{
    DRV_EVT_RX_DONE = 0,
    DRV_EVT_RX_TO,
    DRV_EVT_RX_FULL,
    DRV_EVT_TX_DONE,
    DRV_EVT_TX_EMPTY
} DRV_EVT;

typedef struct _DR_TRANS
{
    uint32_t evt_id;
    uint32_t len;
    uint8_t buf[RX_IRQ_SIZE + 1];
} DRV_TRANS_DATA_TYPE;

#define DRV_TRANS_ITEM_SIZE (sizeof(DRV_TRANS_DATA_TYPE))
#define UART_QUEUE_NUM (20) // 2k

static QueueHandle_t uart1Queue = NULL;  // com1
static QueueHandle_t uart2Queue = NULL; //  com2
static EventGroupHandle_t send_uart1_event;
static EventGroupHandle_t send_uart2_event;

static drv_uart_read_cb_t uart1_read_cb_func = NULL;
static drv_uart_read_cb_t uart2_read_cb_func = NULL;

#define UART1_SEND_CPLT (1 << 0)
#define UART2_SEND_CPLT (1 << 1)

typedef struct
{
    uint32_t evt;
    uint32_t len;
    uint8_t *data; // pt to static buffer
} DRV_EVENT_TYPE;

typedef volatile struct
{
    volatile uint16_t buffSize;
    volatile uint8_t *pHead;
    volatile uint8_t *pTail;
    volatile uint8_t *pBuff;
} OS_DS_QUEUE_TYPE;

uint16_t os_ds_que_create(OS_DS_QUEUE_TYPE *q, uint8_t *buff, uint16_t size)
{
    uint8_t *pbuff = malloc(size);

    if (NULL == pbuff)
    {
        APP_TPRINTF("os_ds_que_create fail for malloc %d for buffer", size);
    }
    else
    {
        memset(pbuff, 0, size);
    }

    q->buffSize = size;
    q->pBuff = pbuff;
    q->pHead = pbuff;
    q->pTail = pbuff;

    return 0;
}

int os_ds_que_cleanup(OS_DS_QUEUE_TYPE *q)
{
    if (q->pBuff != NULL)
    {
        q->pHead = q->pBuff;
        q->pTail = q->pBuff;
        memset((void *)q->pBuff, 0, q->buffSize);
        return 0;
    }
    else
    {
        return -1;
    }
}

uint16_t os_ds_que_destroy(OS_DS_QUEUE_TYPE *q)
{
    free((uint8_t *)q->pBuff);

    q->buffSize = 0;
    q->pBuff = NULL;
    q->pHead = NULL;
    q->pTail = NULL;

    return 0;
}

uint16_t os_ds_que_size(OS_DS_QUEUE_TYPE *q)
{
    volatile uint8_t *pHead = NULL;
    volatile uint8_t *pTail = NULL;
    uint16_t size = 0;

    pHead = q->pHead;
    pTail = q->pTail;

    if (pTail - pHead >= 0)
    {
        size = pTail - pHead;
    }
    else
    {
        size = pTail - pHead + q->buffSize;
    }

    return size;
}

uint16_t os_ds_que_push(OS_DS_QUEUE_TYPE *q, uint8_t byte)
{
    volatile uint8_t *pTail = NULL;

    pTail = q->pTail;

    if (++pTail >= (q->pBuff + q->buffSize)) // back to buffer area header
    {
        pTail = q->pBuff;
    }

    if (pTail == q->pHead) // que is full
    {
        return 0;
    }

    *(q->pTail) = byte;

    q->pTail = pTail;

    return 1;
}

uint16_t os_ds_que_pop(OS_DS_QUEUE_TYPE *q)
{
    uint8_t byte = 0;

    if (q->pHead != q->pTail)
    {
        byte = *(q->pHead);
        q->pHead++;

        if (q->pHead >= q->pBuff + q->buffSize)
        {
            q->pHead = q->pBuff;
        }
    }

    return byte;
}

uint16_t os_ds_que_packet_in(OS_DS_QUEUE_TYPE *q, uint8_t *buff, uint16_t len)
{
    volatile uint8_t *pTail = NULL;
    uint16_t idx = 0;

    pTail = q->pTail;

    for (idx = 0; idx < len; ++idx)
    {
        if (++pTail >= q->pBuff + q->buffSize)
        {
            pTail = q->pBuff;
        }
        if (pTail == q->pHead)
        {
            break;
        }

        *(q->pTail) = *(buff);
        buff++;

        q->pTail = pTail;
    }

    return idx;
}

uint16_t os_ds_que_packet_out(OS_DS_QUEUE_TYPE *q, uint8_t *buff, uint16_t len)
{
    uint16_t idx = 0;

    while ((q->pHead != q->pTail) && (idx < len) && (idx < q->buffSize))
    {
        buff[idx++] = *(q->pHead);
        q->pHead++;

        if (q->pHead >= q->pBuff + q->buffSize)
        {
            q->pHead = q->pBuff;
        }
    }

    return idx;
}

typedef struct _map_uart
{
    int com_id;
    UART_HandleTypeDef *uart;
} MAP_UART;

//static MAP_UART uart_tbl[2] =
//    {
//        {.com_id = CMD_PORT, .uart = &huart1},
//        {.com_id = DATA_PORT, .uart = &huart2}};
static uint8_t uart1_rx_buf[RX_DMA_SIZE+1] ;
static uint8_t uart2_rx_buf[RX_DMA_SIZE+1] ;
// static uint8_t uart1_buf[RX_BUFF_SIZE];
// static uint8_t uart2_buf[RX_BUFF_SIZE];
// static uint32_t uart1_offset = 0;
// static uint32_t uart2_offset = 0;
static HAL_StatusTypeDef start_uart_rx_irq(UART_HandleTypeDef *huart);
#define MAX_SEND_WAIT 50
const uint32_t BPS_MS_WAIT = ((100 * 115200) / 1000) + 1; // 100ms
static void UART_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    DRV_TRANS_DATA_TYPE data;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int rx_len = size;
//    int split_len = 0;
    // HAL_UART_ReceiverTimeout_Config(huart, BPS_MS_WAIT);
    // HAL_UART_EnableReceiverTimeout(huart);
    data.len = 0;
    data.evt_id = DRV_EVT_RX_DONE;
    // check RxEventType
    // if(huart->RxEventType == HAL_UART_RXEVENT_HT)
    // {
    // 	return;
    // }
    APP_TPRINTF("rx=%d evt=%x\r\n", size,huart->RxEventType);
    if (rx_len > 0)
    {
        //data.len = rx_len;
        if (huart->Instance == USART1)
        {
        	// not handle half dma
        	if(huart->RxEventType == HAL_UART_RXEVENT_IDLE ||
        			huart->RxEventType == HAL_UART_RXEVENT_TC)
        	{
				for (int i = 0; i < rx_len; i += RX_IRQ_SIZE)
				{
					int length = MIN(RX_IRQ_SIZE, rx_len - i);
					memcpy(data.buf, uart1_rx_buf+i, length);
					data.len = length;
                    if (!xQueueIsQueueFullFromISR(uart1Queue))
                    {
                        xQueueSendToBackFromISR(uart1Queue, &data, &xHigherPriorityTaskWoken);
                        //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
                    }
                    else
                    {
                        APP_TPRINTF("full com1\r\n");
                    }
				}
                //HAL_UARTEx_ReceiveToIdle_IT(huart, &uart1_rx_buf[0], RX_IRQ_SIZE);

                start_uart_rx_irq(huart);
        	}

        }

        if (huart->Instance == USART2)
        {
        	// not handle half dma
        	if(huart->RxEventType == HAL_UART_RXEVENT_IDLE ||
        			huart->RxEventType == HAL_UART_RXEVENT_TC)
        	{
                // APP_TPRINTF("uart2 rx done \r\n");
//                memcpy(data.buf, uart2_rx_buf, rx_len);
//                //HAL_UARTEx_ReceiveToIdle_IT(huart, &uart2_rx_buf[0], RX_IRQ_SIZE);
//                start_uart_rx_irq(huart);
//                //memset(uart2_rx_buf, 0, sizeof(uart2_rx_buf));
//                if (!xQueueIsQueueFullFromISR(uart2Queue))
//                {
//                    xQueueSendToBackFromISR(uart2Queue, &data, &xHigherPriorityTaskWoken);
//                    //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//                }

				for (int i = 0; i < rx_len; i += RX_IRQ_SIZE)
				{
					int length = MIN(RX_IRQ_SIZE, rx_len - i);
					memcpy(data.buf, uart2_rx_buf+i, length);
					data.len = length;
                    if (!xQueueIsQueueFullFromISR(uart2Queue))
                    {
                        xQueueSendToBackFromISR(uart2Queue, &data, &xHigherPriorityTaskWoken);
                        //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
                    }
                    else
                    {
                        APP_TPRINTF("full com1\r\n");
                    }
				}
				start_uart_rx_irq(huart);
        	}
        }
    }
}
/* dma + irq idle*/
static void UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;
    xHigherPriorityTaskWoken = pdFALSE;
    /* Prevent unused argument(s) compilation warning */
    if (huart->Instance == USART1)
    {
        xResult = xEventGroupSetBitsFromISR(
            send_uart1_event, /* The event group being updated. */
            UART1_SEND_CPLT,  /* The bits being set. */
            &xHigherPriorityTaskWoken);
        APP_TPRINTF("txcpl com1\r\n");
    }

    if (huart->Instance == USART2)
    {
        xResult = xEventGroupSetBitsFromISR(
            send_uart2_event, /* The event group being updated. */
            UART2_SEND_CPLT,  /* The bits being set. */
            &xHigherPriorityTaskWoken);
        APP_TPRINTF("txcpl com2\r\n");   
    }

    if (xResult != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


static HAL_StatusTypeDef start_uart_rx_irq(UART_HandleTypeDef *huart)
{
    HAL_StatusTypeDef ret = HAL_ERROR;
    //HAL_UART_ReceiverTimeout_Config(huart, BPS_MS_WAIT);
    //HAL_UART_EnableReceiverTimeout(huart);
#if (UART_DMA_IDLE_MODE==0)
    if (huart->Instance == USART1)
    {
        ret = HAL_UARTEx_ReceiveToIdle_IT(huart, &uart1_rx_buf[0], RX_DMA_SIZE);
    }
    if (huart->Instance == USART2)
    {
        ret = HAL_UARTEx_ReceiveToIdle_IT(huart, &uart2_rx_buf[0], RX_DMA_SIZE);
    }
#else
    if (huart->Instance == USART1)
    {
        ret = HAL_UARTEx_ReceiveToIdle_DMA(huart,&uart1_rx_buf[0],RX_DMA_SIZE);
        // Disable DMA Half Transfer interrupt to avoid unnecessary interrupts
        if(huart->hdmarx != NULL)
        {
            __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
        }
        APP_TPRINTF("UART1 DMA RX start ret=%d\r\n", ret);
    }
    if (huart->Instance == USART2)
    {
        ret = HAL_UARTEx_ReceiveToIdle_DMA(huart,&uart2_rx_buf[0],RX_DMA_SIZE);
        // Disable DMA Half Transfer interrupt to avoid unnecessary interrupts
        if(huart->hdmarx != NULL)
        {
            __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
        }
        APP_TPRINTF("UART2 DMA RX start ret=%d hdmarx=%p\r\n", ret, huart->hdmarx);
    }
#endif
    return ret;
}


void UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    // if (huart->ErrorCode == HAL_UART_ERROR_RTO)
    // {
    //     APP_TPRINTF("uart1 to");
    //     start_uart_rx_irq(huart);
    // }
	APP_TPRINTF("Uart[%d] ErrorCode:0x%x\r\n",(huart->Instance ==USART2), huart->ErrorCode);
	if(huart->ErrorCode == HAL_UART_ERROR_DMA)
	{
		start_uart_rx_irq(huart);// DMA error,data will drop
	}
    if (huart->ErrorCode == HAL_UART_ERROR_ORE)
    {
        DRV_TRANS_DATA_TYPE data;
        BaseType_t xHigherPriorityTaskWoken;
        xHigherPriorityTaskWoken = pdFALSE;
        if (huart->Instance == USART1)
        {
            int xlen = huart->RxXferCount ;
            if (xlen > 0)
            {
            	data.evt_id = DRV_EVT_RX_DONE;
				for (int i = 0; i < xlen; i += RX_IRQ_SIZE)
				{
					int length = MIN(RX_IRQ_SIZE, xlen - i);
					memcpy(data.buf, uart1_rx_buf+i, length);
					data.len = length;
                    if (!xQueueIsQueueFullFromISR(uart1Queue))
                    {
                        xQueueSendToBackFromISR(uart1Queue, &data, &xHigherPriorityTaskWoken);
                        //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
                    }
                    else
                    {
                        APP_TPRINTF("full com1\r\n");
                    }
				}

            }
            start_uart_rx_irq(huart);
//            data.evt_id = DRV_EVT_RX_DONE;
//            data.len = xlen;
            APP_TPRINTF("uart1 ov xlen=%d \r\n",xlen);
//            //HAL_UARTEx_ReceiveToIdle_IT(huart, &uart1_rx_buf[0], RX_IRQ_SIZE);

//            if (!xQueueIsQueueFullFromISR(uart1Queue))
//            {
//                xQueueSendToBackFromISR(uart1Queue, &data, &xHigherPriorityTaskWoken);
//                //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//            }
//            else
//            {
//                APP_TPRINTF("full com1\r\n");
//            }
        }

        if (huart->Instance == USART2)
        {
            int xlen = huart->RxXferCount;
            if (xlen > 0)
                //memcpy(data.buf, huart->pRxBuffPtr, xlen);
            {
            	data.evt_id = DRV_EVT_RX_DONE;
				for (int i = 0; i < xlen; i += RX_IRQ_SIZE)
				{
					int length = MIN(RX_IRQ_SIZE, xlen - i);
					memcpy(data.buf, uart2_rx_buf+i, length);
					data.len = length;
                    if (!xQueueIsQueueFullFromISR(uart2Queue))
                    {
                        xQueueSendToBackFromISR(uart2Queue, &data, &xHigherPriorityTaskWoken);
                        //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
                    }
                    else
                    {
                        APP_TPRINTF("full com1\r\n");
                    }
				}
            }
//            data.evt_id = DRV_EVT_RX_DONE;
//            data.len = xlen;
            start_uart_rx_irq(huart);
            APP_TPRINTF("uart2 ov  xlen=%d \r\n",xlen);
            //HAL_UARTEx_ReceiveToIdle_IT(huart, &uart2_rx_buf[0], RX_IRQ_SIZE);

//            if (!xQueueIsQueueFullFromISR(uart2Queue))
//            {
//                xQueueSendToBackFromISR(uart2Queue, &data, &xHigherPriorityTaskWoken);
//                //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//            }
//            else
//            {
//                APP_TPRINTF("full com2\r\n");
//            }
        }
    }
}

static void drv_uart_event_task(void *pvParameters)
{
    int port = *(int *)pvParameters;
    QueueHandle_t *pQueue = NULL;
    drv_uart_read_cb_t *read_cb_func = NULL;
    //uint8_t rxBuff[RX_BUFF_SIZE];
    APP_TPRINTF( "drv_uart_event_task\r\n");
    APP_TPRINTF("--- drv evt ---Free heap memory: %d bytes------\r\n", xPortGetFreeHeapSize());
    DRV_TRANS_DATA_TYPE event;
    switch (port)
    {
    case 1:
    {
        pQueue = &uart1Queue;
        read_cb_func = &uart1_read_cb_func;
        APP_TPRINTF("uart[%d] event task running prio %d:\r\n", port,
                    DRV_UART_EVENT_TASK_PRI - (COM_PORT_IDX == 0 ? 0 : 1));
        break;
    }
    case 2:
    {
        pQueue = &uart2Queue;
        read_cb_func = &uart2_read_cb_func;
        APP_TPRINTF("uart[%d] event task running prio %d:\r\n", port,
                    DRV_UART_EVENT_TASK_PRI - (COM_PORT_IDX == 1 ? 0 : 1));
        break;
    }

    default:
    {
        APP_TPRINTF("Unknown port[%d],%s", port, pcTaskGetName(NULL));
        vTaskDelete(NULL);
        return;
    }
    }

    while (1)
    {
        // Waiting for UART event.
        if (pdPASS == xQueueReceive(*pQueue, (void *)&event, (portTickType)portMAX_DELAY))
        {
            switch (event.evt_id)
            {
            case DRV_EVT_RX_TO:
            case DRV_EVT_RX_DONE:
            {
                APP_TPRINTF( "[UART %d DATA]: %d\r\n", port, event.len);
                // Check if the read callback has been defined
                if (NULL == *read_cb_func)
                {
                    // No read callback defined
                    //APP_TPRINTF(" [UART %d ]drop %d bytes\r\n", port, event.len);

                #if(PORT_NUM == 2)
                    // If we have two ports, let's assume this data arrived on the
                    // 'data' port and send it to the radio if in TRNS TX mode
                    if ((event.len > 0) && (usr_cmd_is_trans_tx()))
                    {
                        APP_LOG(TS_ON,VLEVEL_M,"trans=%d \r\n", event.len);
                        pub_rtcm(event.buf, event.len);
                    }
                #endif
                }
                else
                {
                    // If the data length is > zero, send the data to the callback
                    if (event.len > 0)
                    {
                        (*read_cb_func)(event.buf, event.len);
                    }
                }
            }
            break;

            case DRV_EVT_TX_EMPTY:
            case DRV_EVT_TX_DONE:
            {
                ;
            }
            break;

            // Others
            default:
            {
                APP_TPRINTF("uart %d event type: %d\r\n", port, event.evt_id);
            }
            break;
            }
        }
        //taskYIELD();
    }

    vTaskDelete(NULL);
}

void com2_init()
{
    MX_DMA_Init();
    MX_USART2_UART_Init();

    /* Make sure that no UART transfer is on-going */
    // while (__HAL_UART_GET_FLAG(&huart2, USART_ISR_BUSY) == SET)
    //     ;

    // /* Make sure that UART is ready to receive)   */
    // while (__HAL_UART_GET_FLAG(&huart2, USART_ISR_REACK) == RESET)
        // ;
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_27);
}

 void com2_deinit()
{
    /* USER CODE BEGIN vcom_DeInit_1 */

    /* USER CODE END vcom_DeInit_1 */
    /* ##-1- Reset peripherals ################################################## */
    __HAL_RCC_USART2_FORCE_RESET();
    __HAL_RCC_USART2_RELEASE_RESET();

    /* ##-2- MspDeInit ################################################## */
    HAL_UART_MspDeInit(&huart2);

    /* ##-3- Disable the NVIC for DMA ########################################### */
    /* USER CODE BEGIN 1 */
    HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);
}

void com2_send_block(uint8_t *p_data, uint16_t size)
{
    HAL_UART_Transmit(&huart2, p_data, size, 1000);
}

void com2_send_DMA(uint8_t *p_data, uint16_t size)
{
    /* USER CODE BEGIN vcom_Trace_DMA_1 */

    /* USER CODE END vcom_Trace_DMA_1 */
    HAL_UART_Transmit_DMA(&huart2, p_data, size);
    /* USER CODE BEGIN vcom_Trace_DMA_2 */

    /* USER CODE END vcom_Trace_DMA_2 */
}

void com1_init()
{
    MX_DMA_Init();
    MX_USART1_UART_Init();

    // /* Make sure that no UART transfer is on-going */
    // while (__HAL_UART_GET_FLAG(&huart1, USART_ISR_BUSY) == SET)
    //     ;

    // /* Make sure that UART is ready to receive)   */
    // while (__HAL_UART_GET_FLAG(&huart1, USART_ISR_REACK) == RESET)
    //     ;
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_26);
}

 void com1_deinit()
{
    /* USER CODE BEGIN vcom_DeInit_1 */

    /* USER CODE END vcom_DeInit_1 */
    /* ##-1- Reset peripherals ################################################## */
    __HAL_RCC_USART1_FORCE_RESET();
    __HAL_RCC_USART1_RELEASE_RESET();

    /* ##-2- MspDeInit ################################################## */
    HAL_UART_MspDeInit(&huart1);

    /* ##-3- Disable the NVIC for DMA ########################################### */
    /* USER CODE BEGIN 1 */
    HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
}

void com1_send_block(uint8_t *p_data, uint16_t size)
{
    HAL_UART_Transmit(&huart1, p_data, size, 1000);
}

void com1_send_DMA(uint8_t *p_data, uint16_t size)
{
    /* USER CODE BEGIN vcom_Trace_DMA_1 */

    /* USER CODE END vcom_Trace_DMA_1 */
    HAL_UART_Transmit_DMA(&huart1, p_data, size);
    /* USER CODE BEGIN vcom_Trace_DMA_2 */

    /* USER CODE END vcom_Trace_DMA_2 */
}

static const int port1 = 1;
int drv_uart_com1_init(void)
{
    //const int cmdPort = CMD_PORT;
    //BaseType_t xret;

    uart1Queue = xQueueCreate(UART_QUEUE_NUM, DRV_TRANS_ITEM_SIZE);
    send_uart1_event = xEventGroupCreate();

    com1_init();
    xEventGroupSetBits(send_uart1_event, UART1_SEND_CPLT);
    //xret = xTaskCreate(drv_uart_event_task, "uart_com1", (1024 * 3), (void *)&cmdPort, DRV_UART_CMD_EVENT_TASK_PRI,
    //                   NULL);
    // TODO: figure out why xTaskCreate stops UART1 from sending...
    // xTaskCreate(drv_uart_event_task, "uart_com1", 1024 * 4, (void *)&port1,
    //             DRV_UART_EVENT_TASK_PRI - (COM_PORT_IDX == 0 ? 0 : 1),
    //             NULL);
    HAL_UART_RegisterCallback(&huart1, HAL_UART_TX_COMPLETE_CB_ID,
                              UART_TxCpltCallback);
    HAL_UART_RegisterRxEventCallback(&huart1, UART_RxEventCallback);
    HAL_UART_RegisterCallback(&huart1, HAL_UART_ERROR_CB_ID, UART_ErrorCallback);

    start_uart_rx_irq(&huart1);
    
    APP_TPRINTF( "--- cmd init ---Free heap memory: %d bytes------\r\n", xPortGetFreeHeapSize());
    return 0;
}

static const int port2 = 2;
int drv_uart_com2_init(void)
{
    //const int dataPort = DATA_PORT;

    uart2Queue = xQueueCreate(UART_QUEUE_NUM, DRV_TRANS_ITEM_SIZE);
    send_uart2_event = xEventGroupCreate();
    
    com2_init();
    xEventGroupSetBits(send_uart2_event, UART2_SEND_CPLT);
    //xTaskCreate(drv_uart_event_task, "uart_com2", 1024 * 3, (void *)&dataPort, DRV_UART_DATA_EVENT_TASK_PRI,
    //            NULL); 
    xTaskCreate(drv_uart_event_task, "uart_com2", 1024 * 4, (void *)&port2,
                DRV_UART_EVENT_TASK_PRI - (COM_PORT_IDX == 1 ? 0 : 1),
                NULL); 
    HAL_UART_RegisterCallback(&huart2, HAL_UART_TX_COMPLETE_CB_ID,
                              UART_TxCpltCallback);
    HAL_UART_RegisterRxEventCallback(&huart2, UART_RxEventCallback);
    HAL_UART_RegisterCallback(&huart2, HAL_UART_ERROR_CB_ID, UART_ErrorCallback);
    
    start_uart_rx_irq(&huart2);
    
    APP_TPRINTF( "--- cmd init ---Free heap memory: %d bytes------\r\n", xPortGetFreeHeapSize());
    //drv_printf("--- cmd init ---Free heap memory: %d bytes------\r\n", xPortGetFreeHeapSize());
    return 0;
}

int drv_uart_com1_clear_rx(void)
{
    xQueueReset(uart1Queue);

    return 0;
}

int drv_uart_com2_clear_rx(void)
{
    xQueueReset(uart2Queue);

    return 0;
}

int drv_uart_com1_read_set_cb(const drv_uart_read_cb_t cb)
{
    uart1_read_cb_func = cb;
    return 0;
}

// send via irq
int drv_uart_com1_send(const uint8_t *data, const uint16_t len)
{
    const TickType_t xTicksToWait = pdMS_TO_TICKS(MAX_SEND_WAIT);
    EventBits_t uxBits;

    uxBits = xEventGroupWaitBits(
        send_uart1_event, /* The event group being tested. */
        UART1_SEND_CPLT,
        pdTRUE,
        pdFALSE,
        xTicksToWait);

    int ret = -1;
    if (HAL_OK == HAL_UART_Transmit_IT(&huart1, data, len))
    {
        ret = 0;
    }

    return ret;
}

int drv_uart_com2_read_set_cb(const drv_uart_read_cb_t cb)
{
    uart2_read_cb_func = cb;
    return 0;
}

int drv_uart_com2_send(const uint8_t *data, const uint32_t len)
{
    const TickType_t xTicksToWait = pdMS_TO_TICKS(MAX_SEND_WAIT);
    EventBits_t uxBits;

    uxBits = xEventGroupWaitBits(
        send_uart2_event, /* The event group being tested. */
        UART2_SEND_CPLT,
        pdTRUE,
        pdFALSE,
        xTicksToWait);

    int ret = -1;
    if (HAL_OK == HAL_UART_Transmit_IT(&huart2, data, len))
    {
        ret = 0;
    }

    return ret;
}

static void tiny_snprintf_like(char *buf, uint32_t maxsize, const char *strFormat, ...)
{
  /* USER CODE BEGIN tiny_snprintf_like_1 */

  /* USER CODE END tiny_snprintf_like_1 */
  va_list vaArgs;
  va_start(vaArgs, strFormat);
  UTIL_ADV_TRACE_VSNPRINTF(buf, maxsize, strFormat, vaArgs);
  va_end(vaArgs);
  /* USER CODE BEGIN tiny_snprintf_like_2 */

  /* USER CODE END tiny_snprintf_like_2 */
}
static void TimestampNow(uint8_t *buff, uint16_t *size)
{
	#define MAX_TS_SIZE (int) 16
  /* USER CODE BEGIN TimestampNow_1 */

  /* USER CODE END TimestampNow_1 */
  SysTime_t curtime = SysTimeGet();
  tiny_snprintf_like((char *)buff, MAX_TS_SIZE, "%ds%03d:", curtime.Seconds, curtime.SubSeconds);
  *size = strlen((char *)buff);
  /* USER CODE BEGIN TimestampNow_2 */

  /* USER CODE END TimestampNow_2 */
}

#define LOG_BUF_SIZE (256)
static uint8_t v_buf[LOG_BUF_SIZE];
int drv_printf(const char *strFormat, ...)
{
  va_list vaArgs;
  
  uint16_t timestamp_size = 0u;
//  uint16_t writepos = 0;
//  uint16_t len =0;;
  uint16_t buff_size;

  TimestampNow(v_buf,&timestamp_size);

  va_start( vaArgs, strFormat);

  buff_size =(uint16_t)UTIL_ADV_TRACE_VSNPRINTF((char *)(v_buf+timestamp_size),LOG_BUF_SIZE, strFormat, vaArgs);
  va_end(vaArgs); 
  buff_size += timestamp_size;

  send_cmd_rsp(v_buf,buff_size);

  return  buff_size;
}
