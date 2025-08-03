/*
 * ymodem_test_config.h
 *
 *  Created on: 2025_07_28
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */

#ifndef MODEM_XMODEM_TEST_CONFIG_RRD_H_
#define MODEM_XMODEM_TEST_CONFIG_RRD_H_

#include "modem_common.h"

#define ymodem_test_save_data_type_callback (0)
#define ymodem_test_save_data_type_buff_ptr (1)

#define ymodem_test_modem_type              (modem_ymodem)
#define ymodem_test_length_type             (modem_length_null)
#define ymodem_test_verify_type             (modem_crc16)
#define ymodem_test_save_data_type          (ymodem_test_save_data_type_callback)

// #if ymodem_test_modem_type == modem_xmodem
//     #if ymodem_test_length_type == (modem_128)
//         #define ymodem_test_raw_data_buff_size  (133)
//         #define ymodem_test_data_buff_size      (MODEM_128_SIZE)
//     #else
//         #define ymodem_test_raw_data_buff_size (1029)
//         #define ymodem_test_data_buff_size      (MODEM_1k_SIZE)
//     #endif
// #else
    #define ymodem_test_raw_data_buff_size  (1029)
    #define ymodem_test_data_buff_size      (MODEM_1k_SIZE)
// #endif

#endif