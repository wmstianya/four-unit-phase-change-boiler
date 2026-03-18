# 0001 - 静态时间触发调度器 (Cyclic Executive)

## 状态: draft

## 动机

当前 `while(1)` 主循环执行时间完全不确定。`read_serial_data()` 单次可能占用 15ms，
`Union_ModBus2_Communication()` 内含阻塞式串口发送 (最坏 ~14ms)，
`Write_Admin_Flash()` 内部 Flash 擦写 ~20ms，`SPI_FLASH_SectorErase()` 最坏 ~400ms。
这些阻塞点叠加后，主循环单圈时间可从 5ms 波动到 **450ms+**，导致：

- PID 控制周期不确定 (应为 100ms 精确节拍)
- 安全检测 (火焰/超压/水位) 响应时间无上限保证
- 联控通信丢帧/超时

---

## 影响范围

- **涉及文件**: stm32f10x_it.c, main.c, 新增 scheduler.c/h, system_tick.c/h
- **涉及模块**: TIM4 ISR, 所有串口发送函数, Flash 写入, 主循环任务调度
- **风险等级**: **high** (改变了核心调度模型)

---

## PHASE 1: 阻塞函数审计

### 已确认的阻塞源 (按严重程度排序)

```
┌────┬──────────────────────────────────┬──────────────┬───────────────────────────────────────┐
│ #  │ 函数                              │ WCET 估算    │ 阻塞机制                               │
├────┼──────────────────────────────────┼──────────────┼───────────────────────────────────────┤
│ B1 │ SPI_FLASH_SectorErase()          │ ~400ms       │ SPI_FLASH_WaitForWriteEnd() 轮询 WIP  │
│ B2 │ SPI_FLASH_PageWrite()            │ ~5ms/page    │ SPI_FLASH_WaitForWriteEnd() 轮询 WIP  │
│ B3 │ SPI_FLASH_BulkErase()            │ ~25s         │ SPI_FLASH_WaitForWriteEnd() 轮询 WIP  │
│ B4 │ Write_Admin_Flash()              │ ~20ms        │ FLASH_ErasePage() + 30×ProgramWord     │
│ B5 │ Write_Internal_Flash()           │ ~10ms        │ FLASH_ErasePage() + 3×ProgramWord      │
│ B6 │ Write_Second_Flash()             │ ~15ms        │ FLASH_ErasePage() + 12×ProgramWord     │
│ B7 │ Usart_SendStr_length(USARTx,..)  │ 0.1~14ms     │ 字节级 while(USART_FLAG_TXE==RESET)   │
│    │   @9600bps, 130 字节              │ ~135ms       │                                       │
│    │   @9600bps, 30 字节               │ ~31ms        │                                       │
│    │   @115200bps, 130 字节            │ ~11ms        │                                       │
│ B8 │ u1_printf() / u2_printf()        │ 同 B7        │ vsprintf + 字节级阻塞发送             │
│    │ u4_printf()                       │              │                                       │
│ B9 │ U2_send_str()                    │ 同 B7        │ 循环调用 U2_send_byte (阻塞)          │
│B10 │ fputc() → printf()              │ 1字节/次     │ while((USART1->SR&0x40)==0)            │
│B11 │ Union_ModBus2_Communication()    │ ~14ms        │ 内部调用 Usart_SendStr_length 发送应答 │
│B12 │ ModBus_Communication()           │ ~3ms         │ CRC 计算 + 阻塞发送                   │
│B13 │ ModBus_Uart4_Local_Communication │ ~5ms         │ CRC 计算 + 阻塞发送                   │
└────┴──────────────────────────────────┴──────────────┴───────────────────────────────────────┘
```

### WCET 计算依据

