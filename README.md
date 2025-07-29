<div align="center">
  <img src="./Docs/images/icon.webp" height="150">
  <h1>ModemSuite</h1>
  <span>一个集成了XMODEM、YMODEM 文件协议的项目</span>
</div>
<br>
<div align="center">
  <img src="https://img.shields.io/badge/License-Apache2.0-green?logoColor=63%2C%20185%2C%2017&label=license&labelColor=63%2C%20185%2C%2017&color=63%2C%20185%2C%2017">
  <img src="https://img.shields.io/badge/Language-C-green?logoColor=63%2C%20185%2C%2017&labelColor=63%2C%20185%2C%2017&color=63%2C%20185%2C%2017">
</div>
<p align="center">
<a href="./Docs/README_en.md">English</a> | <a href="">简体中文</a>
</p>

## 项目介绍
**ModemSuite** 是一个专注于嵌入式与通用平台的串口文件传输协议套件，主要实现了 XMODEM、YMODEM 等协议，适用于固件升级、设备调试、通信协议学习等多种场景。项目采用模块化架构，便于扩展和集成，支持多种协议参数自定义，能够灵活适配不同的底层通信环境（如串口、TCP/IP等）。

## 基本功能
- 内置测试工程，支持TCP测试用例
- 支持 XMODEM、YMODEM 等经典串口文件传输协议
- 支持协议参数自定义（如包大小、重试次数、校验方式等）
- 内置自动协商机制，支持重传、校验和/CRC 校验等多种错误检测与恢复策略
- 提供统一的接口层，便于与不同的串口驱动、网络通信模块或自定义底层适配

## 目录结构
- `test/`：发送端、接收端测试工程
- `common/`：协议通用实现与基础定义
- `interface/`：协议接口抽象与声明
- `src/`：协议核心实现（如 xymodem_sender/receiver）

## 快速上手
### 1. 克隆项目
```bash
git clone https://github.com/Rev-RoastedDuck/ModemSuite.git
cd ModemSuite/ModemSuite
```

### 2. 编译测试工程
以 Linux 环境为例，进入测试用例目录并编译：
```bash
cd test/sender
mkdir -p build
cd build
cmake ..
make
```

接收端同理：

```bash
cd ../../receiver
mkdir -p build
cd build
cmake ..
make
```

### 3. 运行测试
先启动接收端（监听 12345 端口）：
```bash
cd test/receiver/build
./start
```
再启动发送端（向本地 12345 端口发送文件）：
```bash
cd test/sender/build
./start
```
默认会发送 `file.txt` 文件，接收端会保存为 `received_file.txt`。

### 4. 集成到你的项目
- 参考 `test/sender/sender_test.c` 和 `test/receiver/receiver_test.c`，调用 `xymodem_sender_init`、`xymodem_receiver_init` 等接口即可实现协议集成。
- 可根据实际需求自定义串口/网络适配层。

## 文档
请阅读[帮助文档](./Docs/user-guide/manual_zh.md)，你想知道的，都在这里~

## 声明
该项目 **ModemSuite** 使用 Apache License 2.0 授权发布，具体内容请参阅 [**LICENSE**](./LICENSE)