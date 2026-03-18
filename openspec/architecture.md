# Architecture Document

## 系统架构总览

```
┌─────────────────────────────────────────────────────────────────────┐
│                         main() 主循环                               │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐              │
│  │ 初始化   │ │ 传感器   │ │ 通信调度 │ │ 业务逻辑 │              │
│  │ 序列     │ │ 采集     │ │ 处理     │ │ 控制     │              │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘              │
└────────┬───────────┬───────────┬───────────┬──────────────────────┘
         │           │           │           │
    ┌────▼────┐ ┌────▼────┐ ┌───▼────┐ ┌───▼──────┐
    │ HAL层   │ │ ADC/SPI │ │ UART   │ │ System   │
    │ GPIO    │ │ 驱动    │ │ ModBus │ │ Control  │
    │ Timer   │ │         │ │        │ │ (状态机) │
    └────┬────┘ └────┬────┘ └───┬────┘ └───┬──────┘
         │           │          │           │
    ┌────▼───────────▼──────────▼───────────▼──────┐
    │          STM32F10x Standard Peripheral Lib    │
    └──────────────────────────────────────────────┘
    ┌──────────────────────────────────────────────┐
    │          STM32F103VCT6 Hardware               │
    └──────────────────────────────────────────────┘
```

---

## 软件分层

### Layer 0: 硬件抽象层 (CORE + STM32F10x_FWLib)

- **职责**: CMSIS 内核访问、标准外设寄存器操作
- **文件**: `CORE/*`, `STM32F10x_FWLib/*`
- **规则**: 禁止修改

### Layer 1: 板级支持包 / 驱动层 (HARDWARE + SYSTEM 基础)

- **职责**: 将 SPL API 封装为板级功能接口
- **模块**:

| 模块              | 文件                        | 公共接口                                    |
|-------------------|-----------------------------|---------------------------------------------|
| 继电器驱动        | relays/BSP_RELAYS.c/h       | `RELAYS_GPIO_Config()`, `Send_Air_Open/Close()`, `Feed_Main_Pump_ON/OFF()`, ... |
| GPIO/蜂鸣器       | led/bsp_led.c/h             | `LED_GPIO_Config()`, `PortPin_Scan()`, `BEEP_TIME()`, `SPI_Config_Init()` |
| ADC 采集          | ADC/bsp_adc.c/h             | `ADCx_Init()`, `ADC_Process()`              |
| ADS1220           | ADS1220/ADS1220.c/h         | `ADS1220Init()`, `ADS1220Config()`          |
| PWM 风机调速      | PWM/pwm_output.c/h          | `TIM3_PWM_Init()`                           |
| 定时器            | timers/timer.c/h            | `TIM4_Int_Init()`, `TIM2_Int_Init()`, `TIM5_Int_Init()` |
| 并转串 (输入扩展) | Parallel_Serial/bsp_parallel_serial.c/h | `IO_Interrupt_Config()` |
| WiFi 模块         | USR_C210/usr_c210.c/h       | WiFi 配置与通信                             |

### Layer 2: 通信协议层 (UART + ModBus)

- **职责**: 串口初始化、ModBus RTU 帧收发与解析
- **模块**:

| 模块              | 文件                        | 协议功能                                    |
|-------------------|-----------------------------|---------------------------------------------|
| USART1 (RS485)    | SYSTEM/usart/usart.c/h      | 远程传感器、服务器通信; `ModBus_Communication()` |
| USART2 (HMI)      | HARDWARE/USART2/usart2.c/h  | 10.1寸屏/4013屏 数据交换; `Union_ModBus2_Communication()`, `ModBus2LCD4013_Lcd7013_Communication()` |
| USART3 (联控)     | HARDWARE/USART3/usart3.c/h  | 多机联控主从通信; `Modbus3_UnionTx_Communication()`, `ModBus_Uart3_LocalRX_Communication()` |
| UART4 (联控)      | HARDWARE/USART4/usart4.c/h  | 外置联控通信; `Union_Modbus4_UnionTx_Communication()`, `Union_ModBus_Uart4_Local_Communication()` |
| UART5 (预留)      | HARDWARE/USART5/usart5.c/h  | 预留扩展                                    |

### Layer 3: 系统服务层 (SYSTEM)

- **职责**: 跨模块系统服务

| 模块              | 文件                                  | 功能                           |
|-------------------|---------------------------------------|--------------------------------|
| 延时              | SYSTEM/delay/delay.c/h                | `SysTick_Delay_ms/us()`       |
| 内部 Flash        | SYSTEM/in_flash/Internal_flash.c/h    | 参数掉电保存/读取              |
| SPI Flash         | SYSTEM/out_flash/bsp_spi_flash.c/h    | 故障记录外部存储               |
| RTC 时钟          | SYSTEM/rtc/bsp_rtc.c/h               | 实时时钟初始化与读取           |
| 日历/日期         | SYSTEM/rtc/bsp_calendar.c/h, bsp_date.c/h | 日期计算                  |
| 看门狗            | SYSTEM/iwdg/bsp_iwdg.c/h             | IWDG 配置与喂狗               |
| CPU 加密          | SYSTEM/activate_key/activate_key.c/h  | CPU ID 加密验证                |
| NVIC 配置         | SYSTEM/sys/sys.c/h                    | 中断优先级分组                 |