```
UART 单字节发送时间:
  @9600bps:    1 start + 8 data + 1 stop = 10 bits → 1.04 ms/byte
  @115200bps:  10 bits → 0.087 ms/byte

典型帧长度:
  ModBus 应答 (联控数据):  ~130 bytes → @9600 = 135ms, @115200 = 11.3ms
  ModBus 应答 (短回复):    ~7 bytes  → @9600 = 7.3ms
  LCD 4013 分屏数据:       ~100 bytes → @9600 = 104ms

内部 Flash:
  Page Erase:  ~20ms (F103 datasheet typ)
  Word Program: ~0.7ms (52.5us typ × 多次)

SPI Flash (W25Q64):
  Sector Erase (4KB): 45~400ms
  Page Program (256B): 0.7~3ms
  Chip Erase:          25~50s
```

### 非阻塞的函数 (无需改造)

```
┌──────────────────────────────────┬──────────┬──────────────────────────┐
│ 函数                              │ WCET     │ 备注                    │
├──────────────────────────────────┼──────────┼──────────────────────────┤
│ SpiReadData()                    │ <0.5ms   │ 软件 SPI 位操作         │
│ ADC_Process()                    │ <0.2ms   │ DMA 已完成，只做换算     │
│ IWDG_Feed()                     │ <0.01ms  │ 单寄存器写              │
│ ModBusCRC16()                   │ <0.1ms   │ 纯计算 (30字节)         │
│ One_Sec_Check()                 │ <0.3ms   │ 标志判断+简单赋值       │
│ Get_IO_Inf()                    │ <0.1ms   │ GPIO 读取               │
│ System_All_Control()            │ <2ms     │ 状态机判断 (无 I/O 等待) │
│ Fan_Speed_Check_Function()      │ <0.1ms   │ 计算比较                │
│ LCD4013_Data_Check_Function()   │ <0.1ms   │ 数据搬运                │
│ Alarm_Out_Function()            │ <0.1ms   │ GPIO 设置               │
│ XiangBian_Steam_AddFunction()   │ <0.5ms   │ 补水逻辑判断            │
│ Check_Config_Data_Function()    │ <0.1ms   │ 参数范围校验            │
└──────────────────────────────────┴──────────┴──────────────────────────┘
```

---

## PHASE 2: 静态时序表设计

### 核心设计原则

1. **基础节拍**: 1ms (TIM4 中断维护 `gTickMs`)
2. **Major Frame**: 100ms (所有任务在 100ms 内完成一个完整周期)
3. **Minor Frame**: 5ms (最小调度槽位)
4. **所有 UART 发送**: 改为 DMA 或中断驱动异步发送
5. **所有 Flash 写入**: 改为状态机分步写，每步 < 1ms

### 任务分配表

