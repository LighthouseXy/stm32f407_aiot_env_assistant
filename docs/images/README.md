# 图片与验证证据归档

本目录用于保存项目开发过程中的硬件照片、接线照片、串口截图、OLED 显示照片和功能验证证据。

图片文件按功能命名，便于在 README、项目计划和简历项目说明中引用。

## 硬件与模块照片

| 文件 | 内容 | 用途 |
| --- | --- | --- |
| `dev-board-overview.jpg` | 探索者 V3 STM32F407 开发板整体图 | GitHub README 和硬件说明 |
| `aht30-module-wiring.jpg` | AHT30 模块接线图 | AHT30 硬件连接说明 |
| `oled-module.jpg` | OLED 模块照片 | OLED 模块说明和接线记录 |

## AHT30 温湿度验证

| 文件 | 内容 | 用途 |
| --- | --- | --- |
| `aht30-temperature-serial.png` | AHT30 温度串口日志 | 证明温度采集正常 |
| `aht30-humidity-serial.png` | AHT30 湿度串口日志 | 证明湿度采集正常 |

## KEY0 模式切换验证

| 文件 | 内容 | 用途 |
| --- | --- | --- |
| `key0-mode-switch-led-on.png` | KEY0 模式切换验证截图 | 证明模式切换或状态变化有效 |
| `key0-mode-switch-demo.mp4` | KEY0 模式切换视频 | 保存动态功能证据 |
| `key0-regression-after-key1.png` | KEY1 修改后 KEY0 回归验证截图 | 证明新增 KEY1 后 KEY0 功能未破坏 |

## KEY1 状态打印验证

| 文件 | 内容 | 用途 |
| --- | --- | --- |
| `key1-before-press-normal-log.png` | KEY1 按下前正常运行日志 | 作为状态打印前的基准 |
| `key1-status-print.png` | KEY1 状态打印截图 | 证明 KEY1 可输出系统状态 |
| `key1-long-press-no-repeat.png` | KEY1 长按不重复触发截图 | 证明按键防重复触发逻辑有效 |
| `key1-status-fixed-temp-hum.png` | KEY1 温湿度字符串格式修复验证 | 证明温湿度格式化显示正常 |

## KEY2 报警清除验证

| 文件 | 内容 | 用途 |
| --- | --- | --- |
| `key2-alert-clear.png` | KEY2 清除报警截图 | 证明 KEY2 可将 ALERT 切回 NORMAL 并清除报警标志 |

## I2C 与 OLED 验证

| 文件 | 内容 | 用途 |
| --- | --- | --- |
| `i2c-scan-aht30-oled.png` | I2C 扫描发现 AHT30 和 OLED | 证明 I2C1 上存在 `0x38` 和 `0x3C` 两个设备 |
| `oled-test-pattern.jpg` | OLED 测试图案显示 | 证明 OLED 初始化和写屏链路正常 |

## 板载光敏传感器验证

| 文件 | 内容 | 用途 |
| --- | --- | --- |
| `light-sensor-oled-bright.jpg` | OLED 光照数据显示证据之一 | 证明 OLED 能显示光照百分比 |
| `light-sensor-oled-covered.jpg` | OLED 光敏遮挡/变化显示证据之一 | 证明遮挡/照光时显示状态有变化 |
| `light-sensor-serial-percent.png` | 串口 `light_raw / light_percent` 变化日志 | 证明 ADC 原始值和百分比换算已经工作 |
| `light-sensor-adc-config-or-log.png` | 光敏 ADC 配置或中间验证截图 | 保存配置或中间验证证据 |

## ESP01S AT 通信验证

| 文件 | 内容 | 用途 |
| --- | --- | --- |
| `esp01s-at-ok.png` | ESP01S USART3 AT + OK 最小通信成功截图 | 证明 STM32 通过 `USART3` 成功收到 ESP01S 的 `OK` 回复 |

## ESP01S WiFi 入网验证

涉及明文 WiFi 密码的原始截图不直接归档，公开文档只使用脱敏截图或不含密码的局部裁剪图。