### Layer 4: 业务逻辑层 (SYSTEM/system_control)

- **职责**: 核心控制状态机、安全保护、联控调度
- **文件**: `SYSTEM/system_control/system_control.c/h`
- **这是系统中最复杂、最核心的模块** (预估 3000+ 行)

---

## 主循环执行流程

```
main()
  │
  ├── [初始化阶段]
  │   ├── HSE_SetSysClk()         时钟配置
  │   ├── SysTick_Init()          系统滴答
  │   ├── NVIC_Configuration()    中断分组
  │   ├── LED_GPIO_Config()       GPIO 初始化
  │   ├── IO_Interrupt_Config()   输入中断配置
  │   ├── RELAYS_GPIO_Config()    继电器初始化
  │   ├── ADCx_Init()             ADC 初始化
  │   ├── ADS1220Init/Config()    外部 ADC
  │   ├── SPI_Config_Init()       SPI 总线
  │   ├── uart_init(9600)         串口1 RS485
  │   ├── uart2_init(9600)        串口2 HMI
  │   ├── uart3_init(9600)        串口3 联控
  │   ├── uart4_init(9600)        串口4 联控
  │   ├── TIM4_Int_Init(1ms)      系统定时器
  │   ├── TIM2_Int_Init()         蜂鸣器定时器
  │   ├── TIM3_PWM_Init()         PWM 风机
  │   ├── clear_struct_memory()   结构体清零
  │   ├── Write_CPU_ID_Encrypt()  加密验证
  │   ├── 拨码地址读取             确定主/从角色
  │   ├── sys_control_config()    系统参数默认值
  │   ├── 开机自检 (10s)           检查小屏连接
  │   ├── read_serial_data() ×5   预读串口数据
  │   ├── Flash_Read_Protect()    Flash 读保护
  │   └── IWDG_Config()           启动看门狗
  │
  └── [主循环] while(1)
      │
      ├── SpiReadData()                        炉内温度读取
      ├── IWDG_Feed()                          喂狗
      ├── ADC_Process()                        模拟量处理
      ├── read_serial_data()                   串口数据转接 (~15ms)
      ├── One_Sec_Check()                      1秒周期任务
      ├── ModBus_Communication()               USART1 通信处理
      ├── Relays_NoInterrupt_ON_OFF()          继电器强制处理
      │
      └── switch(Address_Number)
          │
          ├── case 0: [主控设备]
          │   ├── Union_ModBus2_Communication()          10.1寸屏通信
          │   ├── switch(Device_Style)
          │   │   ├── Modbus3_UnionTx_Communication()    联控发送
          │   │   ├── ModBus_Uart3_LocalRX_Communication() 联控接收
          │   │   ├── Union_Modbus4_UnionTx_Communication() 串口4联控
          │   │   ├── Union_ModBus_Uart4_Local_Communication()
          │   │   ├── Union_Check_Config_Data_Function() 参数校验
          │   │   ├── D50L_SoloPressure_Union_MuxJiZu_Control_Function() 联控PID
          │   │   ├── Alarm_Out_Function()               报警输出
          │   │   └── JiaYao_Supply_Function()           加药泵
          │   └──
          │
          └── case 1~6: [从机设备]
              ├── LCD4013_Data_Check_Function()          分屏数据检查
              ├── ModBus2LCD4013_Lcd7013_Communication() 分屏通信
              ├── Modbus3_UnionTx_Communication()        联控响应
              ├── ModBus_Uart3_LocalRX_Communication()
              ├── ModBus_Uart4_Local_Communication()
              ├── XiangBian_Steam_AddFunction()          相变补水
              ├── System_All_Control()                   ★ 核心状态机
              └── Fan_Speed_Check_Function()             风机转速检测
```

---

## 中断服务程序 (ISR) 映射

| 中断源           | 优先级(抢占:响应) | 处理内容                              |
|------------------|--------------------|---------------------------------------|
| USART1_IRQHandler| 2:0                | 接收字节存入 U1_Inf.RX_Data[]         |
| USART2_IRQHandler| 2:1                | 接收字节存入 U2_Inf.RX_Data[]         |
| USART3_IRQHandler| 2:2                | 接收字节存入 U3_Inf.RX_Data[]         |
| UART4_IRQHandler | 2:2                | 接收字节存入 U4_Inf.RX_Data[]         |
| TIM4_IRQHandler  | -                  | 1ms: 串口接收超时计数、各种定时标志   |
| TIM2_IRQHandler  | 0:3                | 蜂鸣器 PWM 翻转                      |
| TIM5_IRQHandler  | -                  | 0.1ms: 风机脉冲计数 (按需开启)       |

---

## 核心状态机 (System_All_Control)