```
┌──────┬─────────────────────────────────┬────────┬──────────┬───────────────────────────────┐
│ 优先 │ 任务名                           │ 周期   │ 预算     │ 包含的现有函数                 │
│ 级   │                                  │ (ms)   │ (ms)     │                               │
├──────┼─────────────────────────────────┼────────┼──────────┼───────────────────────────────┤
│  0   │ safetyTask                      │   5    │  0.5     │ Get_IO_Inf()                  │
│      │ 安全保护检测 (最高优先级)         │        │          │ 超压/水位/火焰即时判断        │
│      │                                  │        │          │ HardWare_Protect_Relays()     │
├──────┼─────────────────────────────────┼────────┼──────────┼───────────────────────────────┤
│  1   │ sensorTask                      │  10    │  1.0     │ SpiReadData()                 │
│      │ 传感器采集+换算                  │        │          │ ADC_Process()                 │
├──────┼─────────────────────────────────┼────────┼──────────┼───────────────────────────────┤
│  2   │ controlTask                     │  10    │  3.0     │ System_All_Control()          │
│      │ 燃烧状态机 + 水位管理            │        │          │ XiangBian_Steam_AddFunction() │
│      │                                  │        │          │ Water_Balance_Function()      │
│      │                                  │        │          │ Fan_Speed_Check_Function()    │
├──────┼─────────────────────────────────┼────────┼──────────┼───────────────────────────────┤
│  3   │ pidTask                         │ 100    │  1.0     │ Pid_Cal_Function()            │
│      │ PID 压力/功率调节                │        │          │ Speed_Pressure_Function()     │
│      │                                  │        │          │ (精确 100ms 节拍是 PID 正确   │
│      │                                  │        │          │  运行的前提)                  │
├──────┼─────────────────────────────────┼────────┼──────────┼───────────────────────────────┤
│  4   │ modbusRxTask                    │  10    │  2.0     │ ModBus_Communication() [仅RX] │
│      │ 所有串口接收帧解析               │        │          │ ModBus_Uart4_Local_Comm [仅RX]│
│      │ (收到完整帧 → 解析 → 更新数据)  │        │          │ ModBus_Uart3_LocalRX [仅RX]   │
│      │                                  │        │          │ Union_ModBus2_Comm [仅RX解析] │
├──────┼─────────────────────────────────┼────────┼──────────┼───────────────────────────────┤
│  5   │ modbusTxTask                    │  20    │  1.0*    │ 发送队列出队 + DMA 启动       │
│      │ 串口发送调度                     │        │          │ * 发送本身由 DMA 完成,        │
│      │ (从发送队列取帧, 启动DMA发送)    │        │          │   CPU 仅启动传输 <0.05ms      │
├──────┼─────────────────────────────────┼────────┼──────────┼───────────────────────────────┤
│  6   │ hmiTask                         │  50    │  2.0     │ LCD4013_Data_Check_Function() │
│      │ HMI 数据准备 + LCD 刷新          │        │          │ 数据打包 (不含发送)           │
│      │                                  │        │          │ 联控数据同步                  │
├──────┼─────────────────────────────────┼────────┼──────────┼───────────────────────────────┤
│  7   │ unionTask                       │  50    │  2.0     │ D50L_SoloPressure_Union...()  │
│      │ 多机联控调度                     │        │          │ Union_Check_Config_Data()     │
│      │ (仅主机 Address=0 执行)          │        │          │ JiaYao_Supply_Function()      │
├──────┼─────────────────────────────────┼────────┼──────────┼───────────────────────────────┤
│  8   │ nvStoreTask                     │ 100    │  1.0     │ 非阻塞 Flash 写入状态机       │
│      │ EEPROM/Flash 参数持久化          │        │          │ 每步仅做 1次 ProgramWord      │
│      │ (分步写入, 每步 < 1ms)           │        │          │ 或仅做 1次 SPI_SendByte       │
├──────┼─────────────────────────────────┼────────┼──────────┼───────────────────────────────┤
│  9   │ logTask                         │ 100    │  0.5     │ 环形缓冲出队 → 启动 DMA      │
│      │ EasyLogger 异步刷新              │        │          │ 每次限 32 字节                │
├──────┼─────────────────────────────────┼────────┼──────────┼───────────────────────────────┤
│ 10   │ diagnosticTask                  │ 1000   │  1.0     │ One_Sec_Check()               │
│      │ 1秒周期: 统计/诊断/看门狗        │        │          │ sys_work_time_function()      │
│      │                                  │        │          │ IWDG_Feed()                   │
│      │                                  │        │          │ Admin_Work_Time_Function()    │
│      │                                  │        │          │ overrun 统计上报              │
└──────┴─────────────────────────────────┴────────┴──────────┴───────────────────────────────┘
```

### 时序图: 100ms Major Frame 内的槽位分配

