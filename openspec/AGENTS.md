# OpenSpec Workflow Guide

## 概述

本文件定义了人机协作的工作流程：**用户负责架构决策和功能方向，AI 负责实现细节和代码质量**。所有代码变更必须有文档追踪。

---

## 核心原则

1. **不动已有功能**: 重构只改内部实现，不改外部行为
2. **最小变更**: 每次 PR 只做一件事
3. **可编译验证**: 每次变更后必须通过 Keil 编译
4. **先分析后动手**: 修改前先完成 PHASE 1 分析

---

## 变更工作流

### Step 1: 创建 Change Proposal

在 `openspec/changes/` 目录下创建变更提案：

```
openspec/changes/NNNN-<简短描述>.md
```

模板:

```markdown
# NNNN - 变更标题

## 状态: draft | review | approved | done

## 动机
为什么需要这个变更?

## 影响范围
- 涉及文件: ...
- 涉及模块: ...
- 风险等级: low | medium | high

## 方案
具体怎么做?

## 验证方式
怎么确认变更正确?

## 检查清单
- [ ] 编译通过
- [ ] 不影响现有功能
- [ ] 更新相关文档
```

### Step 2: 分析 (PHASE 1)

AI 必须先分析目标代码，输出：
- 当前逻辑流程
- 数据依赖关系
- 潜在风险点

用户确认分析正确后才能进入实施。

### Step 3: 实施 (PHASE 3)

- 严格按照 approved 的方案执行
- 每个文件的修改要能独立解释
- 保持编译通过

### Step 4: 验证

```powershell
& "C:\Keil_v5\UV4\UV4.exe" -b USER\UART.uvprojx -j0 -o build_log.txt
Get-Content build_log.txt -Tail 10
```

确认: `0 Error(s), 0 Warning(s)` (或 warnings 不增加)

---

## 文件修改规则

### 禁止修改

| 目录/文件                | 原因              |
|--------------------------|--------------------|
| `STM32F10x_FWLib/*`     | ST 官方库          |
| `CORE/*`                | CMSIS 内核文件     |
| `USER/stm32f10x.h`     | 芯片寄存器定义     |

### 谨慎修改 (需 Change Proposal)

| 文件                              | 原因                          |
|-----------------------------------|-------------------------------|
| `system_control.c/h`             | 核心业务逻辑，变更影响面大    |
| `usart2.c/h`                     | HMI 通信协议，影响人机交互    |
| `main.c`                         | 主循环流程，影响全局时序      |
| `stm32f10x_it.c`                | 中断服务，影响实时性          |

### 自由修改

| 范围                              | 说明                          |
|-----------------------------------|-------------------------------|
| `openspec/*`                     | 文档                          |
| 新增独立模块                      | 不影响现有代码                |
| 纯重命名 (编译器验证)            | 改名不改逻辑                  |

---

## 编码标准快查

### 命名

```c
// 函数 & 变量: camelCase
void adcProcessData(void);
uint16_t currentPressure;

// 常量 & 宏: UPPER_SNAKE_CASE
#define MAX_RETRY_COUNT  3
#define BUFFER_SIZE      256

// 结构体 & 枚举类型: PascalCase
typedef struct {
    uint16_t pressure;
    uint8_t  state;
} SensorData;

// 枚举值: UPPER_SNAKE_CASE
typedef enum {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_ERROR
} SystemState;
```

### 函数规范

```c
/**
 * @brief  简要功能描述
 * @param  paramName: 参数说明
 * @retval 0=成功, 1=超时, 2=参数错误
 */
uint8_t functionName(uint16_t paramName)
{
    // 不超过 20 行
}
```

### 禁止事项

- 数字后缀命名 (uart1Init → uartMainInit)
- 魔数 (直接写数字)
- 无超时的 while 循环
- 未经 `static` 限定的模块内部变量

---

## 调试协议 (PHASE 4)

当问题难以定位时:

1. **插桩**: 在关键路径添加 `u1_printf()` 日志
2. **编译**: Keil 编译下载
3. **观察**: 用户通过串口助手获取日志
4. **分析**: AI 根据日志输出定位根因
5. **修复**: 基于确定的根因进行修复

日志格式建议:

```c
u1_printf("[模块名] 事件: 值=%d\n", value);
```

---

## 快速命令参考

| 操作             | 命令                                                                     |
|------------------|--------------------------------------------------------------------------|
| 编译             | `& "C:\Keil_v5\UV4\UV4.exe" -b USER\UART.uvprojx -j0 -o build_log.txt` |
| 重新编译         | `& "C:\Keil_v5\UV4\UV4.exe" -r USER\UART.uvprojx -j0 -o build_log.txt` |
| 查看编译结果     | `Get-Content build_log.txt -Tail 10`                                     |
| 查看 Git 状态    | `git status`                                                             |
| 查看变更差异     | `git diff`                                                               |
