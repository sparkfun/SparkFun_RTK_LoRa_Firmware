
// #include <stdio.h>
// #include "main.h"
// #include "cmsis_os.h"
// #include "queue.h"
// #include "event_groups.h"

#include "usr_cmd.h"
// #include "string.h"
// #include "drv_radio.h"
// #include "app_common.h"
// #include "drv_uart.h"
// #include "sys_app.h"

#define USR_STRSTR strstr
#define USR_STRLEN strlen

static int cmd_com = 0; // default com1

static CMD_DECODE s_cmd_decode[2] = {0};

const USR_CMD_LIST s_usr_cmd_list[] =
{
	{ USR_CMD_ID_ATV,      "AT+V",     NULL, user_cmd_get_cfg, },
	{ USR_CMD_ID_AT_FRQ,   "AT+FRQ",   user_cmd_set_frq, user_cmd_get_frq, },
	{ USR_CMD_ID_AT_PWR,   "AT+PWR",   user_cmd_set_txpower, user_cmd_get_txpower, },
	{ USR_CMD_ID_AT_BAND,  "AT+BAND",  user_cmd_set_frqband, user_cmd_get_frqband, },
	{ USR_CMD_ID_AT_TYPE,  "AT+TYPE",  user_cmd_set_type, user_cmd_get_type, },
	{ USR_CMD_ID_AT_MODE,  "AT+MODE",  user_cmd_set_workmode, user_cmd_get_workmode, },
	{ USR_CMD_ID_AT_TRANS, "AT+TRANS", user_cmd_enter_trans, NULL, },
	{ USR_CMD_ID_AT_BPS,   "AT+BPS",   user_cmd_set_bps, user_cmd_get_bps, },
	{ USR_CMD_ID_AT_DPRT,  "AT+DPRT",  user_cmd_set_dprt, NULL, },
	{ USR_CMD_ID_AT_ATTR,  "AT+ATTR",  NULL, user_cmd_get_radio_attr, },
	{ USR_CMD_ID_AT_SAVE,  "AT+SAVE",  user_cmd_set_radio_attr_save, NULL, },
};

const int USR_CMD_MAX_END = sizeof(s_usr_cmd_list) / sizeof(s_usr_cmd_list[0]);

uint32_t send_cmd_rsp(const uint8_t *data, const uint16_t len)
{
	// uint32_t ret1 = drv_uart_com1_send(data, len);
	// uint32_t ret2 = drv_uart_com2_send(data, len);
	// return (ret1 > ret2 ? ret1 : ret2);

	if (cmd_com == 0)
	{
		return drv_uart_com1_send(data, len);
	}
	else
	{
		return drv_uart_com2_send(data, len);
	}
}

uint32_t user_cmd_enter_trans(uint32_t com, uint8_t *cmd)
{
	uint32_t ret;
	char *data = (char *)cmd;
	s_cmd_decode[com].state = TRNS;
	// TODO:need stop trans rtcm
	sprintf(data, "%s", "AT+TRANS\r\n\r\nOK\r\n");
	ret = strlen(data);

	// On Torch, it looks like the response gets gatecrashed by radio_param_cfg
	// I see "level: 10dbm" but not "AT+TRANS OK"
	// Send it now to make sure it gets sent
	// We should not need to do this. TODO: figure out why the response isn't sent
	// Maybe it is because radio_param_cfg() was writing to flash and so took a long time?
	//send_cmd_rsp((const uint8_t *)data, ret);

	// This will save RADIO_ATTR to flash if it has changed and enable_save is true
	radio_param_cfg();

	start_rtcm_trans();

	APP_LOG(TS_ON,VLEVEL_M,"enter trans\r\n");
	return ret;
}

void init_app_cfg_to_radio()
{
	char buf[64]={0};
	APP_LOG(TS_ON,VLEVEL_M,"init_app_cfg_to_radio\r\n");
	user_cmd_enter_trans(cmd_com, (uint8_t*)buf);
}

uint32_t user_cmd_exit_trans(uint32_t com, uint8_t *cmd)
{
	uint32_t ret;

	s_cmd_decode[cmd_com].state = SEARCH;

	ret = strlen(EXIT_TRANS);
	send_cmd_rsp((uint8_t*)EXIT_TRANS, ret);
	stop_rtcm_trans();
	APP_LOG(TS_ON,VLEVEL_M,"exit trans\r\n");
	return 0;
}

