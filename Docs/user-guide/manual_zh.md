<div align="center">
  <img src="../images/icon.webp" height="150">
  <h1>ModemSuite</h1>
  <span>一个基于C语言实现的集成了XMODEM、YMODEM 文件协议的项目</span>
</div>
<br>
<div align="center">
  <img src="https://img.shields.io/badge/License-Apache2.0-green?logoColor=63%2C%20185%2C%2017&label=license&labelColor=63%2C%20185%2C%2017&color=63%2C%20185%2C%2017">
  <img src="https://img.shields.io/badge/Language-C-green?logoColor=63%2C%20185%2C%2017&labelColor=63%2C%20185%2C%2017&color=63%2C%20185%2C%2017">
</div>

## 接口说明
### 发送端（Sender）接口
- `int xymodem_sender_init(XYMODEM_SENDER_RRD *, MODEM_TYPE_RRD, MODEM_LENGTH_RRD, MODEM_VERIFY_RRD, xymodem_sender_send_data_fn_t);`  
  - 初始化发送端对象，配置协议类型、包长、校验方式及底层发送回调。

- `void xymodem_sender_start_transfer(XYMODEM_SENDER_RRD *, uint8_t *, const char *, const size_t);`  
  - 启动文件发送流程，指定待发送数据、文件名和文件大小。

- `int xymodem_sender_send_data(XYMODEM_SENDER_RRD *, uint8_t);`  
  - 发送数据包，根据接收端响应（ACK/NAK）进行数据包重发或下一个包的发送。

- `void xymodem_sender_stop_transfer(XYMODEM_SENDER_RRD *);`  
  - 主动中止传输流程。

- `void xymodem_sender_close_session(XYMODEM_SENDER_RRD *);`  
  - 结束 YMODEM 会话，发送结束包。

### 接收端（Receiver）接口
- `int xymodem_receiver_init(XYMODEM_RECEIVER_RRD *, MODEM_TYPE_RRD, MODEM_LENGTH_RRD, MODEM_VERIFY_RRD, xymodem_receiver_send_data_fn_t);`  
  - 初始化接收端对象，配置协议类型、包长、校验方式及底层发送回调。

- `void xymodem_receiver_start_transfer(XYMODEM_RECEIVER_RRD *);`  
  - 启动接收流程，向发送端发送握手信号。

- `int xymodem_receiver_unpack(XYMODEM_RECEIVER_RRD *, const uint8_t *, const size_t, uint8_t *, uint8_t *);`  
  - 解包收到的原始数据，提取有效数据包内容。

- `int xymodem_receiver_receive(XYMODEM_RECEIVER_RRD *, const uint8_t *, const size_t, uint8_t *, const size_t);`  
  - 高层接收接口，自动处理包校验、重传等，输出有效数据。

- `void xymodem_receiver_stop_transfer(XYMODEM_RECEIVER_RRD *self);`  
  - 主动中止接收流程。

---

详细用法可参考 [`test/sender/sender_test.c`](../../ModemSuite/test/sender/sender_test.c) 和 [`test/receiver/receiver_test.c`](../../ModemSuite/test/receiver/receiver_test.c) 示例代码。