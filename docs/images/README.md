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
