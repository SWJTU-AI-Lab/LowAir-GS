# Demo-01: UAV Scene Fusion Qt

这是一个基于 Qt C++ 的桌面端演示程序，展示了基础框架、内置场景生成与局部坐标系下外部无人机实时数据的遥测可视化。

## 项目说明

本项目实现了：
- **零外部三维库依赖**：仅依赖 Qt6，无需 Assimp、PCL 或 OSG，保持核心框架清晰轻量
- **内置简化场景**：由程序内部生成的低空场景，包含地面网格、坐标轴、建筑块、起降圆环和参考航线
- 基于 QOpenGLWidget 的三维场景交互渲染
- UDP JSON 遥测数据实时接收（局部 SCENE 坐标）
- 无人机三维位置、姿态和历史轨迹的可视化
- 深色主题（Dark Mode）状态面板 UI 实时刷新

## 依赖环境

- Qt 6 (Core, Gui, Widgets, OpenGLWidgets, Network)
- CMake 3.16+
- C++17 或 C++20 编译器 (如 GCC, Clang, MSVC)
- Python 3 (用于运行模拟器脚本)

## 编译方法

在 Linux / macOS 下：

```bash
cd apps/uav_scene_fusion_qt
mkdir build && cd build
cmake ..
make -j4
```

## 运行方法

1. 启动 Qt 程序：
```bash
./UavSceneFusionQt
```

2. 在程序界面左侧点击 **"Start UDP (14580)"** 按钮，开始监听 UDP 端口。

3. 启动 Python 模拟器发送数据：
```bash
python3 ../../tools/telemetry_simulator/send_uav_udp.py
```

4. 此时，Qt 界面中的三维窗口应能看到一架无人机正在绕圈飞行，并绘制出橙色的历史轨迹线。左侧面板将实时更新遥测数据。

## UDP 数据格式

Qt 程序默认监听 UDP 端口 `14580`。发送的数据需为 JSON 格式：

```json
{
  "uav_id": "sim_01",
  "timestamp_ms": 1710000000123,
  "frame": "SCENE",
  "x": 50.0,
  "y": 30.0,
  "z": 20.0,
  "roll": 0.0,
  "pitch": 0.0,
  "yaw": 90.0,
  "battery": 88.0
}
```

**坐标系说明：** 当前第一版直接使用 `SCENE` 坐标，即发送的 x, y, z 直接映射到三维场景中。

## Python 模拟器使用方法

模拟器脚本位于 `tools/telemetry_simulator/send_uav_udp.py`。
它会每秒发送 20 次 UDP JSON 数据，模拟无人机沿椭圆轨迹飞行，高度和姿态也会有轻微变化。

```bash
# 默认发送到 127.0.0.1:14580
python3 send_uav_udp.py

# 查看参数帮助
python3 send_uav_udp.py --help
```

## 常见问题

- **编译找不到 Qt6：** 确保已安装 Qt6 开发包。Ubuntu 下可通过 `sudo apt install qt6-base-dev` 安装。
- **界面无数据刷新：** 检查是否点击了 "Start UDP" 按钮，并确认模拟器脚本运行正常，且端口 14580 未被其他程序占用。
- **三维场景视角控制：** 鼠标左键拖拽可旋转视角，鼠标滚轮可缩放视角。
