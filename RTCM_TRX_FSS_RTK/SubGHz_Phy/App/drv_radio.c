
#include "main.h"
#include "cmsis_os.h"
#include "queue.h"
#include "event_groups.h"

#include "platform.h"
#include "sys_app.h"
#include "subghz_phy_app.h"

/* USER CODE BEGIN Includes */
#include "stm32_timer.h"
#include "utilities_def.h"
#include "app_version.h"
#include "subghz_phy_version.h"
#include "app_common.h"
#include "radio.h"
#include "sys_app.h"
#include "radio_driver.h"

#include "drv_radio.h"
#include "drv_uart.h"
#include "ring_buffer.h"
#include "usr_cmd.h"
#include <string.h>
#include <stdlib.h>
#include "drv_flash.h"
#include "rtcm_crc.h"
static RADIO_ATTR s_radio_attr UTIL_MEM_ALIGN(8);
static RADIO_ATTR s_radio_attr_flash UTIL_MEM_ALIGN(8);
static RADIO_INFO s_radio_info;
static CFIFO  tx_fifo;
#define MAX_SEND_SIZE  2048
#define MAX_DECODE_LEN  2048
#define MAX_APP_BUFFER_SIZE 255 
static uint8_t s_data_buffer[MAX_DECODE_LEN] = {0};
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */
typedef enum
{
	UHF_EVT_RX_DONE,
	UHF_EVT_RX_TO,
	UHF_EVT_TX_DONE,
	UHF_EVT_TX_TO,
	UHF_EVT_RX_ERR
} UHF_EVT;

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
static QueueHandle_t pullMsgHandler = NULL;
static QueueHandle_t pubMsgHandler = NULL;
static EventGroupHandle_t rtcmEvent = NULL;
static int idx = 0;
#define RADIO_EVT_SEND_DONE (1 << 0)
#define RADIO_EVT_SEND_TO (1 << 1)
#define RADIO_EVT_RCV_DONE (1 << 2)
#define RADIO_EVT_RCV_TO (1 << 3)
#define RADIO_EVT_RCV_ERR (1 << 4)

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Configurations */
/*Timeout*/
#define TX_TIMEOUT_VALUE 10000
#define RX_TIMEOUT_VALUE 2000
#define RX_TIMEOUT_FHSS_ONE_HOP  200 // max wait one hop finished in 250ms
/* Definitions */
#define RX_CONTINUOUS_ON 1
#define RADIO_TX 0 /* do not change*/
#define RADIO_RX 1 /* do not change*/

#define PAK_DTA_SIZ 601

/* wait for remote to be in Rx, before sending a Tx frame*/
#define RX_TIME_MARGIN 200
/* LED blink Period*/
#define LED_PERIOD_MS 400


typedef struct _DR_TRANS
{
	uint32_t evt_id;
	uint32_t len;
	uint8_t buf[MAX_APP_BUFFER_SIZE + 1];
} UHF_TRANS_DATA_TYPE;

#define MAX_TRANS_BUFFER_SIZE 128 // same as uart
typedef struct _DR_TX_TRANS
{
	uint32_t len;
	uint8_t data[MAX_TRANS_BUFFER_SIZE + 1];
} UHF_TX_DATA_TYPE;

#define UHF_TRANS_ITEM_SIZE (sizeof(UHF_TRANS_DATA_TYPE))
#define UHF_RX_QUEUE_NUM (10)
#define UHF_TX_QUEUE_NUM (20) //same as uart


#define FHSS_HOP_SUP 1
#define HOP_BW_KHZ   250 //250

#if(HOP_BW_KHZ == 250)
// US 902-928
#define MAX_CHANLE_NUM_US  26 // 26M for us
//#define US_START_FREQ_HZ     902000250
#define MAX_HOPING_SEQ_US 101 // 4*13 total 4*26 = 104
//#define US_END_FREQ_HZ	     927000750
#define MAX_HOPING_SEQ MAX_HOPING_SEQ_US

//#define FHSS_GRID_FEQ            250000 //250kHz
//#define FHSS_CHANLE_FEQ_STEP     1000000//1Mhz
//#define FHSS_CENTER_OFFSET       125000 // 125kHz
#define HOP_GROUP_NUM 10
#else // 500
// US 902-928
#define MAX_CHANLE_NUM_US  26 // 26M for us
//#define US_START_FREQ_HZ     902250000
#define MAX_HOPING_SEQ_US 50 // 4*13 total 2*26 -2(902.250 center_frq) = 50
//#define US_END_FREQ_HZ	     927750000
#define MAX_HOPING_SEQ MAX_HOPING_SEQ_US

//#define FHSS_GRID_FEQ            500000 //500kHz
//#define FHSS_CHANLE_FEQ_STEP     1000000//1Mhz
//#define FHSS_CENTER_OFFSET       250000 // 250kHz
#define HOP_GROUP_NUM  5
#endif
#define FHSS_MAX_HOP_IN_SECOND      10
// broadcast on center freq
typedef struct _FHSS_BR_HEAD
{
	uint32_t header_len; // not inclued crc
	uint32_t magic_value;// 0xfeedfeed
	uint32_t device_id; // from chip id LL_FLASH_GetUDN
	uint32_t packet_idx; //  fhss packet cnt
	uint8_t  group_id; // 0 -9
	uint8_t hopping_tbl_size; // max   10 for 1 second 
	int8_t hopping_table[FHSS_MAX_HOP_IN_SECOND]; //hopping_seq[0] is center feq
	uint32_t crc32;
}FHSS_SYNC_HEADER_T;

typedef struct __fhss_info
{
	uint32_t center_freq; //  125 [0.250) 375 [250,500) 625[500,750)  875[750-1000)
	uint32_t bandwidth;
	uint32_t datarate;
	uint8_t coderate;
	int hop_group;  // [0,9] for 250 ;[0-4] for 500
	int send_idx_in_second;//
	int next_hop_idx; // 10*hop_ground+ send_idx_in_second
	int skip_hop_seq;
	int8_t hop_seq[MAX_HOPING_SEQ]; //1-101//250khz remove 0-902.125 and 103-927.875 and center freq
									//1-50 // remove 902.250 and center_freq
}FHSS_INFO ;

static FHSS_INFO s_fhss_info;
static FHSS_SYNC_HEADER_T s_sync_header;

static void generate_hop_table(int8_t *tbl, int size,int8_t skip_seq);
static void generate_fhss_header(FHSS_SYNC_HEADER_T *sync);
static uint32_t get_hop_freq(int send_idx);
static void update_fhss_hop_freq(uint32_t hop_freq);
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Radio events function pointer */
static RadioEvents_t RadioEvents;
static int cfg_lora_tx_mode(uint32_t freq_hz,  uint32_t band_id, uint32_t sf,uint8_t cr,int8_t pwr,uint32_t tx_timout);
static int cfg_lora_rx_mode(uint32_t freq_hz,  uint32_t band_id, uint32_t sf,uint8_t cr,uint32_t tx_timout);

