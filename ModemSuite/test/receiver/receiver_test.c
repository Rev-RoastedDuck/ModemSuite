/*
 * receiver_test.h
 *
 *
 *  Created on: 2025_07_28
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "xymodem.h"
#include "xymodem_test_config.h"

#define PORT 12345

int sockfd, new_sockfd;

void ymodem_debug_send_data(uint8_t *data, size_t length){
    printf("[ymodem-send-interface] ");
    for(size_t i = 0; i < length; ++i){
        printf("%02X ", data[i]);
    }
    printf("\r\n");
    send(new_sockfd, data, length, 0);
}

void receive_ymodem_file(const char *output_path) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    unsigned char buffer[ymodem_test_raw_data_buff_size];
    FILE *output_file;

    // 创建 TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // 绑定端口
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        close(sockfd);
        return;
    }

    // 开始监听
    listen(sockfd, 1);
    printf("Waiting for connection...\n");

    // 接受连接
    new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    if (new_sockfd < 0) {
        perror("Accept failed");
        close(sockfd);
        return;
    }

    // 打开输出文件
    output_file = fopen(output_path, "wb");
    if (output_file == NULL) {
        perror("Failed to open output file");
        close(new_sockfd);
        close(sockfd);
        return;
    }

    // 接收数据
    XYMODEM_RECEIVER_RRD ymodem;
    ymodem.config.max_retry_count = 10;
    xymodem_receiver_init(&ymodem, ymodem_test_modem_type, ymodem_test_length_type, ymodem_test_verify_type, ymodem_debug_send_data);

    typedef struct __YMODEM_PACK_INFO{
        size_t length; 
        uint8_t data_pack[ymodem_test_data_buff_size];
    }YMODEM_PACK_INFO;
    
    uint8_t data[1024] = {0};
    while (1) {
        printf("==========\r\n");
        ssize_t bytes_received = recv(new_sockfd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }

        int result = ymodem.interface->receive(&ymodem, buffer, bytes_received, data, sizeof(buffer));
        if(result < 0){
            printf("unpack error. %d \r\n",result);
        } else if (result > 0){
            fwrite(data, 1, result, output_file);
            printf("unpack success. %d \r\n",result);
            
        } else if (result == 0){
            printf("unpack finished. %d \r\n",result);
        }
        usleep(10000);
    }

    fclose(output_file);
    close(new_sockfd);
    close(sockfd);
}

int main() {
    receive_ymodem_file("received_file.txt");
    return 0;
}