```
 0ms       5ms      10ms      15ms      20ms      25ms   ...  95ms     100ms
 │         │         │         │         │         │          │         │
 ├─────────┼─────────┼─────────┼─────────┼─────────┤          ├─────────┤
 │safety   │safety   │safety   │safety   │safety   │          │safety   │
 │sensor   │         │sensor   │         │sensor   │          │         │
 │control  │         │control  │         │control  │          │         │
 │modbusRx │modbusRx │modbusRx │modbusRx │modbusRx │          │modbusRx │
 │         │modbusTx │         │modbusTx │         │          │modbusTx │
 │hmi+union│         │         │         │hmi+union│          │         │
 │pid+nv   │         │         │         │         │          │         │
 │log      │         │         │         │         │          │         │
 │diag(1s) │         │         │         │         │          │         │
 ├─────────┼─────────┼─────────┼─────────┼─────────┤          ├─────────┤
 ~4.5ms     ~2.5ms    ~3.5ms    ~2.5ms    ~4.0ms              ~2.5ms

CPU 利用率估算 (Major Frame):
  safety:     5ms/次 × 0.5ms × 20次  = 10.0ms
  sensor:     1.0ms × 10次           = 10.0ms
  control:    3.0ms × 10次           = 30.0ms
  pid:        1.0ms × 1次            =  1.0ms
  modbusRx:   2.0ms × 10次           = 20.0ms
  modbusTx:   1.0ms × 5次            =  5.0ms
  hmi:        2.0ms × 2次            =  4.0ms
  union:      2.0ms × 2次            =  4.0ms
  nvStore:    1.0ms × 1次            =  1.0ms
  log:        0.5ms × 1次            =  0.5ms
  diag:       1.0ms × 0.1次          =  0.1ms
                                     ────────
  总计:                               ≈ 85.6ms / 100ms = 85.6% 利用率
  余量:                               ≈ 14.4ms (14.4%)
```

---

## PHASE 3: 必须改为非阻塞的函数清单

### 改造 1: UART 发送 → DMA 异步 [最高优先级]

**现状**: 5 路 UART 全部字节级阻塞发送

**目标**: 所有 UART TX 通过 DMA + 发送完成中断实现异步

```c
/* 新增 uart_async.c/h */

#define TX_QUEUE_DEPTH 4

typedef struct {
    uint8_t  buf[TX_QUEUE_DEPTH][300];
    uint16_t len[TX_QUEUE_DEPTH];
    uint8_t  head;
    uint8_t  tail;
    uint8_t  busy;  /* DMA 正在发送中 */
} UartTxQueue;

/* 非阻塞入队 (控制任务调用) */
uint8_t uartTxEnqueue(UartTxQueue *q, uint8_t *data, uint16_t len);

/* 从队列启动 DMA 发送 (modbusTxTask 调用) */
void uartTxStartDma(USART_TypeDef *usart, DMA_Channel_TypeDef *dma, UartTxQueue *q);

/* DMA 发送完成回调 (DMA ISR 中调用) */
void uartTxDmaComplete(UartTxQueue *q);
```

**DMA 通道分配 (STM32F103VCT6)**:

```
USART1_TX → DMA1_Channel4
USART2_TX → DMA1_Channel7  (已有注释代码, 只需启用)
USART3_TX → DMA1_Channel2
UART4_TX  → DMA2_Channel5
UART5_TX  → 无 DMA, 改用 TXE 中断驱动
```

**受影响函数 (必须替换)**:

| 原函数                     | 替换方式                           |
|----------------------------|------------------------------------|
| `u1_printf()`              | `u1AsyncSend(buf, len)` → 入队     |
| `u2_printf()`              | `u2AsyncSend(buf, len)` → 入队     |
| `u4_printf()`              | `u4AsyncSend(buf, len)` → 入队     |
| `U2_send_str()`            | `u2AsyncSend()`                    |
| `U2_send_byte()`           | 废弃, 合并到异步发送               |
| `Usart_SendByte()`         | 废弃                               |
| `Usart_SendStr_length()`   | `uartAsyncSend(usart, buf, len)`   |
| `Usart_SendString()`       | 废弃                               |
| `fputc()` (printf重定向)   | 改为写入 log 环形缓冲             |

### 改造 2: 内部 Flash 写入 → 分步状态机

**现状**: `Write_Admin_Flash()` 同步擦除+写30个Word (~20ms)

**目标**: 拆成状态机, 每步仅做 1 次 Flash 操作

