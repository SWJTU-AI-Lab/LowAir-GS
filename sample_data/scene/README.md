# 场景模型数据说明

此目录用于存放真实摄影测量或 3DGS 导出的三维场景模型。

## 数据放置规范

为避免 GitHub 仓库体积过大，**请勿**将大型模型文件（如 `.obj`, `.glb`, `.ply`, `.splat` 等）直接提交到代码仓库。

后续如果需要接入真实摄影测量模型，请将文件放置在本地的本目录下。推荐的目录结构如下：

```text
sample_data/scene/
├── campus_model.obj      # 场景网格模型 (不提交)
├── campus_model.mtl      # 材质文件 (不提交)
├── textures/             # 纹理贴图文件夹 (不提交)
├── scene_config.json     # 场景配置文件 (可提交，包含偏移、缩放等参数)
└── README.md             # 本说明文件
```

**注意：在 Demo-01（基础框架版）中，程序仅使用内置生成的简化几何体（SimpleScene），不依赖任何外部模型文件。**

外部模型加载（Assimp/TinyGLTF）和坐标系对齐功能将在后续的 Demo-02 和 Demo-03 中实现。