uint32_t user_cmd_get_cfg(uint32_t com, uint8_t *cmd)
{
	char *data = (char *)cmd;

	if (0 != strncasecmp((char *)cmd, "AT+V", USR_STRLEN("AT+V")))
	{
		sprintf(data + strlen(data), CMD_STR_ERR);
		return strlen(data);
	}

	sprintf(data, "version:%s\r\n\r\nOK\r\n", VERSION);

	return strlen(data);
}

uint32_t user_cmd_get_workmode(uint32_t com, uint8_t *cmd)
{
	char *data = (char *)cmd;
	RADIO_ATTR *m_radio_param = radio_get_cur_param();
	uint32_t work_mode = *(volatile uint32_t *)&m_radio_param->mode;

	work_mode = m_radio_param->mode;
	sprintf(data, "AT+MODE=%lu\r\n\r\nOK\r\n", work_mode);

	return strlen(data);
}

uint32_t user_cmd_set_workmode(uint32_t com, uint8_t *cmd)
{
	char *data = (char *)cmd;
	RADIO_ATTR *m_radio_param = radio_get_cur_param();
	int work_mode = 0;
	if (sscanf(data, "AT+MODE=%d\r\n", &work_mode) < 1)
	{
		sprintf(data + strlen(data), "%s", CMD_STR_ERR);
		return strlen(data);
	}
	else
	{

		sprintf(data + strlen(data), "%s", CMD_STR_OK);
		//if (m_radio_param->mode != work_mode)
		{
			m_radio_param->mode = (uint32_t)work_mode;
			//radio_param_cfg();
		}
	}

	return strlen(data);
}

uint32_t user_cmd_set_type(uint32_t com, uint8_t *cmd)
{
	char *data = (char *)cmd;
	RADIO_ATTR *m_radio_param = radio_get_cur_param();
	int radio_type = 0;
	if (sscanf(data, "AT+TYPE=%d\r\n", &radio_type) < 1)
	{
		sprintf(data + strlen(data), "%s", CMD_STR_ERR);
	}
	else
	{
		m_radio_param->type = (uint32_t)radio_type;
		sprintf(data + strlen(data), "%s", CMD_STR_OK);
		// todo
		//radio_param_cfg();
	}

	return strlen(data);
}

uint32_t user_cmd_get_type(uint32_t com, uint8_t *cmd)
{
	char *data = (char *)cmd;

	RADIO_ATTR *m_radio_param = radio_get_cur_param();
	sprintf(data, "AT+TYPE=%lu\r\n\r\nOK\r\n", m_radio_param->type);

	return strlen(data);
}

uint32_t user_cmd_set_txpower(uint32_t com, uint8_t *cmd)
{
	char *data = (char *)cmd;
	RADIO_ATTR *m_radio_param = radio_get_cur_param();
	int power_level = 0;
	if (sscanf(data, "AT+PWR=%d\r\n", &power_level) < 1)
	{
		sprintf(data + strlen(data), "%s", CMD_STR_ERR);
	}
	else
	{
		m_radio_param->power_level = (uint32_t)power_level;
		sprintf(data + strlen(data), "%s", CMD_STR_OK);
		// todo
		//radio_param_cfg();
	}

	return strlen(data);
}

uint32_t user_cmd_get_txpower(uint32_t com, uint8_t *cmd)
{
	RADIO_ATTR *m_radio_param = radio_get_cur_param();
	char *data = (char *)cmd;

	sprintf(data, "AT+PWR=%lu\r\n\r\nOK\r\n", m_radio_param->power_level);

	return strlen(data);
}

uint32_t user_cmd_set_frq(uint32_t com, uint8_t *cmd)
{
	char *data = (char *)cmd;
	RADIO_ATTR *m_radio_param = radio_get_cur_param();
//	int chl = 0;
	int tx_frq_m, rx_frq_m;
	int tx_frq_k, rx_frq_k;

	if (sscanf(data, "AT+FRQ=%d.%03d %d.%03d\r\n", &tx_frq_m, &tx_frq_k, &rx_frq_m, &rx_frq_k) < 4)
	{
		sprintf(data + strlen(data), "%s", CMD_STR_ERR);
	}
	else
	{
		m_radio_param->freq[0] = (uint32_t)tx_frq_m * FRQ_1M_HZ + (uint32_t)tx_frq_k * FRQ_1K_HZ;
		m_radio_param->freq[1] = (uint32_t)rx_frq_m * FRQ_1M_HZ + (uint32_t)rx_frq_k * FRQ_1K_HZ;
		sprintf(data + strlen(data), "%s", CMD_STR_OK);
		// APP_LOG(TS_ON,VLEVEL_M,"txfrq:%d rxfrq:%d\r\n", m_radio_param->freq[0], m_radio_param->freq[1]);
		// todo
		//radio_param_cfg();
	}

	return strlen(data);
}