```
                    ┌─────────┐
          开机──────► SYS_IDLE │◄──── 停机/后吹扫完成
                    └────┬────┘
                         │ 开机命令
                    ┌────▼────┐
                    │自检/前吹│
                    │扫阶段   │
                    └────┬────┘
                         │ 自检通过
                    ┌────▼────┐
                    │点火阶段 │◄──── 重试 (最多3次)
                    └────┬────┘
                         │ 点火成功
                    ┌────▼────┐
                    │SYS_WORK │ 自动燃烧控制
                    │ PID调压 │ 水位管理
                    │ 排污控制│ 安全监测
                    └────┬────┘
                         │ 达压停机 / 故障
                    ┌────▼────┐
                    │后吹扫   │
                    └────┬────┘
                         │ 完成
                    ┌────▼────┐
                    │SYS_IDLE │
                    └─────────┘

    任意状态 ──故障──► SYS_ALARM ──复位──► SYS_IDLE
    任意状态 ──手动──► SYS_MANUAL ──退出──► SYS_IDLE
```

---

## 数据流图

### 传感器 → 决策 → 执行器

```
[传感器]                    [决策逻辑]                    [执行器]
                                                          
压力变送器 ─── ADC ──┐                                    
水位探针 ─── GPIO ──┤      ┌──────────────┐              ┌── 风机 (PWM)
火焰检测 ─── GPIO ──┼─────►│ System_All   │──────────────├── 燃气阀 (继电器)
风压开关 ─── GPIO ──┤      │ _Control()   │              ├── 点火器 (继电器)
温度传感器 ─ SPI ───┤      │              │              ├── 补水泵 (继电器)
炉内温度 ── ADS1220─┘      │ PID + 状态机 │              ├── 排污阀 (继电器)
                            └──────────────┘              └── 蜂鸣器 (TIM2)
```

### HMI 通信数据流

```
[10.1寸主控屏]           [MCU主机]              [MCU从机]         [4013分屏]
     │                      │                      │                  │
     │◄──── USART2 ────────►│◄──── USART3 ────────►│◄── USART2 ─────►│
     │   LCD10Struct(136B)  │  Duble5_Info(30B)    │  LCD4013(100B)  │
     │   ALCD10Struct(120B) │  UtoSlave_Info(14B)  │                  │
     │                      │                      │                  │
     │                      │◄──── UART4 ──────────►│                  │
     │                      │   (外置联控扩展)      │                  │
```

---

## 已识别的架构痛点 (重构目标)

### P0 - 关键问题

| # | 问题                                              | 影响                              |
|---|---------------------------------------------------|-----------------------------------|
| 1 | `system_control.c` 单文件超过 3000 行             | 可维护性极差，状态机难以追踪      |
| 2 | `main.h` 统包所有头文件                           | 编译耦合度高，任意修改全量重编    |
| 3 | `usart2.h` 包含 7+ 个不相关数据结构体             | 职责不清，LCD/联控/协议混在一起   |
| 4 | 所有串口 ModBus 协议各自实现，无公共抽象           | 协议逻辑重复 4 次                 |

### P1 - 重要问题

| # | 问题                                              | 影响                              |
|---|---------------------------------------------------|-----------------------------------|
| 5 | 全局变量过多 (~50+ 个 extern)，无访问控制         | 任何模块可随意修改任何状态        |
| 6 | 类型定义 (uint8/uint16/uint32) 在 main.h 中自定义 | 应使用 stdint.h 标准类型          |
| 7 | 中断中直接操作业务标志位                           | 潜在竞态条件，缺少原子保护        |
| 8 | 缺少日志等级控制                                   | 调试信息无法分级过滤              |

### P2 - 改进建议

| # | 问题                                              | 影响                              |
|---|---------------------------------------------------|-----------------------------------|
| 9 | bsp_led.c 包含 SPI 软件模拟代码                   | 文件职责混乱                      |
| 10| 命名不一致 (拼音/英文/混合)                       | 新人理解成本高                    |
| 11| 无单元测试                                        | 逻辑变更无回归保障                |
| 12| 故障记录仅存 8 条                                 | 现场排查能力不足                  |

---

## 重构路线图 (建议)

### Phase 1: 文档化 + 最小侵入整理

1. 完成 openspec 文档 ✅
2. 提取公共 ModBus CRC/帧处理为独立模块
3. 将 `usart2.h` 中的数据结构拆分到各自模块

### Phase 2: 核心解耦

4. 拆分 `system_control.c` 为子模块:
   - `burner_fsm.c` — 燃烧状态机
   - `pressure_pid.c` — PID 压力控制
   - `water_manage.c` — 水位管理
   - `safety_check.c` — 安全保护
   - `union_control.c` — 多机联控调度
5. 消除 `main.h` 统包，建立最小依赖链

### Phase 3: 质量提升

6. 引入 stdint.h 替换自定义类型
7. 统一命名规范 (逐模块迁移)
8. 添加中断安全保护
9. 引入日志等级框架
10. 为关键算法 (PID、状态机) 编写单元测试
