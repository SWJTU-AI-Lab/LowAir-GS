# Demo-05: Gaussian Splat Viewer - 技术报告

## 1. 摘要
本技术报告探讨了 Demo-05 (Gaussian Splat Viewer) 的技术实现。3D Gaussian Splatting (3DGS) 是一种革命性的辐射场渲染技术。本 Demo 成功在 Qt/OpenGL 环境下实现了 3DGS 的前向渲染管线，验证了将照片级真实感场景引入无人机地面站系统的可行性。

## 2. 3DGS 渲染原理简述
传统的 3D 渲染基于多边形网格（Mesh），而 3DGS 放弃了网格，使用数以百万计的 3D 高斯椭球（Gaussian Splats）来表示场景。每个高斯球包含以下属性：
- **位置 (Position)**: 3D 空间坐标 $\mu$。
- **协方差矩阵 (Covariance)**: 决定椭球的形状（缩放 $S$）和方向（旋转 $R$），$\Sigma = R S S^T R^T$。
- **颜色 (Color)**: 通常由球谐函数（Spherical Harmonics, SH）表示，以支持视角相关的颜色变化。
- **不透明度 (Opacity)**: $\alpha$ 值。

渲染过程本质上是将这些 3D 高斯球投影到 2D 屏幕空间，并进行 Alpha 混合。

## 3. 核心技术实现
### 3.1 PLY 数据解析与存储
3DGS 训练输出的标准格式是扩展的 `.ply` 文件。在 `PlyLoader` 中，我们自定义了解析逻辑，提取了标准顶点之外的 `f_dc` (SH 系数)、`scale`、`rot` 和 `opacity` 字段。
由于数据量巨大（通常数百 MB），数据被直接映射到显存中的 Shader Storage Buffer Object (SSBO)，避免了渲染时的 CPU-GPU 数据拷贝。

### 3.2 2D 协方差投影
在 Vertex Shader 中，最关键的数学运算是将 3D 协方差矩阵 $\Sigma$ 投影到 2D 屏幕空间 $\Sigma'$：
$$ \Sigma' = J W \Sigma W^T J^T $$
其中 $W$ 是相机的视图矩阵（View Matrix），$J$ 是透视投影矩阵的雅可比矩阵（Jacobian of the Projective Transformation）。
计算出 $\Sigma'$ 后，通过特征值分解求出 2D 椭圆的长短轴，从而生成一个紧紧包围该椭圆的 2D Quad 传递给片元着色器。

### 3.3 高斯权重计算与混合
在 Fragment Shader 中，计算当前像素 $(x, y)$ 相对于高斯球中心 $(x_0, y_0)$ 的马氏距离（Mahalanobis Distance），并代入高斯函数求得权重 $G$：
$$ G = \exp\left( -\frac{1}{2} \Delta^T (\Sigma')^{-1} \Delta \right) $$
最终的 Alpha 值为 $\alpha \times G$。通过开启 OpenGL 的 `GL_BLEND`，使用 `GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA` 混合模式，完成最终颜色的累加。

## 4. 性能瓶颈与优化策略
### 4.1 深度排序 (Depth Sorting)
Alpha 混合要求严格的从后向前排序。对于数百万个元素，每帧进行 CPU 排序是不可接受的。本 Demo 采用了基于 Compute Shader 的 GPU 排序算法（如 Bitonic Sort），将排序时间压缩到了毫秒级。

### 4.2 视锥体剔除 (Frustum Culling)
为了减少不必要的光栅化开销，在 Vertex Shader 阶段加入了视锥体剔除逻辑。如果高斯球的中心在相机视锥体之外，则将其顶点坐标设为无穷远，使其在裁剪阶段被丢弃。

## 5. 结论
Demo-05 成功实现了 3DGS 的核心渲染管线。相比于传统的倾斜摄影模型（OSGB/3D Tiles），3DGS 提供了无与伦比的视觉保真度和更流畅的渲染性能。这一技术的突破，为 Demo-06 中无人机在超真实场景中的融合监控提供了坚实的渲染基础。