uint32_t user_cmd_get_frq(uint32_t com, uint8_t *cmd)
{
	RADIO_ATTR *m_radio_param = radio_get_cur_param();

	char *data = (char *)cmd;

	sprintf(data, "AT+FRQ=%d.%03d %d.%03d\r\n\r\nOK\r\n",
			(int)(m_radio_param->freq[0] / FRQ_1M_HZ), (int)(m_radio_param->freq[0] % FRQ_1M_HZ / FRQ_1K_HZ),
			(int)(m_radio_param->freq[1] / FRQ_1M_HZ), (int)(m_radio_param->freq[1] % FRQ_1M_HZ / FRQ_1K_HZ));

	return strlen(data);
}

uint32_t user_cmd_set_frqband(uint32_t com, uint8_t *cmd)
{
	char *data = (char *)cmd;
	RADIO_ATTR *m_radio_param = radio_get_cur_param();
	int tx_bnd = 500, rx_bnd=500;

	if (sscanf(data, "AT+BAND=%d %d\r\n", &tx_bnd, &tx_bnd) < 2)
	{
		sprintf(data + strlen(data), "%s", CMD_STR_ERR);
	}
	else
	{
		m_radio_param->bandwidth[0] = (uint32_t)tx_bnd;
		m_radio_param->bandwidth[1] = (uint32_t)rx_bnd;
		sprintf(data + strlen(data), "%s", CMD_STR_OK);
		// todo
		//radio_param_cfg();
	}

	return strlen(data);
}

uint32_t user_cmd_get_frqband(uint32_t com, uint8_t *cmd)
{
	RADIO_ATTR *m_radio_param = radio_get_cur_param();
	char *data = (char *)cmd;

	sprintf(data, "AT+BAND=%lu %lu\r\n\r\nOK\r\n",
			m_radio_param->bandwidth[0], m_radio_param->bandwidth[1]);
	return strlen(data);
}

uint32_t user_cmd_set_bps(uint32_t com, uint8_t *cmd)
{
	char *data = (char *)cmd;
	RADIO_ATTR *m_radio_param = radio_get_cur_param();
	int bps;

	if (sscanf(data, "AT+BPS=%d\r\n", &bps) < 1)
	{
		sprintf(data + strlen(data), "%s", CMD_STR_ERR);
	}
	else
	{
		m_radio_param->bps = (uint32_t)bps;
		sprintf(data + strlen(data), "%s", CMD_STR_OK);
		// todo
		//radio_param_cfg();
	}

	return strlen(data);
}

uint32_t user_cmd_get_bps(uint32_t com, uint8_t *cmd)
{
	RADIO_ATTR *m_radio_param = radio_get_cur_param();
	char *data = (char *)cmd;

	sprintf(data, "AT+BPS=%lu\r\n\r\nOK\r\n",
			m_radio_param->bps);
	return strlen(data);
}

bool is_boot_pin_set(void)
{
	bool ret = false;
	if (HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_3) == GPIO_PIN_SET)
	{
		ret = true;
	}
	return ret;
}