/* USER CODE BEGIN PV */
static __IO uint32_t RadioTxDone_flag = 0;
static __IO uint32_t RadioTxTimeout_flag = 0;
static __IO uint32_t RadioRxDone_flag = 0;
static __IO uint32_t RadioRxTimeout_flag = 0;
static __IO uint32_t RadioError_flag = 0;
static __IO int16_t last_rx_rssi = 0;
static __IO int8_t last_rx_cfo = 0;

// static uint8_t data_buffer[MAX_APP_BUFFER_SIZE];
// uint16_t data_offset = 0;
CFIFO read_fifo;
static __IO uint16_t payloadLen = PAYLOAD_LEN;
#if (TEST_MODE == RADIO_TX)
//static uint16_t payloadLenMax = MAX_APP_BUFFER_SIZE;
#endif /* TEST_MODE == RADIO_TX */

#if ((USE_MODEM_LORA == 0) && (USE_MODEM_FSK == 1))
static uint8_t syncword[] = {0xC1, 0x94, 0xC1};
#endif /* USE_MODEM_FSK */

uint32_t count_RxOk = 0;
uint32_t count_RxKo = 0;
uint32_t PER = 0;
// uint32_t totallen = PAK_DTA_SIZ;
uint32_t rtcm_pak_num = 0;

/* Private function prototypes
-----------------------------------------------*/
void generate_hop_table(int8_t *tbl, int size,int8_t skip_seq)
{
	int8_t tmp;
	int8_t array[104]={0};
	//APP_PRINTF("center seq:%d\r\n",skip_seq);
	//int8_t *array = malloc(size*sizeof(int8_t));
	for(int k = 1,j=0 ; k <= size; k++)
	{
		if(skip_seq == k) // skip fhss sync freq
		{
			APP_PRINTF("skip seq:%d\r\n",skip_seq);
			continue;
		}
		array[j++] = k;
	}
	srand(xTaskGetTickCount());
    for (int i = size-1; i > 0; i--) {
        int8_t x = rand()%i + 1;
        tmp = array[i];
        array[i] = array[x];
        array[x] = tmp;
    }
    if(tbl != NULL)
    {
    	memcpy((void*)(tbl),(void*)array,size);
    }

    for(int k = 0; k <size;)
    {
    	APP_PRINTF("group [%d] = [%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d] \r\n",k/10,
    			tbl[k],tbl[k+1],tbl[k+2],tbl[k+3],tbl[k+4],tbl[k+5],tbl[k+6],tbl[k+7],tbl[k+8],tbl[k+9]);
    	if(k%50 ==0)
    	{
    		OS_DELAY_MS(50);
    	}
    	k+=10;
    }
    APP_PRINTF("size=%d\r\n",size);
}

void show_hop_table(int8_t *array, int size)
{
	int8_t *tbl = array;
    for(int k = 0; k <size;)
    {

    	APP_PRINTF("group [%d] = [%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d] \r\n",k/10,
    			tbl[k],tbl[k+1],tbl[k+2],tbl[k+3],tbl[k+4],tbl[k+5],tbl[k+6],tbl[k+7],tbl[k+8],tbl[k+9]);
    	if(k%50 ==0)
    	{
    		OS_DELAY_MS(10);
    	}
    	k+=10;
    }
}

