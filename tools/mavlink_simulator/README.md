# MAVLink SITL Simulator

这是一个轻量级的 Python MAVLink 模拟器，专为 `Demo-04 MAVLink Telemetry Viewer` 设计，用于在没有真实飞控硬件的情况下进行功能验证。

## 功能描述

该模拟器通过 UDP 发送 MAVLink v1 原始数据流，模拟一架多旋翼无人机在成都双流机场附近（30.7°N, 104.0°E）执行椭圆盘旋任务。

发送的消息包括：
- `HEARTBEAT` (Msg ID: 0)：心跳包，包含飞行模式和解锁状态。
- `SYS_STATUS` (Msg ID: 1)：系统状态，包含电池电量（随时间下降）。
- `ATTITUDE` (Msg ID: 30)：姿态信息，模拟随航向变化的横滚和俯仰。
- `GLOBAL_POSITION_INT` (Msg ID: 33)：全局位置，包含 WGS84 经纬度、绝对高度、相对高度、速度和航向。

## 运行环境

- Python 3.6+
- 仅使用 Python 标准库，**无需**安装 `pymavlink` 或其他第三方依赖。

## 使用方法

在仓库根目录下运行：

```bash
python3 tools/mavlink_simulator/send_mavlink_uav_udp.py
```

### 可选参数

- `--host`：目标 IP 地址（默认：`127.0.0.1`）
- `--port`：目标 UDP 端口（默认：`14550`）
- `--hz`：遥测发送频率（默认：`10.0` Hz）

例如，以 20Hz 频率发送至局域网内的另一台电脑：

```bash
python3 tools/mavlink_simulator/send_mavlink_uav_udp.py --host 192.168.1.100 --port 14550 --hz 20
```

## 与 Demo-04 的配合

1. 启动 Demo-04 程序。
2. 在连接配置面板中，确保 URL 为 `udp://:14550`，点击 **Connect**。
3. 运行本模拟器脚本。
4. Demo-04 界面将实时显示遥测数据，并在三维视口中绘制出无人机的椭圆飞行轨迹。
