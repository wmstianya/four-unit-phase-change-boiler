# Project Specification

## 项目概述

**项目名称**: MCGS 相变蒸汽发生器控制系统 (多拼联控)

**产品用途**: 工业级蒸汽发生器整定控制器固件，支持单机运行和多台设备（最多 16 台）的联控模式。用于相变式蒸汽发生器的全自动点火、燃烧调节、压力控制、水位管理、安全保护以及 MCGS/HMI 触摸屏交互。

**当前版本**: V2.0.1 (2025-08-07)

**版本变更历史**:

| 版本   | 日期       | 变更内容                                                       |
|--------|------------|----------------------------------------------------------------|
| V1.0.1 | 2025-03-25 | 正式发布                                                       |
| V1.0.2 | 2025-04-03 | 手动模式总控风阀测试、各分机组增加指示灯                       |
| V1.0.3 | 2025-04-03 | 修复手动模式风机功率自动清零；修改联控与二次启动逻辑           |
| V1.0.4 | 2025-04-14 | 开机上水时间改为 60s，超压停机风机保持 33% 功率               |
| V1.0.5 | 2025-04-18 | 修正联控风阀不能开启                                           |
| V1.0.6 | 2025-04-21 | 修正联控最小需求量与在线数量冲突                               |
| V1.0.7 | 2025-05-07 | 增加五拼界面，支持 5 台联控，补水等待 100s                    |
| V1.0.8 | 2025-05-19 | 解决排烟温度报警后主机二次启动问题                             |
| V1.1.0 | 2025-05-27 | 解决联控不启动                                                 |
| V1.1.1 | 2025-07-07 | 联控排污启动压力修正                                           |
| V1.1.2 | 2025-07-12 | 待机风机功率由 30% 改为 40%                                   |
| V1.1.3 | 2025-08-02 | 增加外置联控功能，A1B1 对接 A4B4                              |
| V2.0.1 | 2025-08-07 | 串口波特率升级为 115200                                       |

---

## 技术栈

| 项目     | 详情                                               |
|----------|----------------------------------------------------|
| MCU      | STM32F103VCT6 (Cortex-M3, 72MHz, 256KB Flash, 48KB SRAM) |
| SDK      | STM32F10x Standard Peripheral Library (SPL)        |
| IDE      | Keil MDK-ARM v5                                    |
| 编译器   | ARM Compiler v5 (ARMCC)                            |
| 项目文件 | UART.uvprojx                                       |
| 晶振     | 外部 HSE 12MHz (PLL×6 = 72MHz，代码中 HSE_SetSysClk(RCC_PLLMul_9) 实际 8MHz×9) |
| 下载调试 | ST-Link (SWD 模式)                                 |
| 语言标准 | C99                                                |

---

## 硬件外设使用清单

| 外设         | 用途                              | 引脚/备注                        |
|--------------|-----------------------------------|----------------------------------|
| USART1       | RS485 A1B1 远程控制/传感器通信    | PA9(TX), PA10(RX), 9600bps      |
| USART2       | 10.1寸 HMI 触摸屏 / 4013分屏通信 | 9600bps (从机), 115200bps (主机) |
| USART3       | 多机联控 / 本地变频补水           | 9600bps                          |
| UART4        | 联控或本地通信 (外置联控)         | 9600bps                          |
| UART5        | 预留                              |                                  |
| TIM2         | 蜂鸣器 2.7kHz 方波驱动           |                                  |
| TIM3         | PWM 输出 2kHz (风机调速)          |                                  |
| TIM4         | 1ms 系统定时中断                  |                                  |
| TIM5         | 0.1ms 风机转速检测 (按需开启)     |                                  |
| ADC          | 压力变送器、温度传感器模拟采集    | PC0, PC1                        |
| SPI (软件)   | ADS1220 高精度 ADC / SPI Flash    |                                  |
| IWDG         | 独立看门狗                        | 预分频64, 重载900                |
| GPIO (输入)  | 12路数字输入 (水位探针、火焰检测、风压开关、压控信号等) | PD5-7, PB/PE 组 |
| GPIO (输出)  | 10路继电器 (风机、燃气阀、点火器、排污阀、补水泵等) | PD1-3,14-15, PA8, PC6-9 |
| GPIO (输入)  | 4位拨码地址开关 (从机地址)        | PC4,5 PA6,7                     |
| RTC          | 实时时钟 (时间同步)               |                                  |
| Flash (内部) | 系统参数掉电存储                  |                                  |

