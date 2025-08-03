/*
 * xymodem_receiver.h
 *
 *  Created on: 2025_07_28
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */
#include "xymodem_receiver.h"

/******************************************************************************/
/*-----------------------------PRIVATE-FUNCTION-------------------------------*/
/******************************************************************************/
static inline bool xymodem_is_all_zero(const uint8_t* src, size_t length) {
    while(length--){
        if(*src++ != 0){
            return false;
        }
    }
    return true;
}

static inline void xymodem_receiver_send_control_code(XYMODEM_RECEIVER_RRD *self, uint8_t code){
    self->send_data(&code, 1);
}

static void xymodem_receiver_reset_status(XYMODEM_RECEIVER_RRD *self){
    self->status_params.retry_count = 0;
    self->status_params.next_pack_number = self->config.modem_type == modem_xmodem? 1 : 0;
}

/******************************************************************************/
/*------------------------------PUBLIC-FUNCTION-------------------------------*/
/******************************************************************************/
void xymodem_receiver_start_transfer(XYMODEM_RECEIVER_RRD *self){
    xymodem_receiver_reset_status(self);
    size_t start = self->get_time_ms();
    xymodem_receiver_send_control_code(self, g_modem_start_transfer[self->config.verify_type]);
    while (self->get_time_ms() < start + 1000);
    xymodem_receiver_send_control_code(self, g_modem_start_transfer[self->config.verify_type]);
}

void xymodem_receiver_stop_transfer(XYMODEM_RECEIVER_RRD *self){
    xymodem_receiver_send_control_code(self, MODEM_CAN);
}

/**
 * \return size of last pack data
*/
size_t xymodem_receiver_strip_padding(XYMODEM_RECEIVER_RRD *self, const uint8_t *data, const size_t length) {
    size_t new_len = length;
    while (new_len > 0 && data[new_len - 1] == MODEM_NULL) {
        new_len--;
    }
    return new_len;
}