| 文件 | 内容 | 用途 |
| --- | --- | --- |
| `esp01s-at-workflow-ate0-cwmode.png` | ESP01S `AT / ATE0 / CWMODE` 基础流程截图 | 证明入网前 AT 指令封装可连续执行 |
| `esp01s-wifi-joined-cwjap.png` | ESP01S `CWJAP` 成功截图 | 证明 WiFi 入网成功 |
| `esp01s-cifsr-ip.png` | ESP01S `CIFSR` 获取 IP 截图 | 证明已获取 IP 地址 |
| `serial-wifi-connected.png` | 串口 WiFi connected 日志截图 | 证明 `wifi_status` 写入和串口日志同步 |
| `oled-wifi-off.jpg` | OLED `W:OFF` 状态显示照片 | 证明 OLED 可显示 WiFi 关闭状态 |
| `oled-wifi-conn.jpg` | OLED `W:CONN` 状态显示照片 | 证明 OLED 可显示 WiFi 连接中状态 |
| `oled-wifi-ok.jpg` | OLED `W:OK` 状态显示照片 | 证明 OLED 可显示 WiFi 已连接状态 |
| `serial-wifi-connecting.png` | 串口 `wifi=CONNECTING` 状态同步截图 | 证明串口日志可同步 WiFi 连接中状态 |

## MQTT 最小发布验证

当前 MQTT 验证范围是 CONNECT / CONNACK / 单次 PUBLISH，不包含长期周期发布。

| 文件 | 内容 | 用途 |
| --- | --- | --- |
| `mqtt-01-at-timeout.png` | MQTT 前置 AT 探测连续超时 | 记录 ESP01S 入网后短时间不可响应的现象 |
| `mqtt-02-cipmux-blocked.png` | MQTT 流程停在 `AT+CIPMUX=0` | 记录早期接收窗口和任务调度问题 |
| `mqtt-03-link-invalid-after-close.png` | TCP 连接后被关闭，`link is not valid` | 证明 CONNECT 发送时机过慢会导致链路失效 |
| `mqtt-04-cipmux-timeout.png` | `AT+CIPMUX=0` 偶发超时 | 记录需要重试机制的原因 |
| `mqtt-05-connect-send-ok.png` | MQTT CONNECT 报文发送成功 | 证明 `AT+CIPSEND` 和 payload 发送流程可用 |
| `mqtt-06-connack-timeout.png` | CONNECT 已发送但 CONNACK 单独等待超时 | 记录拆分接收窗口导致漏读的中间问题 |
| `mqtt-07-cipmux-retry-timeout.png` | 重试机制下的 CIPMUX 超时 | 记录 MQTT 任务需要可恢复重试 |
| `mqtt-08-cipstart-partial-timeout.png` | `AT+CIPSTART` 只收到部分响应 | 记录串口接收窗口受调度影响的现象 |
| `mqtt-09-connect-send-ok-waiting-connack.png` | CONNECT 发送成功但 CONNACK 未被解析 | 记录修正前的中间状态 |
| `mqtt-10-connack-ok.png` | `+IPD,4: 20 02 00 00` 和 `CONNACK OK` | 证明 MQTT CONNECT / CONNACK 最小闭环成功 |
| `mqtt-11-reboot-retry-connack-ok.png` | 重启后通过 AT 探测重试并再次 CONNACK OK | 证明重试逻辑可以恢复一次 ESP 未就绪状态 |
| `mqtt-12-connect-send-ok-tail-missed.png` | CONNECT 阶段只采到 `D OK` 尾部 | 记录 SEND OK 被截断时需要兼容判断 |
| `mqtt-13-publish-send-ok-tail-missed.png` | PUBLISH 阶段只采到 `Recv xx bytes` 片段 | 记录 PUBLISH 发送确认被拆分的现象 |
| `mqtt-14-publish-cipsend-prompt-timeout.png` | PUBLISH `CIPSEND` 提示符超时 | 记录 CONNACK 二进制尾巴影响下一条 AT 指令 |
| `mqtt-15-connack-ipd-incomplete.png` | 只收到 `+IPD,4:`，CONNACK 未补全 | 记录需要补收 CONNACK 的原因 |
| `mqtt-16-publish-send-ok.png` | 串口显示 `PUBLISH packet SEND OK` | 证明 STM32 已通过 ESP01S 发出 MQTT PUBLISH 报文 |
| `mqtt-17-platform-json-received.png` | HiveMQ WebSocket Client 收到 JSON | 证明平台端订阅 `stm32f407/env` 可收到环境数据 |
