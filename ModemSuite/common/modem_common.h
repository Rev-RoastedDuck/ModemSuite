/*
 * moderm_common.h
 *
 *  Created on: 2025_07_28
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */

#ifndef MODEM_COMMON_MODEM_COMMON_RRD_H_
#define MODEM_COMMON_MODEM_COMMON_RRD_H_

#include "stdio.h"
#include "stdint.h"

/******************************************************************************/
/*------------------------------MODEM_Control_Code----------------------------*/
/******************************************************************************/
#define MODEM_C       (0x43)  // xmodem 使用 crc16 校验的标志
#define MODEM_SOH     (0x01)  // xmodem 128 字节头标志
#define MODEM_STX     (0x02)  // xmodem 1024 字节头标志
#define MODEM_EOT     (0x04)  // xmodem 发送结束标志
#define MODEM_ACK     (0x06)  // xmodem 接收端接收到数据包
#define MODEM_NAK     (0x15)  // xmodem 接收端要求重发数据包
#define MODEM_CAN     (0x18)  // xmodem 取消接收数据
#define MODEM_NULL    (0x1A)  // xmodem 数据包填充字节

/******************************************************************************/
/*-------------------------------MODEM_Error_Code-----------------------------*/
/******************************************************************************/
#define MODEM_CODE_PACK_FINISHED                  ( 0)
#define MODEM_ERROR_NO_FOUND_HEADER               (-1)
#define MODEM_ERROR_LENGTH_IS_INSUFFICIENT        (-2)
#define MODEM_ERROR_CHECK_NUM                     (-3)
#define MODEM_ERROR_OVER_MAX_RETRY_COUNT          (-4)
#define MODEM_ERROR_PTR_NULL                      (-5)
#define MODEM_ERROR_UNKNOWN_CONTROL_CODE          (-6)
#define MODEM_CODE_SESSION_FINISHED               (-10)
#define MODEM_CODE_NULL                           (-11)

/******************************************************************************/
/*----------------------------------MODEM_TYPE--------------------------------*/
/******************************************************************************/
#define MODEM_128_SIZE    (128)
#define MODEM_1k_SIZE     (1024)

typedef enum {
    modem_length_null = -1,
    modem_128 = 0,
    modem_1024 = 1
}MODEM_LENGTH_RRD;

typedef enum {
    modem_sum = 0,
    modem_crc16 = 1
}MODEM_VERIFY_RRD;

typedef enum {
    modem_xmodem = 0,
    modem_ymodem = 1
}MODEM_TYPE_RRD;

extern const uint8_t g_modem_soh[];
extern const uint8_t g_modem_start_transfer[];

/******************************************************************************/
/*---------------------------------MODEM_TOOL---------------------------------*/
/******************************************************************************/
#define moderm_calcu_crc16_safe(_data_ptr, _length)             \
    ((_data_ptr == NULL || _length == 0) ?                      \
        0 : moderm_calcu_crc16(_data_ptr, _length))

#define moderm_calcu_sum_safe(_data_ptr, _length)               \
    ((_data_ptr == NULL || _length == 0) ?                      \
        0 : moderm_calcu_sum(_data_ptr, _length))    

uint8_t moderm_calcu_complement(uint8_t data);
uint8_t moderm_calcu_sum(const uint8_t *data, size_t length);
uint16_t moderm_calcu_crc16(const uint8_t *data, size_t length);
size_t moderm_get_data_length_size(MODEM_LENGTH_RRD lenth_type, size_t remain_data_length);

/******************************************************************************/
/*-----------------------------------DEBUG------------------------------------*/
/******************************************************************************/
#define OPEN_MODEM_TOOLS_TEST 0
#if OPEN_MODEM_TOOLS_TEST
void moderm_tools_debug(void);
#endif
#endif