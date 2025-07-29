<div align="center">
  <img src="../images/icon.webp" height="150">
  <h1>ModemSuite</h1>
  <span>A C-based project integrating XMODEM and YMODEM file transfer protocols</span>
</div>
<br>
<div align="center">
  <img src="https://img.shields.io/badge/License-Apache2.0-green?logoColor=63%2C%20185%2C%2017&label=license&labelColor=63%2C%20185%2C%2017&color=63%2C%20185%2C%2017">
  <img src="https://img.shields.io/badge/Language-C-green?logoColor=63%2C%20185%2C%2017&labelColor=63%2C%20185%2C%2017&color=63%2C%20185%2C%2017">
</div>

## Interface Description

### Sender Interfaces

- `int xymodem_sender_init(XYMODEM_SENDER_RRD *, MODEM_TYPE_RRD, MODEM_LENGTH_RRD, MODEM_VERIFY_RRD, xymodem_sender_send_data_fn_t);`  
  - Initialize the sender object, configure protocol type, packet length, verification method, and the underlying send callback.

- `void xymodem_sender_start_transfer(XYMODEM_SENDER_RRD *, uint8_t *, const char *, const size_t);`  
  - Start the file transfer process, specifying the data to send, file name, and file size.

- `int xymodem_sender_send_data(XYMODEM_SENDER_RRD *, uint8_t);`  
  - Send a data packet. Handles retransmission or next packet sending based on receiver's response (ACK/NAK).

- `void xymodem_sender_stop_transfer(XYMODEM_SENDER_RRD *);`  
  - Actively abort the transfer process.

- `void xymodem_sender_close_session(XYMODEM_SENDER_RRD *);`  
  - End the YMODEM session and send the end packet.

### Receiver Interfaces

- `int xymodem_receiver_init(XYMODEM_RECEIVER_RRD *, MODEM_TYPE_RRD, MODEM_LENGTH_RRD, MODEM_VERIFY_RRD, xymodem_receiver_send_data_fn_t);`  
  - Initialize the receiver object, configure protocol type, packet length, verification method, and the underlying send callback.

- `void xymodem_receiver_start_transfer(XYMODEM_RECEIVER_RRD *);`  
  - Start the receiving process and send handshake signals to the sender.

- `int xymodem_receiver_unpack(XYMODEM_RECEIVER_RRD *, const uint8_t *, const size_t, uint8_t *, uint8_t *);`  
  - Unpack the received raw data and extract the valid data packet.

- `int xymodem_receiver_receive(XYMODEM_RECEIVER_RRD *, const uint8_t *, const size_t, uint8_t *, const size_t);`  
  - High-level receive interface. Automatically handles packet verification, retransmission, and outputs valid data.

- `void xymodem_receiver_stop_transfer(XYMODEM_RECEIVER_RRD *self);`  
  - Actively abort the receiving process.

---

For detailed usage, please refer to the example code in [`test/sender/sender_test.c`](../../ModemSuite/test/sender/sender_test.c) and [`test/receiver/receiver_test.c`](../../ModemSuite/test/receiver/receiver_test.c)