---

## 通信协议

### ModBus RTU (所有串口)

```
| 地址(1B) | 功能码(1B) | 数据(NB) | CRC16(2B) |
```

- **功能码 0x03**: 读保持寄存器
- **功能码 0x06**: 写单个寄存器
- **功能码 0x10**: 写多个寄存器
- CRC 校验: 标准 ModBus CRC16

### 主从架构

- **主机 (Address=0)**: 连接 10.1寸总控屏 (USART2)，管理所有从机
- **从机 (Address=1~6)**: 各连接 4013 分屏 (USART2)，接受主机联控调度
- 地址通过硬件拨码开关或软件配置确定

---

## 系统工作模式

| 模式              | 值 | 说明                                |
|-------------------|----|-------------------------------------|
| SYS_IDLE          | 0  | 待机模式                            |
| SYS_ALARM         | 1  | 报警模式                            |
| SYS_WORK          | 2  | 自动运行模式                        |
| SYS_MANUAL        | 3  | 手动调试模式                        |
| SYS_CLEAN_MODE    | 4  | 除垢模式                            |

---

## 设备类型 (Device_Style)

| 值 | 类型                            |
|----|---------------------------------|
| 0  | 常规单体 1吨 D级蒸汽发生器     |
| 1  | 相变单模块蒸汽发生器            |
| 2  | 相变双拼蒸汽发生器              |
| 3  | 相变四拼/五拼蒸汽发生器         |

---

## 故障代码表

| 代码 | 名称                         | 描述                                 |
|------|------------------------------|--------------------------------------|
| 0    | NO_ERROR                     | 无故障                               |
| 1    | Error1_YakongProtect         | 机械压控保护                         |
| 2    | Error2_YaBianProtect         | 压力变送器超压保护                   |
| 3    | Error3_LowGas               | 燃气低压保护                         |
| 4    | Error4_YaBianLoss            | 压力变送器断线                       |
| 5    | Error5_LowWater              | 极低水位保护                         |
| 6    | Error6_BentiWenDu_Unconnect  | 本体温度传感器未连接                 |
| 7    | Error7_FlameZiJian           | 火焰自检异常                         |
| 8    | Error8_WaterLogic            | 水位探针逻辑故障                     |
| 9    | Error9_AirPressureBad        | 风压异常                             |
| 10   | Error10_SteamValueBad        | (未使用)                             |
| 11   | Error11_DianHuo_Bad          | 点火失败                             |
| 12   | Error12_FlameLose            | 运行中火焰熄灭                       |
| 13   | Error13_AirControlFail       | 风机转速异常                         |
| 14   | Error14_BenTiValueVBad       | 本体温度超保护值                     |
| 15   | Error15_RebaoBad             | 本体过热保护                         |
| 16   | Error16_SmokeValueHigh       | 烟气温度超保护值                     |
| 17   | Error17_OutWenKong_TxBad     | 安全通信超时                         |
| 18   | Error18_SupplyWater_Error    | 进水阀故障                           |
| 19   | Error19_SupplyWater_UNtalk   | 进水阀通信丢失                       |
| 20   | Error20_XB_HighPressureYabian| 相变高压侧压力变送器超压             |
| 21   | Error21_XB_HighPressureYAKONG| 相变高压侧机械压控超压               |
| 22   | Error22_XB_HighPressureWater | 高压侧低水位                         |

---

## 编码规范 (现状 vs 目标)

### 现状

| 方面             | 现状                                                            |
|------------------|-----------------------------------------------------------------|
| 命名风格         | 混合: snake_case + 拼音 + 数字后缀 (uart2_init, Dian_Huo_Power) |
| 函数长度         | 部分函数超过 100 行                                             |
| 注释语言         | 中文注释为主，部分英文                                          |
| 全局变量         | 大量全局结构体，缺少 static 限定                                |
| 魔数             | 存在较多硬编码数值                                              |
| 头文件保护       | 有，但命名不一致 (__LED_H vs __REALYS_H)                       |
| 模块耦合         | main.h 包含所有头文件，模块间通过全局变量直接交互               |

