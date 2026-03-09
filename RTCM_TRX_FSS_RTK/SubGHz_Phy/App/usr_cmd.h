#ifndef _USR_CMD_H_
#define _USR_CMD_H_

#include <stdint.h>

typedef uint32_t (*USER_CMD_CB)(uint32_t com, uint8_t *cmd);

#define CMD_POWER_H RADIO_POWER_1W
#define CMD_POWER_L RADIO_POWER_0_5W

#define CMD_MIN_LEN (5)
#define CMD_MAX_LEN (32)
#define CMD_STR_END ("\r\n")
#define CMD_STR_OK ("\r\nOK\r\n")
#define CMD_STR_ERR ("\r\nERROR\r\n")
#define CMD_HEADER_STR ("AT+")
#define CMD_HEADER_LEN 3

#define CMD_AT_FRQ_FORMAT "AT+FRQ=%d.%03d %d.%03d\r\n"

#define CMD_AT_PWR_FORMAT "AT+PWR=%d\r\n" // db [0-14]

#define CMD_AT_BAND_FORMAT "AT+BAND=%d %d\r\n" // unit:khz

#define CMD_AT_TYPE_FORMAT "AT+TYPE=%d\r\n" // 0:lora 1:uhf

#define CMD_AT_WORKMODE_FORMAT "AT+MODE=%d\r\n" // 0:RX 1:TX


#define CMD_AT_BPS_FORMAT "AT+BPS=%d\r\n" // unit:hundred 19200

#define CMD_AT_PROT_FORMAT "AT+PROT=%s\r\n" // TRANSP  TT450 TRIMARK

#define CMD_AT_ASK "?"

#define CMD_STR_ATV ("AT+V")
#define CMD_STR_AT_FRQ ("AT+FRQ")
#define CMD_STR_AT_PWR ("AT+PWR")
#define CMD_STR_AT_BAND ("AT+BAND")
#define CMD_STR_AT_TYPE ("At+TYPE")
#define CMD_STR_AT_MODE ("AT+MODE")
#define CMD_STR_AT_BPS ("AT+BPS") // 19200 38400 62500
#define CMD_STR_ENTER_TRANS ("AT+TRANS")

#define CMD_REQ_MAX_LEN 64
#define CMD_RSP_MAX_LEN 128

#define PROT_LORA  3 // 38400bps 6000hz 12000hz

typedef enum
{
	BPS_192,
	BPS_384,
	BPS_625
}BPS_T;

typedef enum
{
	USR_CMD_ID_START = 0,
	USR_CMD_ID_ATV,
	USR_CMD_ID_AT_FRQ,
	USR_CMD_ID_AT_PWR,
	USR_CMD_ID_AT_BAND,
	USR_CMD_ID_AT_TYPE,
	USR_CMD_ID_AT_MODE,
	USR_CMD_ID_AT_T,
	USR_CMD_ID_AT_BPS,
	USR_CMD_ID_END
} USR_CMD_ID;

typedef struct
{
	uint8_t msgid;
	uint8_t cmd_str[8];
	USER_CMD_CB user_cmd_set_cb;
	USER_CMD_CB user_cmd_get_cb;
} USR_CMD_LIST;

#define SEARCH (0)
#define SYNC (1)
#define TRNS (2) // +++ exit, 3 sec auto enter trans mode
#define EXIT_TRANS "+++"

typedef struct cmd_decode
{
	uint32_t len;
	uint32_t maxlen;
	uint32_t offset;
	uint8_t state;
	uint8_t *buff;
} CMD_DECODE;

#define FRQ_1K_HZ 1000
#define FRQ_1M_HZ 1000000

uint32_t user_cmd_decode(uint8_t com, uint8_t *data,uint8_t len,uint8_t **act);
void user_cmd_init(uint8_t com, uint8_t *cMsgBuf, uint32_t iMsgSize);

int cmd_read_cb(uint8_t *data, const uint16_t len);

uint32_t user_cmd_enter_trans(uint32_t com, uint8_t *cmd);
uint32_t user_cmd_exit_trans(uint32_t com, uint8_t *cmd);

uint32_t user_cmd_get_cfg(uint32_t com, uint8_t *cmd);

uint32_t user_cmd_get_workmode(uint32_t com, uint8_t *cmd);
uint32_t user_cmd_set_workmode(uint32_t com, uint8_t *cmd);

uint32_t user_cmd_get_type(uint32_t com, uint8_t *cmd);
uint32_t user_cmd_set_type(uint32_t com, uint8_t *cmd);

uint32_t user_cmd_set_txpower(uint32_t com, uint8_t *cmd);
uint32_t user_cmd_get_txpower(uint32_t com, uint8_t *cmd);

uint32_t user_cmd_set_frq(uint32_t com, uint8_t *cmd);
uint32_t user_cmd_get_frq(uint32_t com, uint8_t *cmd);

uint32_t user_cmd_set_frqband(uint32_t com, uint8_t *cmd);
uint32_t user_cmd_get_frqband(uint32_t com, uint8_t *cmd);
uint32_t user_cmd_set_bps(uint32_t com, uint8_t *cmd);
uint32_t user_cmd_get_bps(uint32_t com, uint8_t *cmd);

uint32_t send_cmd_rsp(const uint8_t *data, const uint16_t len);

void clear_cmd_buf(void);

//call one time
void init_app_cfg_to_radio();
#endif