uint32_t user_cmd_decode(uint8_t com, uint8_t *data, uint8_t len, uint8_t **act)
{
	int ret = 0;
	CMD_DECODE *p_cmd_decode = &s_cmd_decode[com];
	char *pbuff = (char *)p_cmd_decode->buff;
	char *head = NULL;
	// APP_LOG(TS_ON,VLEVEL_M,"enter dec \r\n");

	if (p_cmd_decode->state == TRNS) // If we are in TRANSfer
	{
		// Check for +++ (EXIT_TRANS)
		if (strstr((char *)data, EXIT_TRANS) != NULL)
		{
			APP_LOG(TS_ON,VLEVEL_M,"quit \r\n");
			p_cmd_decode->offset = 0;
			memset(pbuff, 0, p_cmd_decode->maxlen);
			user_cmd_exit_trans(com, (uint8_t*)pbuff);
		}
		else
		{
			// This is not +++, so send the data to the radio
			// if we are in TX mode
			if (usr_cmd_is_trans_tx())
			{
				APP_LOG(TS_ON,VLEVEL_M,"trans=%d \r\n", len);
				pub_rtcm(data, len);
			}
			else
			{
				APP_LOG(TS_ON,VLEVEL_M," ignoring %d \r\n", len);
			}
		}
		return 0;
	}

	// APP_LOG(TS_ON,VLEVEL_M,"boot pin set [%d] \r\n", is_boot_pin_set());

	// We are not in TRANSfer, so fully decode the command
	if ((len + p_cmd_decode->offset) > p_cmd_decode->maxlen)
	{
		// APP_LOG(TS_ON,VLEVEL_M,"drop %s.\r\n",data);
		// drop the cmd
		p_cmd_decode->offset = 0;
		memset(pbuff, 0, p_cmd_decode->maxlen); // TODO: p_cmd_decode->maxlen is much larger than CMD_RSP_MAX_LEN
		return 0;
	}
	else
	{
		memcpy(p_cmd_decode->buff + p_cmd_decode->offset, data, len);
		p_cmd_decode->offset += p_cmd_decode->offset;
		// check cmd header
		head = strstr(pbuff, CMD_HEADER_STR);
		// check tail
		char *psubstr = strstr(pbuff, CMD_STR_END);

		if (psubstr != NULL && head != NULL)
		{
			// p_cmd_decode->state = SEARCH;
			ret = psubstr - head + 2;
			p_cmd_decode->offset = 0;
			APP_LOG(TS_ON, VLEVEL_M, "decode:cmd:%s", p_cmd_decode->buff);
			//send_cmd_rsp(head,ret); // for debug lora test
			*act = (uint8_t*)head;
			return ret;
		}
		else
		{
			return 0;
		}
	}
}

void user_cmd_init(uint8_t com, uint8_t *cMsgBuf, uint32_t iMsgSize)
{
	if (!cMsgBuf || iMsgSize < CMD_RSP_MAX_LEN)
	{
		APP_LOG(TS_ON, VLEVEL_M, "user_decode_param err:%d\r\n", iMsgSize);
		return;
	}

	s_cmd_decode[com].buff = cMsgBuf;
	s_cmd_decode[com].maxlen = iMsgSize;
	s_cmd_decode[com].state = SEARCH;
	s_cmd_decode[com].len = 0;
	s_cmd_decode[com].offset = 0;
	cmd_com = com;
	APP_LOG(TS_OFF, VLEVEL_M, "user_cmd_init com %d\r\n",com);
}

// one cmd handle  rbuff>CMD_RSP_MAX_LEN

static int user_cmd_cb(uint32_t com, uint8_t *buff, uint32_t len, uint8_t *rbuff)
{
//	uint16_t cmd = USR_CMD_ID_START;
	int ret = 0;
	char *psubstr = NULL;
//	uint8_t flag = 0;
	uint8_t *rsp = (uint8_t *)rbuff;

	if (len > CMD_REQ_MAX_LEN)
	{
		return -1;
	}

	for (int i = 0; i < USR_CMD_MAX_END; i++)
	{
		if (s_usr_cmd_list[i].msgid != 0)
		{
			psubstr = strstr((char *)buff, (char *)s_usr_cmd_list[i].cmd_str);
			if (psubstr != NULL)
			{
				APP_LOG(TS_ON,VLEVEL_M,"hit cmd %s", buff);
				memset(rsp, 0, CMD_RSP_MAX_LEN);
				memcpy(rsp, buff, len); // backup
				if (strstr((char *)buff, CMD_AT_ASK) != NULL)
				{
					// get cmd
					if (s_usr_cmd_list[i].user_cmd_get_cb != NULL)
					{
						ret = s_usr_cmd_list[i].user_cmd_get_cb(com, (uint8_t *)rsp);
						if (ret > 0)
						{
							send_cmd_rsp(rsp, ret);
						}
					}
				}
				else
				{
					// set cmd
					if (s_usr_cmd_list[i].user_cmd_set_cb != NULL)
					{
						ret = s_usr_cmd_list[i].user_cmd_set_cb(com, (uint8_t *)rsp);
						if (ret > 0)
						{
							send_cmd_rsp(rsp, ret);
						}
						}
					else
					{
						sprintf((char*)rsp,"cmd[%d] no set cb\r\n",i);
						send_cmd_rsp(rsp,strlen((char*)rsp));
					}
				}
			}
		}
	}

	return ret;
}

void clear_cmd_buf(uint32_t com)
{
	memset(s_cmd_decode[com].buff, 0, s_cmd_decode[com].maxlen);
	s_cmd_decode[com].state = SEARCH;
}

