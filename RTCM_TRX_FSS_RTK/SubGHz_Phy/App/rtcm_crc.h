#ifndef __RTCM_CRC_H
#define __RTCM_CRC_H
#include "stdint.h"
uint16_t rtk_crc16_1d0f(const uint8_t* buff, int32_t len);
uint16_t rtk_crc16(const uint8_t* buff, int32_t len);
uint32_t rtk_crc24q(const uint8_t *buff, int32_t len);
uint32_t rtk_crc32d(const uint8_t *buff, int32_t len);
uint32_t rtk_crc32(const uint8_t *buff, int32_t len);
uint32_t rtk_crc32_acc(uint32_t pcrc, const uint8_t  *buff, int32_t len);

#endif
