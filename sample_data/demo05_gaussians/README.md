# Demo-05 示例数据说明

本目录包含用于 Demo-05 的 3D Gaussian Splatting（3DGS）示例数据。

## 数据来源

这里的所有 `.ply` 文件均由 Python 脚本 `tools/gaussian_generators/generate_demo05_gaussians.py` 自动生成。
**这些数据仅用于教学和程序功能验证，并非来自真实的 3DGS 训练流程。**

真实的 3DGS 模型文件通常很大（几百 MB 到几 GB），为了保持 GitHub 仓库轻量，我们不提交真实数据。

## 文件列表

1. `tiny_colored_gaussians.ply` (200 个 Gaussian)
   - 球形分布的彩色 Gaussian 点云，用于验证基础的 PLY 加载和颜色解析。
2. `lowair_demo_gaussian_cloud.ply` (3000 个 Gaussian)
   - 模拟低空无人机视角下的典型场景语义（包含地面层、建筑层、植被层），用于验证稍大规模点云的渲染性能和 Billboard 效果。
3. `metadata.json`
   - 包含这些生成数据的元信息说明，以及为 Demo-06 预留的坐标对齐占位符。

## 格式说明

文件采用标准的 3DGS PLY 格式（ASCII 或 Binary），包含以下核心属性：
- `x, y, z`：中心位置
- `f_dc_0, f_dc_1, f_dc_2`：球谐函数（Spherical Harmonics）的 0 阶系数（DC分量），用于计算基础颜色
- `opacity`：不透明度（存储为 sigmoid 的逆变换 logit）
- `scale_0, scale_1, scale_2`：三轴缩放比例（存储为自然对数）
- `rot_0, rot_1, rot_2, rot_3`：旋转四元数

Demo-05 的 `GaussianPlyLoader` 能够正确解析上述标准字段。
