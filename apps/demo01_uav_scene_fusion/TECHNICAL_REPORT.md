# Demo-01: UAV Scene Fusion - 技术报告

## 1. 摘要
本技术报告详细说明了 Demo-01 (UAV Scene Fusion) 的技术实现细节。该 Demo 验证了在 Qt3D 框架下，实时接收无人机遥测数据并驱动 3D 模型在基础场景中运动的可行性，为后续更复杂的场景融合奠定了基础。

## 2. 系统架构
系统采用经典的 MVC 架构思想，分为数据层、逻辑层和表现层：
- **数据层**：`UdpJsonReceiver` 负责接收和解析来自模拟器的 UDP 遥测数据。
- **逻辑层**：`MainWindow` 负责协调数据流和用户交互，将接收到的数据分发给表现层。
- **表现层**：`RenderWidget` 利用 Qt3D 构建场景图（Scene Graph），实时更新 3D 视图；左侧的 Qt Widgets 面板实时显示状态信息。

## 3. 关键技术实现
### 3.1 遥测数据接收与解析
系统使用 `QUdpSocket` 监听本地端口。当有数据到达时，触发 `readyRead` 信号。数据报被读取后，使用 `QJsonDocument` 进行解析。为了保证渲染的流畅性，数据接收在主线程的事件循环中异步处理，避免了阻塞 UI。

### 3.2 3D 场景图构建
基于 Qt3D 的实体-组件系统（ECS, Entity-Component-System），场景被构建为一个树状结构：
- **RootEntity**: 场景根节点。
- **Camera**: 观察场景的相机，由 `QOrbitCameraController` 控制。
- **Light**: 场景光源。
- **SceneGrid**: 基础网格平面，作为无人机飞行的参考基准。
- **UavEntity**: 无人机实体，包含网格组件（Mesh）、材质组件（Material）和变换组件（Transform）。

### 3.3 实时姿态更新与轨迹绘制
- **姿态更新**：接收到遥测数据后，`RenderWidget::updateUavState` 会根据位置 $(x, y, z)$ 和欧拉角 $(roll, pitch, yaw)$ 构造一个新的 `QMatrix4x4` 变换矩阵，并更新到 `UavEntity` 的 `QTransform` 组件中，从而实现无人机的实时运动。
- **轨迹绘制**：为了记录无人机的飞行路径，系统维护了一个轨迹点列表。每次位置更新时，若距离上一个轨迹点超过一定阈值，则将新点加入列表。轨迹的渲染通过动态更新 `QGeometry` 中的顶点缓冲区实现。

## 4. 性能分析
- **渲染帧率**：在基础场景下，Qt3D 能够轻松维持 60 FPS 的渲染帧率。
- **数据延迟**：本地 UDP 通信的延迟在毫秒级，JSON 解析开销极小，能够满足 20Hz 甚至更高频率的遥测数据更新需求。

## 5. 结论与展望
Demo-01 成功验证了基础的无人机场景融合方案。后续的 Demo 将在此基础上，引入更复杂的 3D 场景（如 OSGB、3D Tiles）、更高级的渲染技术（如 Gaussian Splatting）以及更丰富的业务逻辑（如任务评估、事件触发）。