/**
 * \return > 0: data start index
 *         = 0: transfer finished
 *         < 0: error code
*/
int xymodem_receiver_unpack(XYMODEM_RECEIVER_RRD *self, 
                            const uint8_t *raw_data, const size_t raw_data_length,
                            uint8_t *dest, const size_t dest_capacity,
                            uint8_t *pack_index, size_t *pack_length){
    if(raw_data == NULL
        || (self->save_data == NULL && dest == NULL)){
        return MODEM_ERROR_PTR_NULL;
    }

    // 0. check EOT / CAN / SESSION_FINISHED
    if(raw_data_length == 1 && raw_data[0] == MODEM_EOT){
        // save last pack
        if(self->status_params.next_pack_number > 1){
            self->status_params.prev.length = xymodem_receiver_strip_padding(self, 
                                                                            self->status_params.prev.data_pack, 
                                                                            self->status_params.prev.length);
            *pack_length = dest_capacity > self->status_params.prev.length? self->status_params.prev.length : dest_capacity;
            if(self->save_data == NULL){
                memcpy(dest, self->status_params.prev.data_pack,*pack_length);
            }else{
                self->save_data(self->status_params.prev.data_pack,*pack_length);
            }
        }
        xymodem_receiver_send_control_code(self, MODEM_ACK);
        return MODEM_CODE_PACK_FINISHED;
    }
    
    if(raw_data_length == 1 && raw_data[0] == MODEM_CAN){
        xymodem_receiver_reset_status(self);
        return MODEM_ERROR_MANUALLT_CANCEL;
    }

    // 1. check length
    if((self->config.length_type == modem_length_null && raw_data_length < MODEM_128_SIZE + 3 + (self->config.verify_type == modem_sum? 1 : 2))
        || (self->config.length_type == modem_128 && raw_data_length < MODEM_128_SIZE + 3 + (self->config.verify_type == modem_sum? 1 : 2))
        || (self->config.length_type == modem_1024 && raw_data_length < MODEM_1k_SIZE + 3 + (self->config.verify_type == modem_sum? 1 : 2))){
        return MODEM_ERROR_LENGTH_IS_INSUFFICIENT;
    }

    // 2. find SOH/STX pack_num ~pack_num
    bool is_tail_pack = false;
    size_t data_pack_size = 0;
    int soh_index = -1;
    for(size_t index = 2; index < raw_data_length;++index){
        if(((self->config.length_type == modem_128 && raw_data[index - 2] == MODEM_SOH)
            || (self->config.length_type == modem_1024 && raw_data[index - 2] == MODEM_STX)
            || (self->config.length_type == modem_length_null && (raw_data[index - 2] == MODEM_SOH || raw_data[index - 2] == MODEM_STX)))
            && raw_data[index - 1] == self->status_params.next_pack_number
            && raw_data[index - 0] == (uint8_t)(~(self->status_params.next_pack_number))){ // find SOH/STX
            soh_index = index - 2;
            data_pack_size = raw_data[index - 2] == MODEM_SOH ? MODEM_128_SIZE : MODEM_1k_SIZE;
            break;
        }
        if((raw_data[index - 2] == MODEM_SOH)
            && raw_data[index - 1] == 0x00 && raw_data[index - 0] == 0xff){ // find SESSION_FINISHED
            is_tail_pack = true;
            soh_index = index - 2;
            data_pack_size = MODEM_128_SIZE;
            break;
        }
    }
    if(soh_index < 0){
        return MODEM_ERROR_NO_FOUND_HEADER;
    }

    if(soh_index + data_pack_size 
                 + (self->config.verify_type == modem_sum? 1 : 2) > raw_data_length){
        return MODEM_ERROR_LENGTH_IS_INSUFFICIENT;
    }

    if(is_tail_pack){
        if(xymodem_is_all_zero(&raw_data[soh_index + 3], data_pack_size)){
            xymodem_receiver_send_control_code(self, MODEM_ACK);
            return MODEM_CODE_SESSION_FINISHED;
        }
    }

    // 3. check sum/crc16
    uint16_t check_num = 0;
    uint16_t check_num_get = 0; 
    if(self->config.verify_type == modem_sum){
        check_num_get = raw_data[soh_index + 3 + data_pack_size];
        check_num = moderm_calcu_sum(&raw_data[soh_index + 3], data_pack_size);
    }else{
        check_num = moderm_calcu_crc16(&raw_data[soh_index + 3], data_pack_size);
        check_num_get = ((uint16_t)raw_data[soh_index + 3 + data_pack_size] << 8) | (uint16_t)raw_data[soh_index + 3 + data_pack_size + 1];
    }
    if(check_num != check_num_get){
        xymodem_receiver_send_control_code(self,MODEM_NAK);
        if (++self->status_params.retry_count >= self->config.max_retry_count) {
            xymodem_receiver_send_control_code(self, MODEM_CAN);
            printf("[xymodem_receiver_unpack] out max retry count");
            return MODEM_ERROR_OVER_MAX_RETRY_COUNT;
        }
        printf("[xymodem_receiver_unpack] check_num error, get: %04X, expect: %04X\r\n", check_num_get, check_num);
        return MODEM_ERROR_CHECK_NUM;
    }
    
    // 4. output data
    if(self->status_params.next_pack_number > 1){
        pack_index != NULL && (*pack_index = self->status_params.next_pack_number - 1, true);
        *pack_length = dest_capacity > self->status_params.prev.length? self->status_params.prev.length : dest_capacity;
        if(self->save_data == NULL){
            memcpy(dest, self->status_params.prev.data_pack,*pack_length);
        }else{
            self->save_data(self->status_params.prev.data_pack,*pack_length);
        }
    }

    // 5.update status
    self->status_params.retry_count = 0;
    self->status_params.prev.length = data_pack_size;
    memcpy(self->status_params.prev.data_pack, &raw_data[soh_index + 3], data_pack_size);
    self->status_params.next_pack_number = self->config.modem_type == modem_xmodem
                                            ? ((self->status_params.next_pack_number % 255) + 1)
                                            : ((self->status_params.next_pack_number + 1) % 255);
    
    // 6. ack
    xymodem_receiver_send_control_code(self,MODEM_ACK);

    return MODEM_CODE_UNPACK_SUCCESS;
}

