# Demo-05: Gaussian Splat Viewer - 二次开发指南

## 1. 架构概览
与前几个基于 Qt3D 的 Demo 不同，Demo-05 为了追求极致的渲染性能，直接使用了底层的 `QOpenGLWidget` 来实现 3D Gaussian Splatting 的光栅化渲染。
核心模块包括：
- **MainWindow**: 主窗口，处理 UI 事件。
- **GaussianSplatWidget**: 继承自 `QOpenGLWidget`，封装了完整的 OpenGL 渲染管线。
- **PlyLoader**: 负责解析 3DGS 特有的 `.ply` 文件格式。

## 2. 核心渲染管线说明
3DGS 的渲染是一个高度并行的过程，`GaussianSplatWidget` 中的渲染管线主要分为以下几个阶段：

### 2.1 数据加载与 SSBO 初始化
`PlyLoader` 将 `.ply` 文件解析为包含位置、球谐系数（SH）、协方差矩阵（缩放+旋转）和不透明度的结构体数组。这些数据被上传到 OpenGL 的 Shader Storage Buffer Object (SSBO) 中，供 GPU 直接访问。

### 2.2 深度排序 (Compute Shader)
由于高斯球是半透明的，必须进行严格的从后向前（Back-to-Front）的 Alpha 混合。
- 每次相机移动时，触发 Compute Shader 计算每个高斯球到相机的距离。
- 使用 GPU 端的基数排序（Radix Sort）算法对高斯球的索引进行排序。

### 2.3 投影与光栅化 (Vertex & Fragment Shader)
- **Vertex Shader**: 根据排序后的索引从 SSBO 读取高斯球数据，计算其在屏幕空间的二维协方差矩阵（即椭圆的大小和方向），并输出包围该椭圆的 2D Quad（四边形）。
- **Fragment Shader**: 对 Quad 内的像素进行采样，计算高斯衰减权重，并结合不透明度和颜色进行 Alpha 混合，最终输出像素颜色。

## 3. 扩展与修改
### 3.1 修改 Shader 逻辑
所有的着色器代码（`.vert`, `.frag`, `.comp`）通常以字符串或资源文件形式嵌入。如果需要修改渲染效果（如改变高斯衰减函数、添加光照模型），请直接修改 `GaussianSplatWidget` 中加载的 Shader 源码。

### 3.2 性能优化
当前的排序算法是基于 CPU 或基础 Compute Shader 实现的。为了处理数千万级别的 Splat，可以考虑引入更高效的并行排序算法（如基于 CUDA 的 CUB 库，或更高级的 Vulkan 互操作）。

## 4. 编译说明
本项目使用 CMake 构建，依赖 OpenGL 核心配置。
```cmake
find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL REQUIRED)
# ...
target_link_libraries(Demo05GaussianSplatViewer Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
```