```c
/* 新增 nv_store.c/h */

typedef enum {
    NV_IDLE,
    NV_ERASE_UNLOCK,
    NV_ERASE_EXEC,
    NV_ERASE_LOCK,
    NV_WRITE_UNLOCK,
    NV_WRITE_WORD,    /* 每次进入只写 1 个 Word */
    NV_WRITE_LOCK,
    NV_DONE
} NvState;

typedef struct {
    NvState  state;
    uint32_t pageAddr;
    uint32_t *srcData;      /* 指向 Sys_Admin 等 */
    uint32_t *dstAddr;      /* Flash 目标地址数组 */
    uint8_t  totalWords;
    uint8_t  currentWord;
    uint8_t  dirty;         /* 业务层标记脏数据 */
} NvStoreCtx;

/* nvStoreTask 每 100ms 调用一次, 每次推进一步 */
void nvStoreStep(NvStoreCtx *ctx);
```

**WCET 保证**: 每步 < 1ms (单次 FLASH_ProgramWord ~52us, 单次 ErasePage ~20ms 需拆为独立步)

**关键约束**: Flash Erase 无法中断, 必须占用完整 ~20ms。解决方案:
- Erase 步骤安排在空闲时 (无安全关键任务的窗口)
- 或使用双 Page 乒乓策略: 一个 Page 已擦除可立即写入, 另一个Page 在空闲时擦除

### 改造 3: SPI Flash 写入 → 分步状态机

**现状**: `SPI_FLASH_SectorErase()` 阻塞等待 WIP 清零 ~400ms

**目标**: WIP 轮询变为非阻塞检查

```c
/* 改造 bsp_spi_flash.c */

typedef enum {
    SF_IDLE,
    SF_ERASE_CMD,      /* 发送擦除命令 (<0.1ms) */
    SF_ERASE_WAIT,     /* 每次进入检查 WIP, 未完成则退出 */
    SF_WRITE_ENABLE,
    SF_WRITE_PAGE,     /* 发送 256B 页写命令 (<1ms) */
    SF_WRITE_WAIT,     /* 检查 WIP */
    SF_DONE
} SpiFlashState;

/* 非阻塞: 返回 0=未完成, 1=完成 */
uint8_t spiFlashStep(SpiFlashState *state, ...);
```

### 改造 4: fputc / printf → 日志环形缓冲

**现状**: `printf` → `fputc` → 阻塞等待 USART1 TXE

**目标**: `printf` 替换为 EasyLogger 宏, 写入环形缓冲, logTask 异步 DMA 发送

```c
/* 日志级别过滤 */
#define LOG_LVL_RUN   LOG_LVL_WARN   /* 运行版仅输出 WARN 以上 */
#define LOG_LVL_DBG   LOG_LVL_DEBUG  /* 调试版输出全部 */

/* 环形缓冲限流 */
#define LOG_FLUSH_MAX_BYTES  32  /* logTask 每次最多发送 32 字节 */
```

### 改造 5: ModBus 通信函数拆分 RX/TX

**现状**: `Union_ModBus2_Communication()` 内部 解析帧 + 立即阻塞发送应答

**目标**: 拆为 RX 阶段 (解析+填充应答缓冲) 和 TX 阶段 (异步发送)

```
原流程:
  收到帧 → CRC校验 → 解析 → 组装应答 → 阻塞发送 (整段在一个函数里)

新流程:
  modbusRxTask:  收到帧 → CRC校验 → 解析 → 组装应答 → 入发送队列
  modbusTxTask:  发送队列出队 → 启动 DMA → 返回 (CPU 无阻塞)
```

---

## PHASE 4: 实施顺序