/******************************************************************************/
/*--------------------------Construction/Destruction--------------------------*/
/******************************************************************************/
static XYMODEM_RECEIVER_INTERFACE_RRD g_xymodem_interface = {
    .unpack = (xymodem_receiver_unpack_fn_t)xymodem_receiver_unpack,
    .stop = (xymodem_receiver_stop_transfer_fn_t)xymodem_receiver_stop_transfer,
    .start = (xymodem_receiver_start_transfer_fn_t)xymodem_receiver_start_transfer,
    .strip_padding = (xymodem_receiver_strip_padding_fn_t)xymodem_receiver_strip_padding,
};

int xymodem_receiver_init(XYMODEM_RECEIVER_RRD *self, MODEM_TYPE_RRD modem_type,
                          MODEM_LENGTH_RRD length_type,MODEM_VERIFY_RRD verify_type,
                          xymodem_receiver_send_data_fn_t send_data,
                          xymodem_receiver_get_time_ms_fn_t get_time_ms,
                          xymodem_receiver_save_data_fn_t save_data){
    if(self == NULL || send_data == NULL || get_time_ms == NULL){
        return -1;
    }
    if(modem_type == modem_xmodem && length_type == modem_length_null){
        return -2;
    }
    self->config.modem_type = modem_type;
    self->config.length_type = length_type;
    self->config.verify_type = verify_type;
    self->status_params.retry_count = 0;
    self->status_params.next_pack_number = modem_type == modem_xmodem? 1 : 0;

    self->interface = &g_xymodem_interface;
    self->send_data = send_data;
    self->get_time_ms = get_time_ms;

    if(save_data != NULL){
        self->save_data = save_data;
    }else{
        self->save_data = NULL;
    }

    return 0;
}

/******************************************************************************/
/*-----------------------------------DEBUG------------------------------------*/
/******************************************************************************/
#if OPEN_XYMODEM_TEST
void xymodem_debug_send_data(uint8_t *data, size_t length){
    printf("[xymodem-send-interface] ");
    for(size_t i = 0; i < length; ++i){
        printf("%02X ", data[i]);
    }
    printf("\r\n");
}

