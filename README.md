# STM32F407 AIoT 桌面环境助手

基于 STM32F407ZGT6 与 FreeRTOS 构建的桌面环境监测终端，集成温湿度与光照采集、OLED 本地显示、按键交互、串口诊断、ESP01S WiFi 联网及 MQTT 数据上报，实现从环境感知、状态管理到云端通信的完整数据链路。

系统通过 FreeRTOS 将传感器采集、显示刷新、按键处理、WiFi 入网和 MQTT 通信拆分为独立任务，并使用统一状态模型管理环境数据、工作模式、报警及网络状态。

## 项目效果

MQTT 平台接收 STM32 发布的环境数据：

![MQTT 平台端收到环境数据 JSON](docs/images/mqtt-17-platform-json-received.png)

OLED 本地环境状态显示：

<img src="docs/images/oled-wifi-ok.jpg" alt="OLED 环境与 WiFi 状态显示" width="360">

ESP01S 完成 WiFi 入网：

![ESP01S CWJAP WiFi 入网成功](docs/images/esp01s-wifi-joined-cwjap.png)

MQTT PUBLISH 数据发送结果：

![MQTT PUBLISH SEND OK](docs/images/mqtt-16-publish-send-ok.png)

## 核心能力

- 通过 AHT30 采集温度与湿度。
- 通过板载光敏传感器 `PF7 / ADC3_IN5` 采集光照强度。
- OLED 显示温度、湿度、光照百分比和 WiFi 状态。
- KEY0 / KEY1 / KEY2 分别完成模式切换、状态诊断和报警恢复。
- USART1 输出系统状态、传感器数据和通信日志。
- ESP01S 通过 USART3 完成 AT 初始化、WiFi 入网与 IP 获取。
- STM32 本地构造 MQTT 3.1.1 CONNECT 与 PUBLISH 报文。
- 通过 TCP 连接 MQTT broker，并向 `stm32f407/env` 发布 JSON 环境数据。

## 系统架构

```text
AHT30 -------- I2C1 ----+
OLED --------- I2C1 ----+                    +--> USART1 运行日志
板载光敏 ------ ADC3 ----> STM32F407 + FreeRTOS
KEY0/1/2 ------ GPIO ----+                    +--> USART3 --> ESP01S --> WiFi --> MQTT
                              |
                              +--> AppState_t 统一状态管理
```

数据流：

```text
环境采集 -> AppState_t -> OLED / USART1 -> ESP01S -> MQTT broker
                      -> 按键模式与报警状态
```

## 硬件组成

| 模块 | 连接 / 说明 |
| --- | --- |
| 主控 | 正点原子探索者 STM32F407ZGT6 |
| RTOS | FreeRTOS / CMSIS-RTOS V2 |
| 温湿度 | AHT30，I2C1，地址 `0x38` |
| OLED | SSD1306 128x64 I2C，I2C1，地址 `0x3C` |
| 光敏 | 板载 LS1 / LIGHT_SENSOR，`PF7 / ADC3_IN5` |
| WiFi | ESP01S / ESP8266 AT 固件，`USART3 PB10/PB11` |
| 日志串口 | USART1 |
| 按键 | KEY0 模式切换，KEY1 状态诊断，KEY2 报警恢复 |

I2C1 连接：

```text
PB6 / I2C1_SCL -> AHT30 SCL + OLED SCL
PB7 / I2C1_SDA -> AHT30 SDA + OLED SDA
3.3V            -> AHT30 VCC + OLED VCC
GND             -> AHT30 GND + OLED GND
```

ESP01S 连接：

```text
PB10 / USART3_TX -> ESP01S RXD
PB11 / USART3_RX -> ESP01S TXD
GND              -> ESP01S GND
3.3V             -> ESP01S VCC / EN
```

> ESP01S 使用 3.3V 供电和 3.3V 电平，不可直接接入 5V。

## 软件设计

### FreeRTOS 任务划分

| 任务 | 职责 |
| --- | --- |
| `sensorTask` | 周期采集 AHT30 和光敏 ADC |
| `displayTask` | 根据统一状态刷新 OLED |
| `keyTask` | 处理模式切换、状态诊断和报警恢复 |
| `wifiTask` | 执行 ESP01S 初始化与 WiFi 入网 |
| `mqttTask` | 建立 TCP 连接，完成 MQTT 握手与环境数据发布 |

应用逻辑集中在 `App/` 目录，与 CubeMX 生成代码保持边界：