```
┌────────────────────────────────────────────────────────────────────────┐
│ Step 0 (前置): 修复 TIM4 ISR 已知 bug (1-2天)                        │
│   ✅ time_300ms 400→300                                               │
│   ✅ sys_control_time 下溢保护                                        │
│   ✅ sys_delay_time 下溢保护                                          │
│   ✅ 编译验证: 0 Error                                                │
├────────────────────────────────────────────────────────────────────────┤
│ Step 1: ISR 瘦身 (3天)                                                │
│   ✅ TIM4 ISR → 只维护 gTickMs + 串口超时                            │
│   ✅ 所有时间标志 → system_tick.c:timingUpdate() 在主循环调用         │
│   ✅ 编译验证: 功能不变                                               │
├────────────────────────────────────────────────────────────────────────┤
│ Step 2: UART DMA 异步发送 (5天)                                       │
│   ✅ 实现 uart_async.c/h (发送队列 + DMA 启动)                       │
│   ✅ USART1/2/3/UART4 的 DMA TX 初始化                               │
│   ✅ 替换所有阻塞发送函数                                             │
│   ✅ 全量回归测试: 所有串口通信正常                                   │
├────────────────────────────────────────────────────────────────────────┤
│ Step 3: Flash 写入分步化 (3天)                                        │
│   ✅ 实现 nv_store.c/h (内部 Flash 状态机)                           │
│   ✅ 改造 SPI Flash 写入为非阻塞                                      │
│   ✅ 断电 EEPROM 双副本+CRC 保护                                     │
│   ✅ 验证: 参数掉电保存正确                                          │
├────────────────────────────────────────────────────────────────────────┤
│ Step 4: ModBus RX/TX 分离 (3天)                                       │
│   ✅ 所有 ModBus_Communication 拆为 RX 解析 + TX 入队                │
│   ✅ modbusTxTask 统一管理所有串口的发送                              │
│   ✅ 验证: 联控/HMI/远程通信正常                                     │
├────────────────────────────────────────────────────────────────────────┤
│ Step 5: 调度器上线 (2天)                                              │
│   ✅ 实现 scheduler.c/h (静态任务表 + overrun 检测)                  │
│   ✅ 注册所有任务到调度表                                             │
│   ✅ main while(1) → schedDispatch()                                 │
│   ✅ 验证: overrun 计数 = 0                                          │
├────────────────────────────────────────────────────────────────────────┤
│ Step 6: 集成 EasyLogger + nvStore (2天)                               │
│   ✅ EasyLogger 对接异步 UART 输出                                    │
│   ✅ nvStoreTask 周期性检查 dirty 标志                                │
│   ✅ logTask 限流刷新                                                 │
│   ✅ 全系统集成测试                                                   │
└────────────────────────────────────────────────────────────────────────┘

总工期估算: 约 3 周 (含测试验证)
```

---

## 验证方式

### 编译验证

```powershell
& "C:\Keil_v5\UV4\UV4.exe" -b USER\UART.uvprojx -j0 -o build_log.txt
Get-Content build_log.txt -Tail 10
# 期望: 0 Error(s)
```

### 时序验证

```c
/* 在 scheduler.c 中实现 */
typedef struct {
    uint32_t wcet;     /* 最坏执行时间 (us) */
    uint32_t avg;      /* 平均执行时间 (us) */
    uint16_t overrun;  /* 超限次数 */
    uint16_t runCount; /* 执行次数 */
} TaskMetrics;

/* DWT cycle counter 测量每个任务的真实执行时间 */
#define DWT_ENABLE()  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; \
                      DWT->CYCCNT = 0; DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk
#define DWT_GET_US()  (DWT->CYCCNT / 72)  /* 72MHz → 1 cycle = 13.9ns */
```

### 功能验证

- [ ] 所有 5 路 ModBus 通信正常 (无丢帧)
- [ ] HMI 触摸屏数据实时刷新
- [ ] 联控模式多机调度正常
- [ ] PID 压力控制稳定 (100ms 精确节拍)
- [ ] 参数掉电保存/上电恢复正确
- [ ] 安全保护响应 < 10ms (5ms 任务周期 + 处理时间)
- [ ] overrun 计数长期为 0
- [ ] 看门狗不误触发

### 检查清单

- [ ] 编译通过 (0 Error)
- [ ] 不影响现有功能 (全量回归)
- [ ] 更新 openspec/architecture.md 中的调度模型图
- [ ] 更新 openspec/project.md 中的版本号
