/*
 * moderm_common.h
 *
 *  Created on: 2025_07_28
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */

#include "modem_common.h"

const uint8_t g_modem_start_transfer[] = {
    [modem_sum] = MODEM_NAK,
    [modem_crc16] = MODEM_C,
};

const uint8_t g_modem_soh[] = {
    [modem_128] = MODEM_SOH,
    [modem_1024] = MODEM_STX,
};

uint8_t moderm_calcu_complement(uint8_t data){
    return ~data;
}

uint8_t moderm_calcu_sum(const uint8_t *data, size_t length){
    uint8_t re = 0;
    for(size_t index = 0; index < length;++index){
        re += data[index];
    }
    return re;
}

uint16_t moderm_calcu_crc16(const uint8_t *data, size_t length) {
    uint16_t crc = 0x0000;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

size_t moderm_get_data_length_size(MODEM_LENGTH_RRD lenth_type, size_t remain_data_length){
    if(lenth_type == modem_128){
        return MODEM_128_SIZE;
    }
    else if(lenth_type == modem_1024){
        return MODEM_1k_SIZE;
    }
    else if(lenth_type == modem_length_null){
        return remain_data_length <= MODEM_128_SIZE? MODEM_128_SIZE : MODEM_1k_SIZE;
    }
}

/******************************************************************************/
/*-----------------------------------DEBUG------------------------------------*/
/******************************************************************************/
#if OPEN_MODEM_TOOLS_TEST
#include "stdio.h"
void moderm_tools_debug(void){
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t sum = moderm_calcu_sum_safe(data, 0);
    uint16_t crc = moderm_calcu_crc16_safe(data, sizeof(data));
    printf("sum: %d, crc:%x \r\n",sum,crc);
    return;
}
#endif
