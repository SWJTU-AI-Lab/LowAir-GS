# Demo-04: MAVLink Telemetry Viewer (MAVLink 无人机遥测接入与三维显示)

本项目是 LowAir-GS (低空数字孪生系统) 的第四个演示程序，展示了如何通过 UDP 接入真实/模拟的 MAVLink 无人机遥测数据，并利用 Demo-03 建立的四级坐标转换架构（WGS84 -> ECEF -> ENU -> SCENE -> OpenGL），在三维场景中实时渲染无人机位姿和历史轨迹。

## 功能特性

1. **MAVLink Raw 遥测接入**：
   - 监听 UDP 端口（默认 `udp://:14550`），解析 MAVLink v1 原始数据流。
   - 提取 `HEARTBEAT`, `GLOBAL_POSITION_INT`, `ATTITUDE`, `SYS_STATUS` 消息。
   - 实时解析无人机状态（经纬高、姿态、飞行模式、电量等）。
2. **地理坐标转换复用**：
   - 完全复用 Demo-03 的 `CoordinateTransformer`。
   - 将 WGS84 坐标实时转换为 ENU 局部直角坐标，再映射至 OpenGL 渲染管线。
3. **三维轨迹渲染**：
   - 实时渲染无人机实体标记（含航向指示）。
   - 动态记录并绘制历史轨迹（支持清除）。
   - 提供 `OrbitCamera` 轨道相机自由观察视角。
4. **遥测数据记录**：
   - 支持将接收到的遥测数据保存为 `.jsonl` (JSON Lines) 格式的飞行日志。

## 目录结构

```text
demo04_mavlink_telemetry_viewer/
├── CMakeLists.txt              # CMake 构建配置
├── main.cpp                    # 程序入口
├── MainWindow.h/cpp            # 主界面与业务逻辑
├── RenderWidget.h/cpp          # OpenGL 渲染视口
├── camera/                     # 相机控制模块
│   └── OrbitCamera.h/cpp
├── geo/                        # 地理坐标转换核心 (复用 Demo-03)
│   ├── GeoCoordinate.h
│   ├── CoordinateTransformer.h/cpp
│   └── Wgs84Converter.h/cpp
├── render/                     # OpenGL 渲染器
│   ├── TelemetrySceneRenderer.h/cpp
│   ├── TrajectoryRenderer.h/cpp
│   └── UavMarkerRenderer.h/cpp
├── telemetry/                  # 遥测接入层
│   ├── DroneTelemetryState.h   # 统一遥测状态结构
│   ├── TelemetrySource.h       # 遥测源抽象接口
│   ├── MavlinkRawTelemetrySource.h/cpp # MAVLink 解析实现
│   └── TelemetryRecorder.h/cpp # 日志记录器
└── config/
    └── demo04_mavlink_config.json # 配置文件
```

## 编译指南

本项目依赖 Qt6 (Widgets, OpenGLWidgets)。

```bash
cd apps/demo04_mavlink_telemetry_viewer
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## 运行与模拟测试

程序支持接入真实的 PX4/ArduPilot 飞控（通过 UDP 转发），或使用内置的 Python 模拟器进行 SITL (Software In The Loop) 测试。

### 1. 启动接收端程序

```bash
cd build
./Demo04MavlinkTelemetryViewer
```

在程序界面左侧面板点击 **Connect** 按钮（默认监听 `udp://:14550`）。

### 2. 启动 Python MAVLink 模拟器

在仓库根目录下，提供了一个 Python 脚本用于模拟无人机发送 MAVLink 遥测数据（椭圆盘旋轨迹）：

```bash
python3 tools/mavlink_simulator/send_mavlink_uav_udp.py --port 14550 --hz 10
```

此时程序中应能看到绿色的 "已连接" 状态，并在三维视口中看到无人机标记和逐渐生成的青色飞行轨迹。

## 验收截图

所有验收截图均保存在仓库根目录的 `screenshots/demo04/` 文件夹下：

1. **01_app_start_connection_panel.png**：程序启动初始状态。
2. **02_mavlink_connected_telemetry.png**：成功连接 MAVLink 并接收到首批遥测数据。
3. **03_trajectory_ellipse_enu.png**：无人机飞行一段时间后，形成椭圆轨迹。
4. **04_log_recording_active.png**：点击 Start Log 按钮，开始记录飞行日志。
5. **05_orbit_camera_view.png**：旋转视角，从侧面观察立体轨迹。
6. **06_full_trajectory_telemetry_panel.png**：完整的飞行轨迹和详细的遥测状态面板数据。

## 功能边界说明

- **只读设计**：当前 Demo 仅实现**遥测接收**，未实现指令发送（如起飞、降落、航点控制），确保地面站软件不会意外干扰真实飞行器的安全。
- **协议支持**：目前仅支持 MAVLink v1 协议。当前 `MavlinkRawTelemetrySource` 实现为**教学演示用途**，暂未进行 MAVLink CRC 校验，直接分发解析结果；若需接入真实 PX4/ArduPilot 飞控或复杂网络环境，建议引入完整 MAVSDK 或补充标准 MAVLink CRC 校验逻辑，以避免误解析损坏的数据帧。
- **后续扩展方向**：完整的 MAVSDK / MAVLink v2 接入可作为 Demo-04 的扩展版本独立实现，支持 v2 长包、组件寻址和复杂任务控制，与本 Demo 的教学演示定位保持分离。
