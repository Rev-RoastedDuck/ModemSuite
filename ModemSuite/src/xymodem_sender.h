/*
 * xymodem_sender.h
 *
 *
 *  Created on: 2025_07_02
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */

#ifndef MODEM_XYMODEM_SENDER_RRD_H_
#define MODEM_XYMODEM_SENDER_RRD_H_

#include "stdint.h"
#include "stddef.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "modem_common.h"

/******************************************************************************/
/*----------------------------------INTERFACE---------------------------------*/
/******************************************************************************/
typedef void    (*xymodem_sender_stop_transfer_fn_t)(void *self);
typedef void    (*xymodem_sender_close_session_fn_t)(void *self);
typedef int     (*xymodem_sender_send_fn_t)(void *self, uint8_t resp);
typedef void    (*xymodem_sender_start_transfer_fn_t)(void *self, const uint8_t *data, const char *file_name, const size_t file_size);

typedef void (*xymodem_sender_send_data_fn_t)(uint8_t *data, size_t length);

typedef struct __XYMODEM_SENDER_INTERFACE_RRD{
    xymodem_sender_send_fn_t send;
    xymodem_sender_stop_transfer_fn_t stop;
    xymodem_sender_start_transfer_fn_t start;
    xymodem_sender_close_session_fn_t close_session;
}XYMODEM_SENDER_INTERFACE_RRD;

typedef struct __XYMODEM_SENDER_RRD {
    struct {
        uint8_t max_retry_count;
        MODEM_TYPE_RRD modem_type; 
        MODEM_LENGTH_RRD length_type;
        MODEM_VERIFY_RRD verify_type;
    } config;
 
    struct {
        uint8_t current_pack_number;
        uint8_t retry_count;

        uint8_t *data;
        size_t offset;
        size_t remain_count;
        size_t current_length;
    } status_params;

    XYMODEM_SENDER_INTERFACE_RRD *interface;

    xymodem_sender_send_data_fn_t send_data; // callback
} XYMODEM_SENDER_RRD;

int xymodem_sender_init(XYMODEM_SENDER_RRD *self, MODEM_TYPE_RRD modem_type,
                    MODEM_LENGTH_RRD length_type, MODEM_VERIFY_RRD verify_type,
                    xymodem_sender_send_data_fn_t send_data);

/******************************************************************************/
/*-----------------------------------DEBUG------------------------------------*/
/******************************************************************************/
#define OPEN_XYMODEM_SENDER_TEST 0
#if OPEN_XYMODEM_SENDER_TEST
void xymodem_sender_debug(void);
#endif
#endif
