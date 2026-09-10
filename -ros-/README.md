# 从零复刻麦克纳姆轮 ROS 机器人

这是一个从机械结构、STM32 电机控制开始，逐步复刻麦克纳姆轮 ROS 机器人的学习与实践记录。

目标不是只调用现成的机器人功能，而是理解并重新实现从底层电机到 ROS 速度指令之间的完整链路：

```text
ROS cmd_vel -> 通信协议 -> STM32 -> PWM/方向 -> 电机
                                      ^
                              编码器/速度反馈
```

仓库中的代码、硬件照片、接线图、问题记录和学习日志共同组成项目档案。每个阶段都尽量保留可以重新导入和编译的工程文件。

## 当前阶段

当前已经完成底层 STM32 四电机控制和基础机械记录，正在整理可复现的工程文档。

| 内容 | 状态 | 说明 |
| --- | --- | --- |
| CubeMX / CubeIDE 工程搭建 | 已完成 | STM32F103ZET6 工程 |
| 四路 PWM 输出 | 已完成 | PWM 功能已验证，频率和占空比数据待集中记录 |
| 四个电机方向控制 | 已完成 | 需要以最终装配后的机械方向为准 |
| 编码器初始化与计数 | 已完成 | 编码器方向符号已结合电机逐个确认 |
| 基础速度计算 | 已完成 | 精确计数周期、转速和误差数据待补 |
| 速度闭环 / PID | 已完成基础调试 | 最终车体装配后的参数仍需复核和记录 |
| 串口提示、数据输出和调参基础 | 已完成基础功能 | 通信命令和参数格式待整理 |
| ROS 遥控 | 未开始 | 见 [后续路线图](docs/roadmap.md) |

## 当前项目

### motor4_speed_monitor

基于 STM32F103ZET6 的四电机控制与速度监测工程。

- 工程配置：[motor4_speed_monitor.ioc](motor4_speed_monitor/motor4_speed_monitor.ioc)
- 项目说明：[motor4_speed_monitor/README.md](motor4_speed_monitor/README.md)
- 调试记录：[debuge和问题记录.md](motor4_speed_monitor/debuge和问题记录.md)
- 学习日志：[learning-log.md](docs/learning-log.md)
- 后续路线图：[roadmap.md](docs/roadmap.md)

## 硬件与过程记录

当前工程目录中的项目 README 记录了 MCU、电机驱动、电机、电源、麦克纳姆轮、车架、接线图和视频资料。

## 仓库使用方式

每完成一个可以独立验证的小功能，就在 VSCode 的源代码管理中检查 diff、暂存相关文件、提交并推送。

建议一次提交只表达一个主题：

```text
feat: add four motor PWM control
feat: add encoder speed measurement
fix: correct motor direction sign
docs: update wiring and test result
```

完成重要阶段后创建版本标签，例如：

```text
v0.1.0  PWM 输出
v0.2.0  编码器测速
v0.3.0  四电机控制
v0.4.0  基础速度闭环
```

## 目录说明

```text
fourdriver/
├─ README.md                         # 项目总览
├─ docs/
│  ├─ learning-log.md                # 按日期记录学习过程
│  └─ roadmap.md                     # 后续学习路线
├─ 图片层/                            # 硬件照片、接线图和演示视频
└─ motor4_speed_monitor/
   ├─ README.md                      # 当前 STM32 工程说明
   ├─ debuge和问题记录.md             # 故障与排查记录
   ├─ motor4_speed_monitor.ioc       # CubeMX 配置
   ├─ Core/                           # 应用代码
   ├─ Drivers/                       # STM32 驱动
   └─ Middlewares/                    # FreeRTOS
```

## 注意事项

`Debug/`、`Release/`、`.metadata/` 和编译中间文件已经加入 `.gitignore`，不会作为正常提交内容上传。CubeMX 的 `.ioc` 文件、CubeIDE 工程文件、源码、驱动和 FreeRTOS 文件需要保留，保证工程可以重新导入和编译。
