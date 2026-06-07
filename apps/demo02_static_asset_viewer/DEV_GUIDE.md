# Demo-02: Static Asset Viewer - 二次开发指南

## 1. 架构概览
Demo-02 主要用于静态 3D 资产的加载与展示，基于 Qt3D 构建。核心模块包括：
- **MainWindow**: 主窗口，负责界面布局、模型加载对话框和状态显示。
- **RenderWidget**: 3D 渲染组件，封装了 Qt3D 的场景图构建、模型加载（`QSceneLoader`）和相机控制。

## 2. 核心类说明
### 2.1 RenderWidget
负责 3D 场景的渲染和资产加载。
- `loadModel(const QString &filePath)`: 接收文件路径，使用 `Qt3DRender::QSceneLoader` 加载外部 3D 模型。
- `resetView()`: 重置相机到默认视角。
- `setupScene()`: 初始化场景根节点、光源和相机。

## 3. 扩展与修改
### 3.1 支持更多模型格式
Qt3D 的 `QSceneLoader` 底层依赖于 Assimp 库，默认支持多种常见格式（如 OBJ, GLTF, FBX 等）。若需增强格式支持或自定义加载逻辑，可以：
1. 捕获 `QSceneLoader::statusChanged` 信号，处理加载错误。
2. 遍历加载后的实体树（Entity Tree），手动调整材质或应用自定义 Shader。

### 3.2 调整光照和材质
当前使用的是简单的定向光。若需实现更真实的渲染效果（如 PBR 材质），请在 `RenderWidget` 中进行修改：
```cpp
// 替换默认材质为 PBR 材质
Qt3DExtras::QMetalRoughMaterial *pbrMaterial = new Qt3DExtras::QMetalRoughMaterial();
pbrMaterial->setBaseColor(QColor(Qt::gray));
pbrMaterial->setMetalness(0.5f);
pbrMaterial->setRoughness(0.5f);
// 将其应用到加载的模型实体上
```

## 4. 编译说明
本项目使用 CMake 进行构建。若添加了新的源文件，请在 `CMakeLists.txt` 中相应地更新 `add_executable` 的文件列表。
```cmake
add_executable(Demo02StaticAssetViewer
    main.cpp
    MainWindow.cpp
    RenderWidget.cpp
    # 添加新的 .cpp 文件
)
```
