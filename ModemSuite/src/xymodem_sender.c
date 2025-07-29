/*
 * xymodem_sender.h
 *
 *  Created on: 2025_07_02
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */
#include "xymodem_sender.h"

/******************************************************************************/
/*-----------------------------PRIVATE-FUNCTION-------------------------------*/
/******************************************************************************/
static inline void xymodem_sender_send_control_code(XYMODEM_SENDER_RRD *self, uint8_t code){
    self->send_data(&code, 1);
}

static void xymodem_sender_reset_status(XYMODEM_SENDER_RRD *self){
    self->status_params.data = NULL;
    self->status_params.offset = 0;
    self->status_params.retry_count = 0;
    self->status_params.remain_count = 0;
    self->status_params.current_length = 0;
    self->status_params.current_pack_number = self->config.modem_type == modem_xmodem? 1 : 0;
}

static uint8_t xymodem_sender_unpack_control_code(XYMODEM_SENDER_RRD *self, uint8_t resp){
    if (resp == MODEM_ACK) {
        self->status_params.offset += self->status_params.current_length;
        self->status_params.remain_count -= self->status_params.current_length;
        self->status_params.current_pack_number = self->config.modem_type == modem_xmodem
                                                    ? ((self->status_params.current_pack_number % 255) + 1)
                                                    : ((self->status_params.current_pack_number + 1) % 255);
        self->status_params.retry_count = 0;
        if(self->status_params.remain_count == 0){
            xymodem_sender_send_control_code(self, MODEM_EOT);
            xymodem_sender_reset_status(self);
            return MODEM_CAN;
        }
        return resp;
    } else if (resp == MODEM_NAK) {
        if (++self->status_params.retry_count >= self->config.max_retry_count) {
            self->interface->stop(self);
            return MODEM_ERROR_OVER_MAX_RETRY_COUNT;
        }
        return resp;
    } else if (resp == MODEM_CAN) {
        return resp;
    }

    return MODEM_ERROR_UNKNOWN_CONTROL_CODE;
}

static int xymodem_sender_pack_and_send(XYMODEM_SENDER_RRD *self, const uint8_t *data, const size_t length){
    if(data == NULL){
        return MODEM_ERROR_PTR_NULL;
    }

    // 0. check length
    if(length > MODEM_1k_SIZE){
        return MODEM_ERROR_LENGTH_IS_INSUFFICIENT;
    }

    // 1. malloc pack
    size_t data_pack_size = moderm_get_data_length_size(self->config.length_type, length);
    MODEM_LENGTH_RRD length_type = data_pack_size == MODEM_128_SIZE? modem_128 : modem_1024;
    size_t pack_num = 3 + data_pack_size + (self->config.verify_type == modem_sum ? 1 : 2);
    uint8_t *pack = (uint8_t*)malloc(pack_num);
    if(pack == NULL){
        return MODEM_ERROR_PTR_NULL;
    }

    // 2. add pack header
    pack[0] = g_modem_soh[length_type];
    pack[1] = self->status_params.current_pack_number;
    pack[2] = (uint8_t)(~self->status_params.current_pack_number);
    
    // 3. copy data
    memcpy(&pack[3], data, length);
    if(data_pack_size > length){
        memset(&pack[3 + length], MODEM_NULL, data_pack_size - length);
    }

    // 4. add sum or crc16
    if (self->config.verify_type == modem_sum) {
        pack[3 + data_pack_size] = moderm_calcu_sum(&pack[3], data_pack_size);
    } else {
        uint16_t crc = moderm_calcu_crc16(&pack[3], data_pack_size);
        pack[3 + data_pack_size]     = (crc >> 8) & 0xFF;
        pack[3 + data_pack_size + 1] = crc & 0xFF;
    }

    // 5. send pack
    self->send_data(pack, pack_num);
    free(pack);
    return pack_num;
}

/******************************************************************************/
/*------------------------------PUBLIC-FUNCTION-------------------------------*/
/******************************************************************************/
void xymodem_sender_stop_transfer(XYMODEM_SENDER_RRD *self){
    xymodem_sender_send_control_code(self, MODEM_CAN);
    xymodem_sender_reset_status(self);
}