void xymodem_debug(void){
    XYMODEM_RECEIVER_RRD xymodem;
    xymodem.config.max_retry_count = 10;
    xymodem_receiver_init(&xymodem, modem_crc16, xymodem_debug_send_data);

    {
        #define MODEM_SIZE          (MODEM_128_SIZE)
        #if MODEM_SIZE == MODEM_1k_SIZE
            #define MODEM_SIZE_TYPE (modem_1024)
        #else
            #define MODEM_SIZE_TYPE (modem_128)
        #endif
        volatile uint8_t raw_data[MODEM_SIZE + 5] = {MODEM_NULL};
        { // 构建第零个数据包
            // 1.xymodem数据包类型
            raw_data[0] = g_modem_soh[MODEM_SIZE_TYPE];

            // 2.数据包编码和反码
            raw_data[1] = 0x00;
            raw_data[2] = ~(raw_data[1]);

            // 3.数据主体
            for(size_t i = 0; i < MODEM_SIZE; ++i){
                raw_data[i + 3] = i;
            }

            // 4.CRC校验码
            uint16_t crc = moderm_calcu_crc16((const uint8_t *)&raw_data[3], MODEM_SIZE);
            raw_data[MODEM_SIZE+3] = (crc >> 8) & 0xFF;
            raw_data[MODEM_SIZE+4] = crc & 0xFF;

            // 5.输出数据包
            for (size_t i = 0; i < sizeof(raw_data); i++){
                printf("%02X ", raw_data[i]);
            }
            printf("\r\n");
        }

        { // 解析第零个数据包
            uint8_t pack_num = 0;
            uint8_t data_pack[MODEM_SIZE] = {0};
            int result = xymodem.interface->unpack(&xymodem, (const uint8_t *)raw_data, sizeof(raw_data), data_pack, &pack_num);
            if(result < 0){
                printf("unpack error: %d\r\n", result);
            } else if (result > 0){
                printf("unpack success, data_pack size: %zu\r\n", xymodem.status_params.data_pack_size);
                for(size_t i = 0; i < xymodem.status_params.data_pack_size; ++i){
                    printf("%02X ", data_pack[i]);
                }
                printf("\r\n");
            }
        }
        #undef MODEM_SIZE
        #undef MODEM_SIZE_TYPE
    }

    {
        #define MODEM_SIZE          (MODEM_1k_SIZE)
        #if MODEM_SIZE == MODEM_1k_SIZE
            #define MODEM_SIZE_TYPE (modem_1024)
        #else
            #define MODEM_SIZE_TYPE (modem_128)
        #endif

        volatile uint8_t raw_data[MODEM_SIZE + 5] = {0};
        memset((uint8_t *)raw_data, MODEM_NULL, sizeof(raw_data));
        { // 构建数据包
            // 1.xymodem数据包类型
            raw_data[0] = g_modem_soh[MODEM_SIZE_TYPE];
            
            // 2.数据包编码和反码
            raw_data[1] = 0x01;
            raw_data[2] = ~(raw_data[1]);

            // 3.数据主体
            for(size_t i = 0; i < MODEM_SIZE; ++i){
                raw_data[i + 3] = i;
            }

            // 4.CRC校验码
            #if MODEM_SIZE == MODEM_1k_SIZE
                raw_data[MODEM_SIZE+3] = 0xC2;
                raw_data[MODEM_SIZE+4] = 0xE0;
            #else
                raw_data[MODEM_SIZE+3] = 0xE8;
                raw_data[MODEM_SIZE+4] = 0x0A;
            #endif

            // 5.输出数据包
            for (size_t i = 0; i < sizeof(raw_data); i++){
                printf("%02X ", raw_data[i]);
            }
            printf("\r\n");
        }

        { // 解析第一个数据包
            uint8_t pack_num = 0;
            uint8_t data_pack[MODEM_SIZE] = {0};
            int result = xymodem.interface->unpack(&xymodem, (const uint8_t *)raw_data, sizeof(raw_data), data_pack, &pack_num);
            if(result < 0){
                printf("unpack error: %d\r\n", result);
            } else if (result > 0){
                printf("unpack success, data_pack size: %zu\r\n", xymodem.status_params.data_pack_size);
                for(size_t i = 0; i < xymodem.status_params.data_pack_size; ++i){
                    printf("%02X ", data_pack[i]);
                }
                printf("\r\n");
            }
        }
        #undef MODEM_SIZE
        #undef MODEM_SIZE_TYPE
    }

    {
        #define MODEM_SIZE          (MODEM_128_SIZE)
        #if MODEM_SIZE == MODEM_1k_SIZE
            #define MODEM_SIZE_TYPE (modem_1024)
        #else
            #define MODEM_SIZE_TYPE (modem_128)
        #endif
        volatile uint8_t raw_data[MODEM_SIZE + 5] = {MODEM_NULL};
        { // 构建第零个数据包
            // 1.xymodem数据包类型
            raw_data[0] = g_modem_soh[MODEM_SIZE_TYPE];

            // 2.数据包编码和反码
            raw_data[1] = 0x00;
            raw_data[2] = ~(raw_data[1]);

            // 3.数据主体
            for(size_t i = 0; i < MODEM_SIZE; ++i){
                raw_data[i + 3] = 0;
            }

            // 4.CRC校验码
            uint16_t crc = moderm_calcu_crc16((const uint8_t *)&raw_data[3], MODEM_SIZE);
            raw_data[MODEM_SIZE+3] = (crc >> 8) & 0xFF;
            raw_data[MODEM_SIZE+4] = crc & 0xFF;

            // 5.输出数据包
            for (size_t i = 0; i < sizeof(raw_data); i++){
                printf("%02X ", raw_data[i]);
            }
            printf("\r\n");
        }

        { // 解析第二个数据包 (结束会话)
            uint8_t pack_num = 0;
            uint8_t data_pack[MODEM_SIZE] = {0};
            int result = xymodem.interface->unpack(&xymodem, (const uint8_t *)raw_data, sizeof(raw_data), data_pack, &pack_num);
            if(result < 0){
                printf("unpack error: %d\r\n", result);
            } else if (result > 0){
                printf("unpack success, data_pack size: %zu\r\n", xymodem.status_params.data_pack_size);
                for(size_t i = 0; i < xymodem.status_params.data_pack_size; ++i){
                    printf("%02X ", data_pack[i]);
                }
                printf("\r\n");
            }
        }
        #undef MODEM_SIZE
        #undef MODEM_SIZE_TYPE
    }
}
#endif