// maybe from irq
__IO int send_idx = 0;
static void OnTxDone(void)
{
	APP_LOG(TS_ON,VLEVEL_M,"OnTxDone:%d\n\r", send_idx);

	BaseType_t xHigherPriorityTaskWoken, xResult;
	xHigherPriorityTaskWoken = pdFALSE;
	xResult = xEventGroupSetBitsFromISR(
		rtcmEvent,			 /* The event group being updated. */
		RADIO_EVT_SEND_DONE, /* The bits being set. */
		&xHigherPriorityTaskWoken);
	if (xResult != pdFAIL)
	{
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	//tx idle
	Radio.Sleep();
}
static UHF_TRANS_DATA_TYPE data;
// RxDone used for lora mode
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{

	BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult;

	APP_LOG(TS_ON,VLEVEL_M,"OnRxDone sz=%d\n\r",size);
	APP_LOG(TS_ON,VLEVEL_M,"RssiValue=%d dBm, SnrValue=%ddB\n\r", rssi, LoraSnr_FskCfo);
	if(s_radio_attr.type == 1){
		// APP_LOG(TS_ON,VLEVEL_M,"lora rx here\r\n");
		if(size!=cfifo_write(&read_fifo,payload,size))
		{
			APP_TPRINTF("fifo full\r\n");
		}
	}
	last_rx_rssi = rssi;
	last_rx_cfo = LoraSnr_FskCfo;
	data.len = size;
	data.evt_id = RADIO_EVT_RCV_DONE;
	memcpy(data.buf, payload, size);
	if(s_radio_attr.fhss == 0x00) // fix freq
	{
		Radio.Rx(RX_TIMEOUT_VALUE);
	}
	else
	{
		APP_LOG(TS_ON,VLEVEL_M,"fhss group %d\r\n",
				s_sync_header.group_id);
	}
	if(s_radio_attr.fhss == 0x01)
	{
		xHigherPriorityTaskWoken = pdFALSE;
		xResult = xEventGroupSetBitsFromISR(
			rtcmEvent,			 /* The event group being updated. */
			RADIO_EVT_RCV_DONE, /* The bits being set. */
			&xHigherPriorityTaskWoken);
	}

	if (!xQueueIsQueueFullFromISR(pullMsgHandler))
	{
		xResult = xQueueSendToBackFromISR(pullMsgHandler, &data, &xHigherPriorityTaskWoken);
		if (xResult != pdFALSE)
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
}

static void OnTxTimeout(void)
{
	APP_LOG(TS_ON,VLEVEL_M,"OnTxTimeout\n\r");
	RadioTxTimeout_flag = 1;
}

static void OnRxTimeout(void)
{
	/* USER CODE BEGIN OnRxTimeout */
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	APP_LOG(TS_ON,VLEVEL_M,"OnRxTimeout\n\r");

	if(s_radio_attr.fhss == 0x00)
	{
		//Radio.Rx(RX_TIMEOUT_VALUE);
	}
	else
	{
//		Radio.Rx(RX_TIMEOUT_VALUE);
		APP_LOG(TS_ON,VLEVEL_M,"fhss group %d\r\n",
				s_sync_header.group_id);
	}

	RadioRxTimeout_flag = 1;
	if (rtcmEvent != NULL)
	{
		xEventGroupSetBitsFromISR(rtcmEvent, RADIO_EVT_RCV_TO, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

static void OnRxError(void)
{
	/* USER CODE BEGIN OnRxError */
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	APP_LOG(TS_ON,VLEVEL_M,"OnRxError\n\r");
	RadioError_flag = 1;
	if(s_radio_attr.fhss == 0x00)
	{
		Radio.Rx(RX_TIMEOUT_VALUE);
	}
	else
	{
		APP_LOG(TS_ON,VLEVEL_M,"fhss group %d\r\n",
				s_sync_header.group_id);
	}
	if (rtcmEvent != NULL)
	{
		xEventGroupSetBitsFromISR(rtcmEvent, RADIO_EVT_RCV_ERR, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

RADIO_ATTR *radio_get_cur_param(void)
{
	return &s_radio_attr;
}

RADIO_INFO *radio_get_sys_param(void)
{
	return &s_radio_info;
}

static uint32_t radio_hal_init(void)
{
	/* Get SubGHY_Phy APP version*/
	APP_LOG(TS_OFF, VLEVEL_M, "HAL APPLICATION_VERSION: V%X.%X.%X\r\n",
			(uint8_t)(APP_VERSION_MAIN),
			(uint8_t)(APP_VERSION_SUB1),
			(uint8_t)(APP_VERSION_SUB2));

	/* Get MW SubGhz_Phy info */
	APP_LOG(TS_OFF, VLEVEL_M, "HAL MW_RADIO_VERSION:    V%X.%X.%X\r\n",
			(uint8_t)(SUBGHZ_PHY_VERSION_MAIN),
			(uint8_t)(SUBGHZ_PHY_VERSION_SUB1),
			(uint8_t)(SUBGHZ_PHY_VERSION_SUB2));
	/* USER CODE END SubghzApp_Init_1 */

	/* Radio initialization */
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.RxDone = OnRxDone;
	RadioEvents.TxTimeout = OnTxTimeout;
	RadioEvents.RxTimeout = OnRxTimeout;
	RadioEvents.RxError = OnRxError;

	Radio.Init(&RadioEvents);

	return 0;
}
// for lora tx
static int cfg_lora_tx_mode(uint32_t freq_hz,  uint32_t band_id, uint32_t sf,uint8_t cr,int8_t pwr,uint32_t tx_timout)
{
	/* Radio Set frequency */
	Radio.SetChannel(freq_hz); // depende on

	/*lora modulation*/
	Radio.SetTxConfig(MODEM_LORA, pwr, 0, band_id,
					  sf, cr,
					  LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
					  true, 0, 0, LORA_IQ_INVERSION_ON, tx_timout);

	Radio.SetMaxPayloadLength(MODEM_LORA, MAX_APP_BUFFER_SIZE);

	APP_LOG(TS_ON,VLEVEL_M,"tx feq:%d band_id=%d", freq_hz,band_id);
	return 0;
}
// for lora rx
static int cfg_lora_rx_mode(uint32_t freq_hz, uint32_t band_id,uint32_t sf,uint8_t cr,uint32_t rx_timout)
{
	Radio.SetChannel(freq_hz); // depende on
	/* RX Continuous */
	Radio.SetRxConfig(MODEM_LORA, band_id, sf,
					  cr, 0, LORA_PREAMBLE_LENGTH,
					  LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
					  0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

	Radio.SetMaxPayloadLength(MODEM_LORA, MAX_APP_BUFFER_SIZE);
	Radio.Rx(rx_timout);
	APP_LOG(TS_ON,VLEVEL_M, "rx feq:%d band_id:%d", freq_hz,band_id);
	return 0;
}

#define FHSS_START_FREQ 	902000000
#define FHSS_STOP_FREQ  	928000000
#if(HOP_BW_KHZ == 250)
#define FHSS_FIRST_FREQ_KHZ 902125000
#else
#define FHSS_FIRST_FREQ_KHZ 902250000
#endif

static bool  is_fhss_need(uint32_t freq_hz)
{
	if(freq_hz > FHSS_START_FREQ && freq_hz < FHSS_STOP_FREQ)
	{
		return true;
	}
	else
	{
		return false;
	}
}

static uint32_t get_center_freq_base_user(uint32_t freq_hz)
{
	uint32_t tmp,k_hz;
	tmp = freq_hz/1000;// to kHz
	k_hz = tmp%1000;

	uint32_t offset = 0;
#if(HOP_BW_KHZ == 250)
	//125khz [0.250) 375k [250,500) 625k[500,750)  875k[750-1000)
	if(k_hz >=0 &&k_hz < 250)
	{
		offset = 125;
	}
	else if(k_hz >= 250 && k_hz <500)
	{
		offset = 375;
	}
	else if(k_hz >= 500 && k_hz <750)
	{
		offset = 625;
	}
	else if(k_hz >= 750 && k_hz <1000)
	{
		offset = 875;
	}
#else
	//250khz [0.500)  750k[500-1000)
	if(k_hz >=0 &&k_hz < 500)
	{
		offset = 250;
	}
	else if(k_hz >= 500 && k_hz <1000)
	{
		offset = 750;
	}
#endif
	tmp = (tmp+offset)*1000; // to hz

	return tmp;
}

static void generate_fhss_header(FHSS_SYNC_HEADER_T *sync)
{
	static uint32_t packet_idx = 0;
	packet_idx++;
	sync->header_len = sizeof(FHSS_SYNC_HEADER_T) - sizeof(sync->crc32);
	sync->hopping_tbl_size = FHSS_MAX_HOP_IN_SECOND;
	sync->packet_idx = packet_idx;
	sync->magic_value = 0xfeedfeed;
	sync->group_id = s_fhss_info.hop_group;

	memcpy(&sync->hopping_table[0],&s_fhss_info.hop_seq[s_fhss_info.hop_group*FHSS_MAX_HOP_IN_SECOND],
			FHSS_MAX_HOP_IN_SECOND);

	//swap half group for next used
	memcpy(&s_fhss_info.hop_seq[s_fhss_info.hop_group*FHSS_MAX_HOP_IN_SECOND],
			&sync->hopping_table[FHSS_MAX_HOP_IN_SECOND/2],FHSS_MAX_HOP_IN_SECOND/2);

	memcpy(&s_fhss_info.hop_seq[s_fhss_info.hop_group*FHSS_MAX_HOP_IN_SECOND + FHSS_MAX_HOP_IN_SECOND/2],
			&sync->hopping_table[0],FHSS_MAX_HOP_IN_SECOND/2);

	sync->crc32 = rtk_crc24q((uint8_t*)sync,sync->header_len);
}

static int8_t  get_skip_hop_seq(uint32_t center_freq)
{
	int8_t tmp = 0;
	uint32_t k_hz = (center_freq-FHSS_START_FREQ)/1000;
#if(HOP_BW_KHZ == 250)
	tmp = k_hz/250;
#else
	tmp = k_hz/500;
#endif
	return tmp;
}

static uint32_t get_hop_freq(int send_idx)
{
	int  hop_seq = s_sync_header.hopping_table[send_idx];
#if(HOP_BW_KHZ == 250)
	uint32_t freq  = FHSS_FIRST_FREQ_KHZ + hop_seq*250000; // to hz
#else
	uint32_t freq  = FHSS_FIRST_FREQ_KHZ + hop_seq*500000; // to hz
#endif
	APP_LOG(TS_ON,VLEVEL_M,"send_idx %d hop_seq %d freq:%d\r\n",send_idx,hop_seq,freq);
	return freq;
}

typedef struct
{
	uint32_t bps;
	RadioLoRaBandwidths_t band;
	RadioLoRaSpreadingFactors_t sf;
	RadioLoRaCodingRates_t cr;
} RADIO_BPS_LIST;

#if(FHSS_HOP_SUP == 1)
#define BPS_NUM 3
#if(BPS_NUM ==2)
const RADIO_BPS_LIST BPS_TBL[BPS_NUM]=
{
	{19200,1 , LORA_SF6  , LORA_CR_4_5},  // Lora 19.2 kbps 18.75 BW=250khz,
	{38400,1 , LORA_SF5  , LORA_CR_4_5},  /* Lora 38.4 kbps 31.25 BW=250khz*/
//	{19200,2 , LORA_SF7  , LORA_CR_4_6},  /* Lora 19.2 kbps LORA_CR_4_6 LORA_CR_4_5 */
//	{38400,2 , LORA_SF5  , LORA_CR_4_8},  /* Lora 38.4 kbps */
//	{62500,2 , LORA_SF5  , LORA_CR_4_5},  /* Lora 62.5 kbps */
};
#else
const RADIO_BPS_LIST BPS_TBL[BPS_NUM]=
{
//	{19200,1 , LORA_SF6  , LORA_CR_4_5},  // Lora 19.2 kbps 18.75 BW=250khz,
//	{38400,1 , LORA_SF5  , LORA_CR_4_5},  /* Lora 38.4 kbps 31.25 BW=250khz*/
	{19200,2 , LORA_SF7  , LORA_CR_4_6},  /* Lora 19.2 kbps LORA_CR_4_6 LORA_CR_4_5 */
	{38400,2 , LORA_SF5  , LORA_CR_4_8},  /* Lora 38.4 kbps */
	{62500,2 , LORA_SF5  , LORA_CR_4_5},  /* Lora 62.5 kbps */
};
#endif
#else
#define BPS_NUM  3
const RADIO_BPS_LIST BPS_TBL[BPS_NUM]=
{
	{19200,2 , LORA_SF7  , LORA_CR_4_6},  /* Lora 19.2 kbps LORA_CR_4_6 LORA_CR_4_5 */
	{38400,2 , LORA_SF5  , LORA_CR_4_8},  /* Lora 38.4 kbps */
	{62500,2 , LORA_SF5  , LORA_CR_4_5},  /* Lora 62.5 kbps */
};
#endif
//BW=125kHz,R b=7��(125000/2 7 )��(4/(4+1))=5.5kbps
//BW=250kHz,R b=7��(250000/2 7)��(4/(4+1))=10.9kbps
//BW=500kHz,R b = 7 �� ( 500000 / 2 7 ) �� ( 4 / ( 4 + 1 ) ) = 21.9 k b p s

static bool is_diff_app_cfg()
{
	bool ret = false;
	if(s_radio_attr_flash.magic != s_radio_attr.magic
			||s_radio_attr_flash.mode != s_radio_attr.mode
			||s_radio_attr_flash.bps != s_radio_attr.bps
			||s_radio_attr_flash.freq[0] != s_radio_attr.freq[0]
			||s_radio_attr_flash.freq[1] != s_radio_attr.freq[1]
			||s_radio_attr_flash.wlbaud[0] != s_radio_attr.wlbaud[0]
			||s_radio_attr_flash.wlbaud[1] != s_radio_attr.wlbaud[1]
		    ||s_radio_attr_flash.power_level != s_radio_attr.power_level)
	{
		ret = true;
	}

	return ret;
}

uint32_t radio_param_cfg(void)
{
	//	s_radio_attr

	uint32_t sf = LORA_SF5;//LORA_SF5;
#if(FHSS_HOP_SUP == 1)
	uint32_t band_id = LORA_BANDWIDTH;
	uint32_t cr = LORA_CR_4_5;//LORA_CR_4_5
#else
	uint32_t cr = LORA_CR_4_8;//LORA_CR_4_8
	uint32_t band_id = 2; //500
#endif
	uint32_t bps =s_radio_attr.bps; // defaut 38400
	int hit_id = -1;

	for(int i = 0; i< BPS_NUM;i++)
	{
		if(BPS_TBL[i].bps == bps)
		{
			hit_id = i;
			break;
		}
	}
	if(hit_id != -1)
	{
		//
		band_id = BPS_TBL[hit_id].band;
		sf = BPS_TBL[hit_id].sf;
		cr =  BPS_TBL[hit_id].cr;
	}
	else
	{
		APP_LOG(TS_ON, VLEVEL_M,"use default 38.4k\r\n");
		s_radio_attr.bps = 38400;
	}

	// lora mode
	if(s_radio_attr.type == 0 || s_radio_attr.prot[1] == PROT_LORA )
	{
		uint32_t freq = 0;
		if (s_radio_attr.mode == RADIO_MODE_TX)
		{
			freq = s_radio_attr.freq[RADIO_MODE_TX];
			APP_LOG(TS_ON, VLEVEL_M,"tx_frq:%d,band:%d,sf:%d,cr:%d,level:%d\r\n",\
					freq,band_id,sf,cr,s_radio_attr.power_level);
			char buf[32];
			sprintf(buf,"level:%lu dbm\r\n",s_radio_attr.power_level);
			send_cmd_rsp((uint8_t*)&buf[0],strlen(buf));
			cfg_lora_tx_mode(freq,band_id, sf,cr,s_radio_attr.power_level,TX_TIMEOUT_VALUE);
		}

		if (s_radio_attr.mode == RADIO_MODE_RX)
		{
      		freq = s_radio_attr.freq[RADIO_MODE_RX];
			cfg_lora_rx_mode(freq,band_id,sf,cr,RX_TIMEOUT_VALUE);
		}

		//for hop 
#if(FHSS_HOP_SUP == 1)
		if(is_fhss_need(freq) == true)
		{
			s_radio_attr.fhss = 0x01;
			// get seq_seq
			s_fhss_info.center_freq = get_center_freq_base_user(freq);
			s_fhss_info.bandwidth = band_id;
			s_fhss_info.datarate = sf;
			s_fhss_info.coderate = cr;
			s_fhss_info.hop_group = 0;
			s_fhss_info.send_idx_in_second = 0;
			s_fhss_info.next_hop_idx = 0;
			s_fhss_info.skip_hop_seq = get_skip_hop_seq(freq);
			//Radio.SetChannel(s_fhss_info.center_freq);
			APP_LOG(TS_ON, VLEVEL_M,"center:%d hz",s_fhss_info.center_freq);
			update_fhss_hop_freq(s_fhss_info.center_freq);
			//build table one time
			generate_hop_table(&s_fhss_info.hop_seq[0],sizeof(s_fhss_info.hop_seq),s_fhss_info.skip_hop_seq);
		}
		else
		{
			s_radio_attr.fhss = 0x00;
		}
#endif
		}
	//diff cfg values
	if(is_diff_app_cfg())
	{
		APP_LOG(TS_ON, VLEVEL_M,"diff app cfg\r\n");
		s_radio_attr_flash = s_radio_attr;
		APP_LOG(TS_ON, VLEVEL_M,"attr_flash: magic %x mode %d bps %d freq %d %d wlbaud %d %d level %d \r\n",
				s_radio_attr_flash.magic,s_radio_attr_flash.mode,
				s_radio_attr_flash.bps,s_radio_attr_flash.freq[0],s_radio_attr_flash.freq[1],
				s_radio_attr_flash.wlbaud[0],s_radio_attr_flash.wlbaud[1],s_radio_attr_flash.power_level);

		STMFLASH_Write(STM32_FLASH_APPCFG_BASE,(u64*)&s_radio_attr_flash,sizeof(s_radio_attr_flash)/8);
	}

	return 0;
}


static void update_fhss_hop_freq(uint32_t hop_freq)
{
	APP_LOG(TS_ON,VLEVEL_M,"fhss:%d freq:%d\r\n",s_radio_attr.fhss,hop_freq);
	if(s_radio_attr.fhss == 0x01)
	{
		// for tx
		if (s_radio_attr.mode == RADIO_MODE_TX)
		{
			APP_LOG(TS_ON,VLEVEL_M,"tx_freq:%d\r\n",hop_freq);
			Radio.SetChannel(hop_freq);
			Radio.SetTxConfig(MODEM_LORA, s_radio_attr.power_level, 0, s_fhss_info.bandwidth,
							  s_fhss_info.datarate, s_fhss_info.coderate,
							  LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
							  true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);
		}
		// for rx
		if (s_radio_attr.mode == RADIO_MODE_RX)
		{
			APP_LOG(TS_ON,VLEVEL_M,"rx_freq:%d\r\n",hop_freq);
			Radio.SetChannel(hop_freq);
			Radio.SetRxConfig(MODEM_LORA,  s_fhss_info.bandwidth, s_fhss_info.datarate,
							  s_fhss_info.coderate, 0, LORA_PREAMBLE_LENGTH,
							  LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
							  0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
		}
	}
}

static void  flash_led_hdlr(TimerHandle_t xTimer)
{
	//off led
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_RESET);
	xTimerStop(xTimer,0);
}
static TimerHandle_t led_timer = NULL;


static void OnledEvent(void *context)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_RESET);
}

/* Led Timers objects*/
static  UTIL_TIMER_Object_t timerLed;
void init_led()
{
  led_timer = xTimerCreate(NULL,  (900)/portTICK_RATE_MS, pdFALSE,NULL, flash_led_hdlr);
  /* Led Timers*/
  UTIL_TIMER_Create(&timerLed, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnledEvent, NULL);
  UTIL_TIMER_SetPeriod(&timerLed, LED_PERIOD_MS);
  //UTIL_TIMER_Start(&timerLed);
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_RESET);
}

void flash_led()
{
	if(xTimerIsTimerActive(led_timer) == pdFALSE)
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_SET);
		UTIL_TIMER_Start(&timerLed);
		xTimerStart(led_timer,0);
	}
}
#define MAX_RADIO_WAIT 50
static EventBits_t wait_radio_event(EventBits_t evt, int wait_ms)
{
	const TickType_t xTicksToWait = pdMS_TO_TICKS(MAX_RADIO_WAIT);
	EventBits_t uxBits;

	uxBits = xEventGroupWaitBits(
		rtcmEvent, /* The event group being tested. */
		evt,
		pdTRUE,
		pdFALSE,
		xTicksToWait);

	return uxBits;
}

static void pull_rtcm_to_uart_handler(void *arg)
{
	UHF_TRANS_DATA_TYPE msg;
	//uint32_t RxFIFO_size;
	uint32_t crc32;
	uint32_t msg_crc;
	uint32_t hop_freq = 0;
	bool  got_fhss_head = false;
	int rcv_idx_in_second = 0;
	int next_hop_seq = 0;
	//int ret = 0;

	OS_DELAY_MS(400);
	if(s_radio_attr.mode != RADIO_MODE_END)
	{
		APP_LOG(TS_ON,VLEVEL_M," init raido with app_cfg ---Free heap memory: %d bytes\r\n",xPortGetFreeHeapSize());
		init_app_cfg_to_radio();
	}
	TickType_t wait_tick = portMAX_DELAY;
#if(FHSS_HOP_SUP == 1)
	wait_tick = pdMS_TO_TICKS(300);
#endif
	while (1)
	{
		if ((s_radio_attr.inited == 0x00) || s_radio_attr.switching || (s_radio_attr.mode != RADIO_MODE_RX))
		{
			if(uxQueueSpacesAvailable (pullMsgHandler) != UHF_RX_QUEUE_NUM)
			{
				xQueueReset(pullMsgHandler);
			}
			OS_DELAY_MS(100);
			continue;
		}
		// RX MODE
		// from uhf rx inject portMAX_DELAY
		if (pdFAIL != xQueueReceive(pullMsgHandler, &msg, wait_tick))
		{
			if (msg.len > 0)
			{

#if(FHSS_HOP_SUP == 1)
				//check fhss sync info
				if(s_radio_attr.fhss == 0x01)
				{
					if(got_fhss_head == false && msg.len == sizeof(s_sync_header))
					{
						FHSS_SYNC_HEADER_T * fhss_head = (FHSS_SYNC_HEADER_T * )&msg.buf[0];
						if(fhss_head->magic_value == 0xfeedfeed
								&& (fhss_head->header_len == (sizeof(FHSS_SYNC_HEADER_T) - sizeof(uint32_t))))
						{
							// clear evt for next rx
							xEventGroupClearBits(
									rtcmEvent,
									RADIO_EVT_RCV_DONE|RADIO_EVT_RCV_TO|RADIO_EVT_RCV_ERR);
							crc32 = rtk_crc24q((uint8_t*)msg.buf,fhss_head->header_len);
							memcpy(&msg_crc,msg.buf+fhss_head->header_len,sizeof(uint32_t));
							if(crc32 != msg_crc)
							{
								APP_LOG(TS_ON,VLEVEL_M,"not fhss head crc32[%08x] != msg_crc[%08x] \r\n",crc32,msg_crc);
								hop_freq = s_fhss_info.center_freq; // must get fhss header first
								s_sync_header = *fhss_head;
								//Radio.SetChannel(hop_freq);
								update_fhss_hop_freq(hop_freq);
								Radio.Rx(RX_TIMEOUT_VALUE);
								next_hop_seq = 0;
								rcv_idx_in_second = 0;
							}
							else
							{
								rcv_idx_in_second = 0;
								got_fhss_head = true;
								s_sync_header = *fhss_head;

								//got fhss header
								{
									//next_hop_seq = s_sync_header.next_hopping_seq;
									next_hop_seq = 0;
									hop_freq = get_hop_freq(next_hop_seq);
									APP_LOG(TS_ON,VLEVEL_M,"1.packet_idx:%d group %d  hop_freq:%d\r\n",
																s_sync_header.packet_idx,s_sync_header.group_id,hop_freq/1000);
									//Radio.SetChannel(hop_freq);
									update_fhss_hop_freq(hop_freq);
									Radio.Rx(RX_TIMEOUT_FHSS_ONE_HOP);
									APP_LOG(TS_ON,VLEVEL_M,"1.next %d freq [%d] \r\n",next_hop_seq,hop_freq/1000);
									rcv_idx_in_second++;
									next_hop_seq++;
									show_hop_table(&s_sync_header.hopping_table[0], s_sync_header.hopping_tbl_size);
								}
							}
							continue;
						}
					}

					if(got_fhss_head == true)
					{
						rcv_idx_in_second++;
						// check if last one, jump to center freq
						if(msg.len < 255)
						{
							APP_LOG(TS_ON,VLEVEL_M,"last item size[%d] in second\r\n",msg.len);
							// one packed finshed
							got_fhss_head = false;
							hop_freq = s_fhss_info.center_freq; // must get fhss header first
							memset((void*)&s_sync_header,0,sizeof(s_sync_header));
							//Radio.SetChannel(hop_freq);
							update_fhss_hop_freq(hop_freq);
							Radio.Rx(RX_TIMEOUT_VALUE);
						}
						else
						{
							hop_freq = get_hop_freq(next_hop_seq);
							APP_LOG(TS_ON,VLEVEL_M,"N.next %d freq [%d] \r\n",next_hop_seq,hop_freq/1000);
							next_hop_seq++;
							//Radio.SetChannel(hop_freq);
							if(next_hop_seq >= FHSS_MAX_HOP_IN_SECOND)
							{
								hop_freq = s_fhss_info.center_freq;
								update_fhss_hop_freq(hop_freq);
								Radio.Rx(RX_TIMEOUT_VALUE);
							}
							else
							{
								update_fhss_hop_freq(hop_freq);
								Radio.Rx(RX_TIMEOUT_FHSS_ONE_HOP);
							}

						}

					}

				}

#endif
				flash_led();
				APP_LOG(TS_ON,VLEVEL_M,"RX LEN=%d \r\n",msg.len);

				// todo 64 byte send
#define SPLIT_RTCM_ENABLE 1

#if (SPLIT_RTCM_ENABLE == 1)
#define SPLIT_SIZE  128
				uint32_t rtcm_len = msg.len;
				uint8_t  *rx_ptr = &msg.buf[0];
				for (int i = 0; i < rtcm_len; i += SPLIT_SIZE)
				{
					int length = MIN(SPLIT_SIZE,rtcm_len - i);
					rx_ptr += i;
					//memcpy(data.buf, uart1_rx_buf+i, length);
					//data.len = length;
#if(COM_PORT_IDX == 0)
					drv_uart_com2_send(rx_ptr, length);
#else
					drv_uart_com1_send(rx_ptr, length);
					if( i== 0) RADIO_DELAY_MS(5);
#endif
				}
#else
				#if(COM_PORT_IDX == 0)
					drv_uart_com1_send(msg.buf, msg.len);
				#else
					drv_uart_com2_send(msg.buf, msg.len);
				#endif
#endif
			}
		} // wait lora packet in pdMS_TO_TICKS(200)
		else
		{
			//timeout rx data
#if(FHSS_HOP_SUP == 1)
			if(s_radio_attr.fhss == 0x01)
			{
				//check rcv flags
				EventBits_t uxBits = wait_radio_event(RADIO_EVT_RCV_TO|RADIO_EVT_RCV_ERR, 0);
				if ((RADIO_EVT_RCV_TO|RADIO_EVT_RCV_ERR) & uxBits)
				{
					// timeout or error hanpended, not found fhss head or lost sub item
					// back to center freq
					memset((void*)&s_sync_header,0,sizeof(s_sync_header));
					hop_freq = s_fhss_info.center_freq;
					got_fhss_head = false;
					next_hop_seq = 0;
					rcv_idx_in_second = 0;
					//Radio.SetChannel(hop_freq);
					update_fhss_hop_freq(hop_freq);
					Radio.Rx(RX_TIMEOUT_VALUE);
					APP_LOG(TS_ON,VLEVEL_M,"no fhss packet\r\n ");
				}
			}
#endif
		 }
	}
	vTaskDelete(NULL);
}

int start_rtcm_trans(void)
{
	s_radio_attr.switching = true;
	s_radio_attr.stop_rtcm = 0x00;
	OS_DELAY_MS(200);
	s_radio_attr.switching = false;
	return 0;
}

int stop_rtcm_trans(void)
{
	s_radio_attr.switching = true;
	s_radio_attr.stop_rtcm = 0x01;
	OS_DELAY_MS(200);
	return 0;
}
static void uint8_to_hex(uint8_t num, uint8_t* hex_str)
{
    uint8_t nibble = (num >> 4) & 0x0F;

    if (nibble < 10)
    {
        *(hex_str++) = '0' + nibble;
    }
    else
    {
        *(hex_str++) = 'A' + (nibble - 10);
    }
    nibble = num & 0x0F;
    if (nibble < 10)
    {
        *(hex_str++) = '0' + nibble;
    }
    else
    {
        *(hex_str++) = 'A' + (nibble - 10);
    }
}
// hex_str = 2*str
void log_hex(uint8_t* str,uint16_t len,uint8_t* hex_str)
{
	uint8_t num;
	for(int i = 0 ; i < len; i++)
	{
		num = *str;
		uint8_to_hex(num,hex_str);
		str++;
		hex_str+=2;
	}
}


#define LORA_TEST 0
#define RTCM_TIME_OUT 200

static void pub_rtcm_msg_handle(void *arg)
{
	uint32_t msg_len = 0,fifo_sz = 0;
	uint8_t send_buf[MAX_APP_BUFFER_SIZE + 1] = {0};

	bool packet_begin = false; // every second send rtcm continue
	//int send_size = 0;
	uint32_t hop_freq = 0;
	BaseType_t xret;
	//int cur_group = 0;
#define WORKAROUND_FOR_PC_TX 0


	while (1)
	{
		if ((s_radio_attr.inited == 0x00) || s_radio_attr.switching || (s_radio_attr.mode != RADIO_MODE_TX))
		{
			if(uxQueueSpacesAvailable (pubMsgHandler) != UHF_TX_QUEUE_NUM)
			{
				xQueueReset(pubMsgHandler);
				cfifo_reset(&tx_fifo);
			}
			OS_DELAY_MS(100);
			continue;
		}
		if (RadioTxTimeout_flag == 1)
		{
			if (s_radio_attr.mode == RADIO_MODE_TX && s_radio_attr.type == 1)
			{
				radio_param_cfg();
				xEventGroupSetBits(rtcmEvent, RADIO_EVT_SEND_DONE);
				APP_LOG(TS_ON,VLEVEL_M, "RadioTxTimeout_flag:: reinit radio hal\r\n");
			}
			RadioTxTimeout_flag = 0;
			OS_DELAY_MS(20);
		}

		xret = xQueueReceive(pubMsgHandler, &msg_len, pdMS_TO_TICKS(50));
		//APP_LOG(TS_ON, VLEVEL_M, "pub_que=%d msg_len=%d\r\n", uxQueueSpacesAvailable(pubMsgHandler),msg_len);
		fifo_sz = cfifo_get_datanum(&tx_fifo);
		if(!s_radio_attr.stop_rtcm  && fifo_sz)
		{
#if(FHSS_HOP_SUP == 1)
			if(fifo_sz >0 && s_radio_attr.fhss == 0x01)
			{
				// handle for fhss header
				if(packet_begin == false)
				{
					//packet begin
					packet_begin = true;
					s_fhss_info.send_idx_in_second = 0;
					//cur_group = s_fhss_info.hop_group;

					generate_fhss_header(&s_sync_header);
					OS_DELAY_MS(10);
					show_hop_table(s_sync_header.hopping_table,FHSS_MAX_HOP_IN_SECOND);
					s_fhss_info.hop_group++;
					if(s_fhss_info.hop_group == HOP_GROUP_NUM)
					{
						s_fhss_info.hop_group = 0;
						//TODO: revert table
						//generate_hop_table(&s_fhss_info.hop_seq,sizeof(s_fhss_info.hop_seq),s_fhss_info.skip_hop_seq);
					}

#if(WORKAROUND_FOR_PC_TX == 1)
					OS_DELAY_MS(100);
#endif
					// broadcast fhss sync info
					hop_freq = s_fhss_info.center_freq;
					Radio.SetChannel(hop_freq);

					memcpy((void*)send_buf,(void*)&s_sync_header,sizeof(s_sync_header));

					send_idx = idx+1;
					// wait send cplt
					EventBits_t uxBits = wait_radio_event(RADIO_EVT_SEND_DONE, 100);
					// for fhss heade send
					if (RADIO_EVT_SEND_DONE == (RADIO_EVT_SEND_DONE & uxBits))
					{
						OS_DELAY_MS(10);
						Radio.Send((uint8_t *)send_buf, sizeof(s_sync_header));
						APP_LOG(TS_ON,VLEVEL_M,"0.send fhss head [%d] send_idx %d \r\n", sizeof(s_sync_header),send_idx);
						OS_DELAY_MS(80);
					}
					else
					{
						OS_DELAY_MS(50);
						Radio.Send((uint8_t *)send_buf, sizeof(s_sync_header));
						APP_LOG(TS_ON,VLEVEL_M,"0.delay send fhss head [%d] send_idx %d \r\n", sizeof(s_sync_header),send_idx);
					}

					idx++;
					fifo_sz = cfifo_get_datanum(&tx_fifo); //get fifo size after send fhss header
				}
			}
#endif
			if(fifo_sz >= MAX_APP_BUFFER_SIZE)
			{
				//rtcm raw
				cfifo_read(&tx_fifo,send_buf,MAX_APP_BUFFER_SIZE);
				flash_led();
				// wait send cplt
				EventBits_t uxBits = wait_radio_event(RADIO_EVT_SEND_DONE, RTCM_TIME_OUT);
#if(FHSS_HOP_SUP == 1)
				if(s_radio_attr.fhss ==1)
				{
					hop_freq = get_hop_freq(s_fhss_info.send_idx_in_second);
					Radio.SetChannel(hop_freq);
					APP_LOG(TS_ON,VLEVEL_M,"1.fhss_cnt:%d group %d next_hop %d  hop_freq:%d\r\n",
							s_sync_header.packet_idx,s_sync_header.group_id,s_fhss_info.send_idx_in_second,hop_freq/1000);
					s_fhss_info.send_idx_in_second++;
					//OS_DELAY_MS(10);
					if(s_fhss_info.send_idx_in_second >=HOP_GROUP_NUM)
					{
						APP_LOG(TS_ON,VLEVEL_M,"pub %d more than %d hop seq in on packet\r\n",s_fhss_info.send_idx_in_second,HOP_GROUP_NUM);
						drv_printf("pub %d item\r\n",s_fhss_info.send_idx_in_second);
						packet_begin = false; // send with new header
						continue;
					}
				}
#endif
				// for 255 send
				if (RADIO_EVT_SEND_DONE == (RADIO_EVT_SEND_DONE & uxBits))
				{
					send_idx = idx+1;
					APP_LOG(TS_ON,VLEVEL_M, "1.send_idx:%d==len:%d\r\n", send_idx, MAX_APP_BUFFER_SIZE);
					Radio.Send((uint8_t *)send_buf, MAX_APP_BUFFER_SIZE);
				}
				else
				{
					send_idx = idx+1;
					EventBits_t uxBits = wait_radio_event(RADIO_EVT_SEND_DONE, (RTCM_TIME_OUT/2));
					if (RADIO_EVT_SEND_DONE != (RADIO_EVT_SEND_DONE & uxBits))
					{
						Radio.Send((uint8_t *)send_buf, MAX_APP_BUFFER_SIZE);
					APP_LOG(TS_ON,VLEVEL_M, "2.delay 100ms send_idx:%d len:%d\r\n", send_idx, MAX_APP_BUFFER_SIZE);
					}
					else{
						Radio.Send((uint8_t *)send_buf, MAX_APP_BUFFER_SIZE);
						APP_LOG(TS_ON,VLEVEL_M, "2.send_idx:%d==len:%d\r\n", send_idx, MAX_APP_BUFFER_SIZE);
					}	

				}

				if(s_radio_attr.bps == 19200)
				{
					OS_DELAY_MS(120); // about 110ms for 255
				}
				else if(s_radio_attr.bps == 38400)
				{
					OS_DELAY_MS(80); // 70->100ms about 65
				}
				else if(s_radio_attr.bps == 62500)
				{
					OS_DELAY_MS(50); // 20->50 for band 500 support
				}
				else
				{
					OS_DELAY_MS(100);
				}
				idx++;

			}
			else if((false == xret) && (fifo_sz > 0 && fifo_sz < MAX_APP_BUFFER_SIZE))
			{
#if(FHSS_HOP_SUP == 1)
				if(s_radio_attr.fhss ==1)
				{
					//need check fake less 255
					//for fask less 255 case
					uint32_t last_sz =  fifo_sz;
					OS_DELAY_MS(35); // delay 35 ms
					fifo_sz = cfifo_get_datanum(&tx_fifo);
					if(fifo_sz > last_sz) //new data in
					{
						//fake less 255
						drv_printf("fake less: pre_sz %d cur_sz %d\r\n",last_sz,fifo_sz);
						continue;
					}
					hop_freq = get_hop_freq(s_fhss_info.send_idx_in_second);
					update_fhss_hop_freq(hop_freq);
					APP_LOG(TS_ON,VLEVEL_M,"3.fhss cnt:%d group %d s_idx %d hop_freq:%d\r\n",
							s_sync_header.packet_idx,s_sync_header.group_id,s_fhss_info.send_idx_in_second,hop_freq/1000);

					s_fhss_info.send_idx_in_second++;
					//OS_DELAY_MS(10);
				}
				packet_begin = false;
#endif
				cfifo_read(&tx_fifo,send_buf,fifo_sz);
				Radio.Send((uint8_t *)send_buf, fifo_sz);
				send_idx = idx+1;
				APP_LOG(TS_ON,VLEVEL_M, "3.last_send_idx:%d==len:%d\r\n", send_idx, fifo_sz)
				OS_DELAY_MS(80);
				idx++;
				}
			}
		else {
				Radio.Sleep();
				OS_DELAY_MS(100);
#define TEST_LORA_RFx 1
#ifdef TEST_LORA_RF
			memset(data_buffer,0xcc,255);
			pub_rtcm(data_buffer,255);
			pub_rtcm(data_buffer,255);

			memset(data_buffer,0x55,255);
			pub_rtcm(data_buffer,255);
			pub_rtcm(data_buffer,255);

			memset(data_buffer,0x33,255);
			pub_rtcm(data_buffer,255);
			pub_rtcm(data_buffer,255);

			memset(data_buffer,0xaa,255);
			pub_rtcm(data_buffer,255);
			pub_rtcm(data_buffer,150);
#endif
		}
	}

	vTaskDelete(NULL);
}

int pub_rtcm(uint8_t *data, const uint16_t len)
{

	//drv_printf("pub_rtcm:%d\r\n",len);
	if (s_radio_attr.inited && (!s_radio_attr.stop_rtcm)
		 && (s_radio_attr.mode == RADIO_MODE_TX))
	{
		if(len != cfifo_write(&tx_fifo,data,len))
		{
			APP_LOG(TS_ON,VLEVEL_M,"err tx_fifo_full\r\n");
		}
		uint32_t tx_len = len;
		if(pdFALSE == xQueueSend(pubMsgHandler, (void *)&tx_len,0))
		{
				APP_LOG(TS_ON,VLEVEL_M,"err tx_pub_queue_full\r\n");
				drv_printf("tx_pub_queue_full\r\n");
		}
	}
	// APP_LOG(TS_ON,VLEVEL_M,"exit len=%d\r\n", len);
	return 0;
}

uint32_t radio_init(void)
{
	BaseType_t xret;
	s_radio_attr.inited = 0x00;
	s_radio_attr.fhss = 0x00;

	rtcmEvent = xEventGroupCreate();
	radio_hal_init();
	//OS_DELAY_MS(50);
	pullMsgHandler = xQueueCreate(UHF_RX_QUEUE_NUM, UHF_TRANS_ITEM_SIZE);
	pubMsgHandler = xQueueCreate(UHF_TX_QUEUE_NUM, sizeof(uint32_t));
    cfifo_init(&tx_fifo, s_data_buffer, MAX_SEND_SIZE);
	cfifo_init(&read_fifo, s_data_buffer, MAX_DECODE_LEN);
	OS_DELAY_MS(50);
	xret = xTaskCreate(pub_rtcm_msg_handle, "pub_rtcm", (1024 * 1), NULL, DRV_UHF_TX_RTCM_TASK_PRI,
					   NULL);
	if (xret != pdPASS)
	{
		APP_LOG(TS_ON,VLEVEL_M,"task pub_rtcm failed\r\n");
	}
	OS_DELAY_MS(50);
	// APP_LOG(TS_ON,VLEVEL_M,"---after pub task ---Free heap memory: %d bytes------\r\n", xPortGetFreeHeapSize());
	xret = xTaskCreate(pull_rtcm_to_uart_handler, "pull_rtcm", 1024, NULL, DRV_UHF_RX_RTCM_TASK_PRI,
					   NULL);
	if (xret != pdPASS)
	{
		APP_LOG(TS_ON,VLEVEL_M,"task pull_rtcm failed\r\n");
	}
	xEventGroupSetBits(rtcmEvent, RADIO_EVT_RCV_DONE);
	xEventGroupSetBits(rtcmEvent, RADIO_EVT_SEND_DONE);

	
	init_led();
	APP_LOG(TS_ON,VLEVEL_M,"---after pull task ---Free heap memory: %d bytes------\r\n", xPortGetFreeHeapSize());

	s_radio_attr.magic = 0xfeedbeef;
	s_radio_attr.inited = 0x01;
	s_radio_attr.type = 0;
	s_radio_attr.switching = true; // true
	s_radio_attr.stop_rtcm = 0x01; // 0x01
	s_radio_attr.mode = RADIO_MODE_RX;
	s_radio_attr.freq[0] = 915000000;
	s_radio_attr.freq[1] = 915000000;
	s_radio_attr.bandwith[0] = 250;
	s_radio_attr.bandwith[1] = 250;
	s_radio_attr.bps = 38400;
	s_radio_attr.prot[1] = s_radio_attr.prot[0] = PROT_LORA;
	s_radio_attr.power_level = 10; //defaut 10 dbm

	//APP_LOG(TS_ON,VLEVEL_M,"attr size=%d \r\n",sizeof(s_radio_attr_flash));
	STMFLASH_Read(STM32_FLASH_APPCFG_BASE,(u64*)&s_radio_attr_flash,sizeof(s_radio_attr_flash)/8);

	APP_LOG(TS_ON,VLEVEL_M,"flash magic:0x%x \r\n",s_radio_attr_flash.magic);
	if(s_radio_attr_flash.magic != 0xfeedbeef)
	{
		APP_LOG(TS_ON,VLEVEL_M,"app cfg  first time init\r\n");
		s_radio_attr_flash = s_radio_attr;
		STMFLASH_Write(STM32_FLASH_APPCFG_BASE,(u64*)&s_radio_attr_flash,sizeof(s_radio_attr_flash)/8);
	}
	else
	{

		s_radio_attr.mode = s_radio_attr_flash.mode;
		s_radio_attr.freq[0] =s_radio_attr_flash.freq[0];
		s_radio_attr.freq[1] = s_radio_attr_flash.freq[1];
		s_radio_attr.bps = s_radio_attr_flash.bps;
		s_radio_attr.prot[0] =s_radio_attr_flash.prot[0];
		s_radio_attr.prot[1] = s_radio_attr_flash.prot[1];
		s_radio_attr.power_level = s_radio_attr_flash.power_level;
		APP_LOG(TS_ON,VLEVEL_M,"mode=%d bps = %d, freq =%d PROT=%d\r\n",s_radio_attr_flash.mode,s_radio_attr_flash.bps,
				s_radio_attr_flash.freq[0],s_radio_attr_flash.prot[0]);
	}

#if (E77_BOARD ==1)
	s_radio_attr.power_level = 20; // 20dbm for E77
#endif
	s_radio_attr.mode = RADIO_MODE_END;
	s_radio_attr.stop_rtcm = 0x01;

	if(s_radio_attr.mode != RADIO_MODE_END)
	{
		//init_app_cfg_to_radio();
	}
	APP_LOG(TS_ON,VLEVEL_M,"RADIO V%s INIT OK ---Free heap memory: %d bytes-----\r\n",VERSION,xPortGetFreeHeapSize());
	return 0;
}