void xymodem_sender_start_transfer(XYMODEM_SENDER_RRD *self, uint8_t *data, const char *file_name, const size_t file_size){
    xymodem_sender_reset_status(self);

    self->status_params.data = data;
    self->status_params.offset = 0;
    self->status_params.current_length = 0;
    self->status_params.remain_count = file_size;

    if(self->config.modem_type == modem_ymodem){
        uint8_t pack[128] = {0};

        // 1.文件名
        size_t name_len = strlen(file_name);
        memcpy(pack, file_name, name_len);
        pack[name_len] = '\0';

        // 2.文件大小
        char file_size_str[16] = {0};
        snprintf(file_size_str, sizeof(file_size_str), "%lX", (unsigned long)file_size);
        size_t size_len = strlen(file_size_str);
        memcpy(&pack[name_len + 1], file_size_str, size_len);
        pack[name_len + size_len + 1] = '\0';

        // 3.发送数据包
        xymodem_sender_pack_and_send(self,pack,sizeof(pack));
    }
}

void xymodem_sender_close_session(XYMODEM_SENDER_RRD *self){
    if(self->config.modem_type == modem_xmodem){
        return;
    }

    uint8_t pack[133] = {0};

    pack[0] = MODEM_SOH;
    pack[1] = 0x00;
    pack[2] = (uint8_t)(~(pack[1]));

    self->send_data(pack, sizeof(pack));
}

/**
 *  \return 0: continue 1: finished
*/
int xymodem_sender_send_data(XYMODEM_SENDER_RRD *self, uint8_t resp){
    uint8_t retval = 0;
    uint8_t ret = xymodem_sender_unpack_control_code(self, resp);
    if(ret == MODEM_CAN || ret == MODEM_ERROR_OVER_MAX_RETRY_COUNT){
        retval = 1;
    }
    size_t data_pack_size = moderm_get_data_length_size(self->config.length_type, self->status_params.remain_count);
    self->status_params.current_length = self->status_params.remain_count >= data_pack_size
                                            ? data_pack_size 
                                            : self->status_params.remain_count;

    xymodem_sender_pack_and_send(self, self->status_params.data + self->status_params.offset, self->status_params.current_length);
    return retval;
}

/******************************************************************************/
/*--------------------------Construction/Destruction--------------------------*/
/******************************************************************************/
static XYMODEM_SENDER_INTERFACE_RRD g_xymodem_interface = {
    .send = (xymodem_sender_send_fn_t)xymodem_sender_send_data,
    .stop = (xymodem_sender_stop_transfer_fn_t)xymodem_sender_stop_transfer,
    .start = (xymodem_sender_start_transfer_fn_t)xymodem_sender_start_transfer,
    .close_session = (xymodem_sender_close_session_fn_t)xymodem_sender_close_session,
};

int xymodem_sender_init(XYMODEM_SENDER_RRD *self, MODEM_TYPE_RRD modem_type,
                  MODEM_LENGTH_RRD length_type, MODEM_VERIFY_RRD verify_type,
                  xymodem_sender_send_data_fn_t send_data){
    if(self == NULL || send_data == NULL){
        return -1;
    }
    if(modem_type == modem_xmodem && length_type == modem_length_null){
        return -2;
    }
    self->config.modem_type = modem_type;
    self->config.length_type = length_type;
    self->config.verify_type = verify_type;
    self->status_params.current_pack_number = modem_type == modem_xmodem? 1 : 0;

    self->send_data = send_data;
    self->interface = &g_xymodem_interface;
}


/******************************************************************************/
/*-----------------------------------DEBUG------------------------------------*/
/******************************************************************************/
#if OPEN_XYMODEM_SENDER_TEST == 1
void xymodem_debug_send_data(uint8_t *data, size_t length){
    printf("[xymodem-send-interface] ");
    for(size_t i = 0; i < length; ++i){
        printf("%02X ", data[i]);
    }
    printf("\r\n");
}

void xymodem_sender_debug(void){
    XYMODEM_SENDER_RRD xymodem_sender;
    xymodem_sender.config.max_retry_count = 10;
    xymodem_sender_init(&xymodem_sender, modem_crc16, xymodem_debug_send_data);

    #define raw_data_size   (1048)
    uint8_t raw_data[raw_data_size] = {0};
    { // 构建数据包
        for(size_t i = 0; i < raw_data_size; ++i){
            raw_data[i] = i;
        }
        #if 0
        for (size_t i = 0; i < raw_data_size; i++){
            printf("%02X ", raw_data[i]);
        }
        printf("\r\n");
        #endif
    }

    xymodem_sender.interface->start(&xymodem_sender,raw_data,"fuck.txt",raw_data_size);
    uint8_t count = 0;
    uint8_t resp = 0;
    while (1){
        count ++;
        if(count == 3){ // if XYMODEM_NAK
            resp = MODEM_NAK;
            printf("XYMODEM_NAK \r\n");
        }

        int ret = xymodem_transfer_data(&xymodem_sender,resp);
        if(ret){
            break;
        }
        resp = MODEM_ACK; // waiting resp
    }
}

#endif