### 重构目标

| 方面             | 目标                                                            |
|------------------|-----------------------------------------------------------------|
| 命名风格         | camelCase 函数/变量，UPPER_SNAKE 常量，PascalCase 结构体       |
| 函数长度         | 单函数不超过 20 行，超出拆分子函数                              |
| 全局变量         | 尽量 static，跨文件访问通过 getter/setter                      |
| 魔数             | 全部替换为有意义的宏或 const                                    |
| 头文件           | 统一 include guard 命名，最小化包含                             |
| 模块解耦         | 各模块只包含直接依赖的头文件，消除 main.h 统包                 |

---

## 构建方式

### Keil MDK (主要)

```powershell
& "C:\Keil_v5\UV4\UV4.exe" -b USER\UART.uvprojx -j0 -o build_log.txt
Get-Content build_log.txt -Tail 10
```

### GCC (备选)

项目包含 `STM32F103VCTx_FLASH.ld` 链接脚本和 `startup_stm32f10x_hd_gcc.s`，可通过 arm-none-eabi-gcc 工具链编译。

---

## 关键数据结构速查

| 结构体/联合体     | 文件                  | 用途                                     |
|--------------------|-----------------------|------------------------------------------|
| `SYS_INF`          | system_control.h      | 系统运行状态位域 (0x10H~0x39H 映射)      |
| `SYS_ADMIN`        | system_control.h      | 管理员参数 (前后吹扫时间、点火功率、PID等)|
| `sys_flags`        | system_control.h      | 系统运行标志量集合 (~120 个标志)         |
| `UART_DATA`        | system_control.h      | 串口收发缓冲区及状态                     |
| `SWITCH_STATUS`    | bsp_relays.h          | 继电器输出状态标志                       |
| `IO_DATA`          | system_control.h      | 12 路数字输入状态 (水位/火焰/风压等)     |
| `LCD10Struct`      | usart2.h              | 10.1寸主控屏数据交换区 (136B)            |
| `ALCD10Struct`     | usart2.h              | 联控参数交换区 (120B)                    |
| `LCD4013_Struct`   | usart2.h              | 4013 分屏数据交换区 (100B)               |
| `UPID`             | system_control.h      | PID 控制器参数与状态                     |
| `AB_EVENTS`        | system_control.h      | 运行异常事件记录                         |

---

## 目录约定

```
UART/
├── CORE/               # Cortex-M3 内核和启动文件
├── HARDWARE/           # 硬件驱动层
│   ├── ADC/            # ADC 采集驱动
│   ├── ADS1220/        # 外部高精度 ADC 驱动
│   ├── led/            # GPIO/蜂鸣器/SPI 软件模拟
│   ├── Parallel_Serial/# 并转串驱动 (输入扩展)
│   ├── PWM/            # PWM 输出驱动 (风机调速)
│   ├── relays/         # 继电器控制驱动
│   ├── timers/         # 定时器配置
│   ├── USART2/         # 串口2 + HMI 协议层 + 数据结构定义
│   ├── USART3/         # 串口3 联控通信
│   ├── USART4/         # 串口4 联控通信
│   ├── USART5/         # 串口5 预留
│   └── USR_C210/       # WiFi 模块驱动
├── STM32F10x_FWLib/    # ST 标准外设库 (不可修改)
│   ├── inc/
│   └── src/
├── SYSTEM/             # 系统级功能
│   ├── activate_key/   # CPU ID 加密/激活
│   ├── delay/          # 延时函数
│   ├── in_flash/       # 内部 Flash 读写
│   ├── iwdg/           # 独立看门狗
│   ├── out_flash/      # SPI Flash 读写
│   ├── rtc/            # RTC 时钟/日历/日期
│   ├── sys/            # 系统基础配置 (NVIC)
│   ├── system_control/ # *** 核心业务逻辑 (燃烧控制状态机) ***
│   └── usart/          # 串口1 + ModBus 协议基础
├── USER/               # 应用入口
│   ├── main.c/h        # 主循环入口
│   ├── rcc/            # RCC 时钟配置
│   ├── systick/        # SysTick 配置
│   └── UART.uvprojx    # Keil 工程文件
└── openspec/           # 项目规范文档 (本目录)
```
