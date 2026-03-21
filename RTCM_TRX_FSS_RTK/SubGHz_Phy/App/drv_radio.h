#ifndef __DRV_RADIO_H
#define __DRV_RADIO_H

typedef enum
{
	RADIO_MODE_TX = 0x00,
	RADIO_MODE_RX = 0x01,
	RADIO_MODE_TXRX = 0x02,
	RADIO_MODE_RELAY = 0x03,
	RADIO_MODE_END
} RADIO_MODE_ENUM;

typedef enum
{
	RADIO_COMBAUD_START,
	RADIO_COMBAUD_4800 = 1,
	RADIO_COMBAUD_9600 = 2,
	RADIO_COMBAUD_19200 = 3,
	RADIO_COMBAUD_38400 = 4,
	RADIO_COMBAUD_115200 = 5,
	RADIO_COMBAUD_230400 = 6,
	RADIO_COMBAUD_460800 = 7,
	RADIO_COMBAUD_921600 = 8,
	RADIO_COMBAUD_END,
} RADIO_COMBAUD_ENUM;

typedef enum
{
	RADIO_POWER_START = 1,
	RADIO_POWER_0_1W = 2,
	RADIO_POWER_0_5W = 3,
	RADIO_POWER_1W = 4,
	RADIO_POWER_2W = 5,
	RADIO_POWER_END
} RADIO_POWER_ENUM;

typedef enum
{
	RADIO_AIRBAUD_START,
	RADIO_AIRBAUD_4800 = 1,
	RADIO_AIRBAUD_9600 = 2,
	RADIO_AIRBAUD_19200 = 3,
	RADIO_AIRBAUD_38400 = 4,
	RADIO_AIRBAUD_END,
} RADIO_AIRBAUD_ENUM;

typedef enum
{
	RADIO_BAND_410_470MHZ = 0,
	RADIO_BAND_450_470MHZ = 1,
	RADIO_BAND_END,
} RADIO_BAND_ENUM;

typedef struct _RADIO_ATTR
{
	uint32_t magic;
	uint32_t version;
	uint32_t payload_len;// from  type to fhss
	uint32_t crc32;

	volatile int stop_rtcm;
	volatile int inited;
	volatile int switching;
	volatile int padding1;

	uint32_t stepper[2];
	uint32_t bandwith[2];

	uint32_t prot[2];
	uint32_t res[2];

	uint32_t freq[2];
	uint32_t wlbaud[2]; // air bps

	uint32_t type; // 0: lora 1: uhf
	uint32_t mode; // RADIO_MODE_TX RADIO_MODE_RX
	uint32_t bps;
	uint32_t advanced;

	uint32_t combaud;	// com bps
	uint32_t power_level;
	uint32_t crc_num;
	uint32_t fhss;
	
	uint32_t dprt; // 0:UART1 1:UART2
	uint32_t padding2;
	uint32_t flash_writes;
	uint32_t tail; // Last, for easy identification in CubeProgrammer
} RADIO_ATTR;

typedef struct __system_param
{
	char product_name[16];
	char hard_version[16];
	char hard_date[16];
	char software_version[16];
	char software_date[16];
} RADIO_INFO;

RADIO_ATTR *radio_get_cur_param(void);

uint32_t radio_init(void);

uint32_t radio_param_cfg(void);

RADIO_INFO *radio_get_sys_param(void);

int pub_rtcm(uint8_t *data, const uint16_t len);

int start_rtcm_trans(void);
int stop_rtcm_trans(void);

#endif
