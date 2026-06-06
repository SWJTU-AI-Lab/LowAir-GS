# Demo-06: Dual Scene Fusion Viewer (摄影测量模型 + 3DGS 双源场景融合显示)

## 1. 简介

本 Demo 实现了摄影测量模型（Mesh）与 3D Gaussian Splatting（3DGS）高斯点云模型的同屏融合显示。通过六层模块化架构，支持多源数据加载、独立坐标系对齐、四种融合渲染模式以及实时的性能统计。

## 2. 核心功能

- **双源资产加载**：支持标准 OBJ/PLY 摄影测量模型和 PLY/.splat 高斯点云模型
- **独立坐标对齐**：支持平移（Tx/Ty/Tz）、旋转（Rx/Ry/Rz）、缩放（Scale）和一键中心对齐
- **四种融合模式**：
  - `Mesh Only`：仅显示摄影测量模型
  - `Gaussian Only`：仅显示 3DGS 高斯模型
  - `Dual Fusion`：双源融合渲染，支持透明度混合
  - `Wireframe Comparison`：线框对比模式（Mesh 线框 + Gaussian Splat）
- **配置文件管理**：支持将对齐参数和图层状态保存为 JSON 格式，便于复用
- **实时性能统计**：统计 FPS、帧耗时、顶点数、三角面数、Gaussian 数量及 VRAM 占用估算

## 3. 架构设计

系统分为六个主要层级：
- `mesh/`：摄影测量模型资产管理与渲染（基于 Assimp）
- `gaussian/`：高斯点云资产解析与渲染（Point/Splat 双模式）
- `fusion/`：融合控制器，管理对齐参数、变换矩阵计算和状态统计
- `render/`：公共渲染组件（网格、坐标轴、包围盒）
- `camera/`：轨道相机（支持旋转、缩放、平移）
- `perf/`：性能统计与 FPS 计数

## 4. 编译与运行

### 4.1 依赖项
- Qt 6 (Core, Gui, Widgets, OpenGLWidgets)
- OpenGL 3.3+ (Core Profile)
- Assimp (用于 Mesh 加载)

### 4.2 编译命令
```bash
mkdir build && cd build
cmake ..
make -j4
```

### 4.3 运行方式
可以直接运行程序，通过界面按钮加载数据，也可以通过命令行参数自动加载：
```bash
# 在仓库根目录生成示例数据
cd /path/to/LowAir-GS
python3 tools/gaussian_generators/generate_demo06_assets.py

# 启动程序（在 build 目录下）
cd apps/demo06_dual_scene_fusion/build
./Demo06DualSceneFusion \
    --mesh ../../../sample_data/demo06_fusion/building_mesh.obj \
    --gaussian ../../../sample_data/demo06_fusion/building_gaussians.ply \
    --config ../../../sample_data/demo06_fusion/fusion_config.json
```

## 5. 验收截图展示

本 Demo 提供 7 张标准验收截图，位于 `screenshots/demo06/` 目录下：
1. `01_app_start_fusion_panel.png`: 程序启动状态，融合面板就绪
2. `02_mesh_loaded_solid.png`: Mesh 加载状态（Solid 模式）
3. `03_gaussian_loaded_splat.png`: Gaussian 加载状态（Splat 模式）
4. `04_gaussian_only_mode.png`: Gaussian Only 渲染模式
5. `05_wireframe_comparison.png`: Wireframe Comparison 线框对比模式
6. `06_dual_fusion_orbit_view.png`: Dual Fusion 双源融合模式（侧视角）
7. `07_stats_panel_dual_fusion.png`: 实时统计面板（顶点、面片、高斯数量及 VRAM）

## 6. 技术边界与 Demo-07 关系

| 特性 | Demo-06 (本程序) | Demo-07 (最终目标) |
|---|---|---|
| **核心定位** | 静态双源场景对齐与融合 | 动态双源场景 + 实时遥测无人机 |
| **坐标系** | 局部相对坐标系（Tx/Ty/Tz） | 全球地理坐标系（WGS84 → ENU → SCENE） |
| **无人机** | 无 | 接入 MAVLink 遥测，实时渲染无人机与轨迹 |
| **应用场景** | 资产预处理、坐标对齐标定 | 低空数字孪生完整系统展示 |

## 7. FAQ

**Q: 为什么在 Dual Fusion 模式下，Mesh 和 Gaussian 的遮挡关系有时不正确？**
A: 本 Demo 采用基于深度的标准 OpenGL 混合渲染。由于 3DGS 渲染（Billboard 面片）未进行严格的深度排序，在半透明区域可能出现混合伪影。这在实时渲染中是正常的性能折衷。

**Q: "Align Center" 按钮的作用是什么？**
A: 它会计算 Mesh 和 Gaussian 的包围盒中心，自动计算平移偏移量（Tx/Ty/Tz），使两个模型的几何中心在原点重合，作为手动微调对齐的起点。

**Q: 融合配置 JSON 包含哪些信息？**
A: 包含平移、旋转、缩放参数，以及 Mesh 和 Gaussian 的启用状态、透明度、渲染模式（Solid/Wireframe、Point/Splat）等 UI 状态。
