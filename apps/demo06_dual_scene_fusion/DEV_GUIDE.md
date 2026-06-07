# Demo-06: Dual Scene Fusion - 二次开发指南

## 1. 架构概览
Demo-06 是一个典型的异构渲染引擎融合项目。它在一个 Qt 应用程序中同时运行了两个完全不同的渲染管线：
1. **Qt3D 渲染管线**：基于 `Qt3DWindow`，负责场景图管理、OSGB 模型渲染、无人机模型加载和轨迹绘制。
2. **原生 OpenGL 渲染管线**：基于 `QOpenGLWidget`（复用 Demo-05 的 `GaussianSplatWidget`），负责 3DGS 模型的高性能光栅化渲染。

## 2. 核心挑战与解决方案
### 2.1 异构窗口的 UI 集成
`Qt3DWindow` 继承自 `QWindow`，而 `QOpenGLWidget` 继承自 `QWidget`。在 Qt 中，将 `QWindow` 嵌入到基于 `QWidget` 的布局中需要特殊处理。
在 `MainWindow` 中，我们使用 `QWidget::createWindowContainer` 将 Qt3D 窗口转换为普通的 Widget：
```cpp
Qt3DExtras::Qt3DWindow *view3d = new Qt3DExtras::Qt3DWindow();
QWidget *container3d = QWidget::createWindowContainer(view3d);

GaussianSplatWidget *gsWidget = new GaussianSplatWidget();

QSplitter *splitter = new QSplitter(Qt::Horizontal);
splitter->addWidget(container3d);
splitter->addWidget(gsWidget);
```

### 2.2 相机同步机制
为了实现双视口的联动，必须保持两个相机的外部参数（View Matrix）和内部参数（Projection Matrix）一致。
我们以 Qt3D 的相机为主相机（Master），3DGS 相机为从相机（Slave）。
- 监听 Qt3D 相机的变换信号（如 `positionChanged`, `viewCenterChanged`）。
- 在信号槽中，提取 Qt3D 相机的位置、目标点和上向量，计算出 View Matrix。
- 将计算出的 View Matrix 传递给 `GaussianSplatWidget`，更新其内部的 Uniform 变量。

```cpp
connect(camera, &Qt3DRender::QCamera::positionChanged, this, [=]() {
    if (m_syncCamera) {
        gsWidget->setCameraPosition(camera->position());
        gsWidget->setCameraLookAt(camera->viewCenter());
    }
});
```

## 3. 扩展与修改
### 3.1 坐标系统一
如果发现两侧视口的模型没有对齐，请检查两个模型的坐标系原点。通常，OSGB 模型可能使用地理坐标（如 EPSG:4549），而 3DGS 模型使用局部坐标。需要在加载时应用一个偏移矩阵（Offset Matrix）进行坐标转换（参考 Demo-03）。

### 3.2 性能优化
同时运行两个 3D 渲染引擎对 GPU 压力极大。
- **限制帧率**：可以通过在 `GaussianSplatWidget` 的 `paintGL` 中加入节流逻辑，限制其最高帧率（如 30 FPS），以保证 UI 的响应性。
- **共享上下文**：在未来的版本中，可以探索通过 OpenGL Context Sharing，将 3DGS 渲染结果作为一个 Texture 传递给 Qt3D，实现真正的画中画（Picture-in-Picture）融合，而不是简单的左右分屏。
