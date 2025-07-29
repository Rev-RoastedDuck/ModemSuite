/*
 * sender_test.h
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

int sockfd;

void ymodem_debug_send_data(uint8_t *data, size_t length){
    printf("[ymodem-send-interface] ");
    for(size_t i = 0; i < length; ++i){
        printf("%02X ", data[i]);
    }
    printf("\r\n");

    send(sockfd, data, length, 0);
}

void send_ymodem_file(const char *file_path, const char *server_ip) {
    struct sockaddr_in server_addr;
    FILE *file;
    
    // 打开文件
    file = fopen(file_path, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    // 移动文件指针到末尾以获取文件大小
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file); // 回到文件开头

    // 分配内存缓冲区
    unsigned char *buffer = (unsigned char *)malloc(file_size);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return;
    }

    // 读取文件内容到缓冲区
    size_t read_size = fread(buffer, 1, file_size, file);
    if (read_size != file_size) {
        perror("Failed to read complete file");
        free(buffer);
        fclose(file);
        return;
    }

    // 创建 TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        fclose(file);
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    
    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        fclose(file);
        close(sockfd);
        return;
    }

    // 发送文件数据
    uint8_t resp = 0;

    XYMODEM_SENDER_RRD ymodem_sender;
    ymodem_sender.config.max_retry_count = 10;
    xymodem_sender_init(&ymodem_sender, ymodem_test_modem_type, ymodem_test_length_type, ymodem_test_verify_type, ymodem_debug_send_data);
    ymodem_sender.interface->start(&ymodem_sender,buffer,file_path,read_size);
    if(ymodem_test_modem_type == modem_ymodem){
        recv(sockfd, &resp, 1, 0); // waiting resp
        if( resp == MODEM_ACK){
            printf("[sender] resp: ack\r\n");
        }
    }
    while (1) {
        printf("==========\r\n");
        int ret = ymodem_sender.interface->send(&ymodem_sender,resp);
        if(ret){
            break;
        }

        recv(sockfd, &resp, 1, 0); // waiting resp
        printf("[sender] resp: %d \r\n", resp);
        usleep(10000);
    }

    printf("File sent successfully.\n");

    fclose(file);
    close(sockfd);
}

int main() {
    send_ymodem_file("file.txt", "127.0.0.1");
    return 0;
}