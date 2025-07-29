<div align="center">
  <img src="./images/icon.webp" height="150">
  <h1>ModemSuite</h1>
  <span>A project integrating XMODEM and YMODEM file transfer protocols</span>
</div>
<br>
<div align="center">
  <img src="https://img.shields.io/badge/License-Apache2.0-green?logoColor=63%2C%20185%2C%2017&label=license&labelColor=63%2C%20185%2C%2017&color=63%2C%20185%2C%2017">
  <img src="https://img.shields.io/badge/Language-C-green?logoColor=63%2C%20185%2C%2017&labelColor=63%2C%20185%2C%2017&color=63%2C%20185%2C%2017">
</div>
<p align="center">
<a href="">English</a> | <a href="../README.md">简体中文</a>
</p>

## Project Introduction
**ModemSuite** is a serial file transfer protocol suite focused on embedded and general platforms, mainly implementing XMODEM, YMODEM and other protocols. It is suitable for firmware upgrades, device debugging, protocol learning, and more. The project adopts a modular architecture for easy extension and integration, supports customizable protocol parameters, and can flexibly adapt to different underlying communication environments (such as UART, TCP/IP, etc.).

## Features
- Built-in test projects, supporting TCP test cases
- Supports classic serial file transfer protocols such as XMODEM and YMODEM
- Customizable protocol parameters (packet size, retry count, verification method, etc.)
- Built-in auto-negotiation mechanism, supports retransmission, checksum/CRC verification, and various error detection and recovery strategies
- Provides a unified interface layer for easy adaptation to different serial drivers, network modules, or custom low-level implementations

## Directory Structure
- `test/`: Sender and receiver test projects
- `common/`: Common protocol implementations and basic definitions
- `interface/`: Protocol interface abstraction and declarations
- `src/`: Core protocol implementations (such as xymodem_sender/receiver)

## Quick Start

### 1. Clone the repository
```bash
git clone https://github.com/Rev-RoastedDuck/ModemSuite.git
cd ModemSuite/ModemSuite
```

### 2. Build the test projects
For example, on Linux, enter the test case directory and build:
```bash
cd test/sender
mkdir -p build
cd build
cmake ..
make
```

Do the same for the receiver:

```bash
cd ../../receiver
mkdir -p build
cd build
cmake ..
make
```

### 3. Run the tests
First, start the receiver (listening on port 12345):
```bash
cd test/receiver/build
./start
```
Then start the sender (sending files to local port 12345):
```bash
cd test/sender/build
./start
```
By default, `file.txt` will be sent and the receiver will save it as `received_file.txt`.

### 4. Integrate into your project
- Refer to `test/sender/sender_test.c` and `test/receiver/receiver_test.c`, and call interfaces such as `xymodem_sender_init` and `xymodem_receiver_init` to integrate the protocol.
- You can customize the serial/network adaptation layer as needed.

## Documentation
Please read the [User Manual](./user-guide/manual_en.md) for everything you want to know.

## License
This project **ModemSuite** is released under the Apache License 2.0. For details, please refer to [**LICENSE**](../LICENSE)