```text
App/app_main.c      应用入口
App/app_state.c     统一系统状态
App/app_sensor.c    AHT30 与光敏采集
App/app_display.c   OLED 页面更新
App/app_wifi.c      ESP01S AT 与 WiFi 入网
App/app_mqtt.c      MQTT CONNECT / PUBLISH
```

### 统一状态模型

`AppState_t` 统一保存以下信息：

- 温度、湿度和 AHT30 有效状态。
- ADC 原始值与光照百分比。
- `NORMAL / FOCUS / ALERT` 工作模式。
- 报警状态。
- WiFi 与 MQTT 连接状态。

OLED、串口和 MQTT 均读取同一份状态数据，保证不同输出端的数据一致性，并降低任务之间的直接耦合。

### 本地交互

| 按键 | 功能 |
| --- | --- |
| KEY0 | 切换 `NORMAL / FOCUS / ALERT` 模式 |
| KEY1 | 打印系统状态并触发 I2C 设备扫描 |
| KEY2 | 清除报警状态，必要时恢复到 `NORMAL` |

I2C 扫描结果：

```text
[i2c] found device: 0x38
[i2c] found device: 0x3C
[i2c] scan done, found=2
```

### WiFi 入网

ESP01S 通过 USART3 执行以下入网流程：

```text
AT
ATE0
AT+CWMODE_CUR=1
AT+CWJAP="YOUR_WIFI_SSID","YOUR_WIFI_PASSWORD"
AT+CIFSR
```

成功入网后，WiFi 状态同步写入 `AppState_t`，OLED 显示 `W:OK`，串口输出连接与 IP 获取结果。

### MQTT 环境数据发布

STM32 通过 ESP01S 建立到 `broker.hivemq.com:1883` 的 TCP 连接，在本地构造并发送 MQTT 3.1.1 CONNECT 与 PUBLISH 二进制报文。

发布主题：

```text
stm32f407/env
```

JSON 数据格式：

```json
{"temp":27.6,"hum":51.2,"light":97,"mode":"NORMAL"}
```

连接与发布结果：

```text
[mqtt] tcp connect OK
[mqtt] CONNECT packet SEND OK
[mqtt] CONNACK OK
[mqtt] PUBLISH packet SEND OK
```

当前固件采用 WiFi 入网后单次发布策略。周期上报与断线重连作为下一版本的运行策略扩展。

## 技术亮点

- 使用 FreeRTOS 对采集、显示、交互和网络通信进行任务化拆分。
- 使用 `AppState_t` 建立统一状态模型，保证 OLED、串口和 MQTT 数据一致。
- 同时覆盖 I2C 数字传感器、ADC 模拟采样、GPIO 按键和多路 UART 通信。
- 在 STM32 侧构造 MQTT 3.1.1 CONNECT / PUBLISH 报文，而非依赖封装后的 MQTT 模块指令。
- 针对 ESP01S 分段返回和二进制响应，实现缓存接收、text/hex 诊断及关键响应判断。
- 通过整数与小数位拼接生成 JSON 浮点字段，避免依赖嵌入式 `printf` 浮点支持。
- 应用层与 CubeMX 生成代码分离，降低外设配置重新生成时的维护成本。

## 构建方式

项目使用 CMake Preset、Ninja 和 ARM GCC：

```bash
cmake --build --preset Debug
```

生成产物：

```text
build/Debug/stm32f407_aiot_env_assistant.elf
```

## 仓库结构

```text
.
├── App/                         # 应用层代码
├── Core/                        # CubeMX 核心代码与 FreeRTOS 任务入口
├── Drivers/                     # STM32 HAL 驱动
├── Middlewares/                 # FreeRTOS 等中间件
├── docs/
│   ├── 项目功能介绍.md           # 功能设计、问题定位与演进路线
│   └── images/                  # 硬件照片与运行结果
├── tools/
│   └── check_protected_changes.sh
├── CMakeLists.txt
├── CMakePresets.json
└── stm32f407_aiot_env_assistant.ioc
```

## 详细文档

- [项目功能介绍](docs/项目功能介绍.md)
- [运行图片索引](docs/images/README.md)

## 演进路线

### 本地交互增强

- 优化 OLED 信息层级和中文界面。
- 增加启动页、设备状态页和报警页。
- 强化工作模式与网络状态的视觉反馈。

### 智能环境助手

- 扩展 CO2、空气质量和人体存在感知。
- 增加环境报警、RGB 状态提示和自动通风控制。
- 引入周期上报、断线恢复与历史趋势分析。
- 接入语音交互和云端大模型服务，实现环境问答、状态解释和设备控制。