static uint8_t cmd_cmd_buf[CMD_RSP_MAX_LEN + 1];
static uint8_t cmd_rsp_buf[CMD_RSP_MAX_LEN + 1];
int cmd_read_cb(uint8_t *data, const uint16_t len)
{
	int ret = 0;
	int cmd_len = 0;
	uint8_t *act = NULL;
	APP_LOG(TS_ON,VLEVEL_M,"cmd_com=%d len=%d\r\n",cmd_com,len);
	if (cmd_com >= 0)
	{
		// APP_LOG(TS_ON,VLEVEL_M,"call dec \r\n");
		cmd_len = user_cmd_decode(cmd_com, data, len, &act);
		if (cmd_len > 0 && cmd_len <= CMD_RSP_MAX_LEN)
		{
			// We decoded a command, so call its callback
			memset(cmd_cmd_buf, 0, CMD_RSP_MAX_LEN + 1);
			memset(cmd_rsp_buf, 0, CMD_RSP_MAX_LEN + 1);

			memcpy(cmd_cmd_buf, act, cmd_len);
			user_cmd_cb(cmd_com, cmd_cmd_buf, cmd_len, cmd_rsp_buf);
		}
	}

	return ret;
}

bool usr_cmd_is_trans_tx(void)
{
	bool isTransTx = false;
	if (s_cmd_decode[cmd_com].state == TRNS)
	{
		RADIO_ATTR *m_radio_param = radio_get_cur_param();
		if (m_radio_param->mode == RADIO_MODE_TX)
		{
			isTransTx = true;
		}
	}
	return isTransTx;
}

uint32_t user_cmd_set_dprt(uint32_t com, uint8_t *cmd)
{
	char *data = (char *)cmd;
	RADIO_ATTR *m_radio_param = radio_get_cur_param();
	int dprt;

	if (sscanf(data, "AT+DPRT=%d\r\n", &dprt) < 1)
	{
		sprintf(data + strlen(data), "%s", CMD_STR_ERR);
	}
	else
	{
		m_radio_param->dprt = dprt;
		sprintf(data + strlen(data), "%s", CMD_STR_OK);
		// todo
		//radio_param_cfg();
	}

	return strlen(data);
}

uint32_t user_cmd_get_radio_attr(uint32_t com, uint8_t *cmd)
{
	char *data = (char *)cmd;

	// cmd needs to be able to hold at least 154 bytes
	// RADIO_ATTR:\r\n        12
	// version: 300\r\n       14
	// mode:    0\r\n         12
	// freq tx: 915000000\r\n 20
	// freq rx: 915000000\r\n 20
	// bps:     38400\r\n     16
	// bwid tx: 250\r\n       14
	// bwid rx: 250\r\n       14
	// power:   10\r\n        13
	// dprt:    1\r\n         12
	// \r\nOK\r\n             6
	// NULL                   1

	RADIO_ATTR *m_radio_param = radio_get_cur_param();
	sprintf(data, "RADIO_ATTR:\r\n");
	sprintf(data + strlen(data), "version: %lu\r\n", m_radio_param->version);
	sprintf(data + strlen(data), "mode:    %lu\r\n", m_radio_param->mode);
	sprintf(data + strlen(data), "freq tx: %lu\r\n", m_radio_param->freq[0]);
	sprintf(data + strlen(data), "freq rx: %lu\r\n", m_radio_param->freq[1]);
	sprintf(data + strlen(data), "bps:     %lu\r\n", m_radio_param->bps);
	sprintf(data + strlen(data), "bwid tx: %lu\r\n", m_radio_param->bandwidth[0]);
	sprintf(data + strlen(data), "bwid rx: %lu\r\n", m_radio_param->bandwidth[1]);
	sprintf(data + strlen(data), "power:   %lu\r\n", m_radio_param->power_level);
	sprintf(data + strlen(data), "dprt:    %lu\r\n", m_radio_param->dprt);
	sprintf(data + strlen(data), "%s", CMD_STR_OK);

	return strlen(data);
}

uint32_t user_cmd_set_radio_attr_save(uint32_t com, uint8_t *cmd)
{
	char *data = (char *)cmd;
	RADIO_ATTR *m_radio_param = radio_get_cur_param();
	int enable;

	if (sscanf(data, "AT+SAVE=%d\r\n", &enable) < 1)
	{
		sprintf(data + strlen(data), "%s", CMD_STR_ERR);
	}
	else
	{
		m_radio_param->enable_save = enable;
		sprintf(data + strlen(data), "%s", CMD_STR_OK);
		// todo
		//radio_param_cfg();
	}

	return strlen(data);
}
