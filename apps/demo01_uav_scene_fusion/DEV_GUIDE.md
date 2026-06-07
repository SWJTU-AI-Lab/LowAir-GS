# Demo-01: UAV Scene Fusion - 二次开发指南

## 1. 架构概览
Demo-01 基于 Qt Widgets 和 Qt3D 框架开发，主要包含以下核心模块：
- **MainWindow**: 主窗口，负责界面布局和事件连接。
- **RenderWidget**: 3D 渲染组件，封装了 Qt3D 的场景图构建、相机控制和轨迹绘制。
- **UdpJsonReceiver**: 遥测数据接收模块，负责监听 UDP 端口并解析 JSON 格式的遥测数据。

## 2. 核心类说明
### 2.1 UdpJsonReceiver
负责网络通信和数据解析。
- `startListening(quint16 port)`: 开始监听指定端口。
- `stopListening()`: 停止监听。
- `processPendingDatagrams()`: 处理接收到的 UDP 数据报，解析 JSON 并发射 `dataReceived` 信号。

### 2.2 RenderWidget
负责 3D 场景的渲染和更新。
- `updateUavState(QString id, double x, double y, double z, double r, double p, double y_aw, double battery)`: 接收遥测数据，更新无人机模型的位置和姿态，并添加新的轨迹点。
- `clearTrajectory()`: 清除所有已绘制的轨迹。
- `resetView()`: 重置相机到默认视角。

## 3. 扩展与修改
### 3.1 添加新的遥测字段
如果需要在遥测数据中添加新的字段（如速度、GPS 状态等），请按照以下步骤进行：
1. 修改 `tools/telemetry_simulator/send_uav_udp.py`，在发送的 JSON 数据中添加新字段。
2. 修改 `UdpJsonReceiver::processPendingDatagrams()`，解析新字段。
3. 更新 `UdpJsonReceiver::dataReceived` 信号的参数列表。
4. 修改 `MainWindow::onDataReceived()` 以在界面上显示新字段。

### 3.2 替换无人机模型
当前使用的是内置的简单几何体（圆柱体组合）作为无人机模型。若需替换为真实的 3D 模型（如 .obj 或 .gltf），请修改 `RenderWidget::setupUavModel()`：
```cpp
// 移除原有的 QExtrudedMesh
// 添加 QMesh 并加载外部模型
Qt3DRender::QMesh *uavMesh = new Qt3DRender::QMesh();
uavMesh->setSource(QUrl::fromLocalFile("/path/to/your/model.obj"));
m_uavEntity->addComponent(uavMesh);
```

### 3.3 自定义场景
默认场景是一个带有网格的平面。可以在 `RenderWidget::setupScene()` 中添加更多的实体（如建筑物、地形等）来丰富场景。

## 4. 编译说明
本项目使用 CMake 进行构建。若添加了新的源文件，请在 `CMakeLists.txt` 中相应地更新 `add_executable` 的文件列表。
```cmake
add_executable(Demo01UavSceneFusion
    main.cpp
    MainWindow.cpp
    RenderWidget.cpp
    telemetry/UdpJsonReceiver.cpp
    # 添加新的 .cpp 文件
)
```
