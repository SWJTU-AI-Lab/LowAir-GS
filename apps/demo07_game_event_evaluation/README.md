# Demo-07: 虚拟目标、事件、特效与游戏化评估 (Virtual Target, Event, Effects & Game-like Evaluation)

## 模块功能说明

本演示程序（Demo-07）是 LowAir-GS 系统的最后一个核心 Demo，综合了之前所有 Demo 的技术积累，实现了一个完整的虚拟巡检与游戏化评估系统。该系统展示了如何在 3D 场景中进行无人机轨迹回放，并通过事件驱动引擎实现虚拟目标的触发、特效渲染以及任务评分。

### 核心特性

1. **虚拟目标与触发区域 (Virtual Targets & Trigger Zones)**
   - 支持在 3D 空间中定义虚拟目标点和触发区域（如禁飞区、重点巡检区）。
   - 目标和区域通过 JSON 配置文件动态加载，实现任务场景的灵活配置。

2. **事件驱动引擎 (Event-Driven Engine)**
   - 实时监控无人机轨迹与虚拟目标/区域的空间关系。
   - 自动触发“到达目标”、“进入区域”、“离开区域”等事件。
   - 维护事件时间线（Event Timeline），记录事件发生的时间、类型和关联对象。

3. **视觉特效与增强渲染 (Visual Effects & Rendering Enhancements)**
   - **Glow Effect (发光特效)**：当无人机到达虚拟目标时，目标会产生高亮发光效果，且目标线框变为绿色。
   - **Particle Effect (粒子特效)**：当无人机到达特定检查点时，触发粒子爆发特效。
   - **Label Popup (浮动文字弹窗)**：事件触发时，在屏幕右上角显示带生命周期的文字提示（如得分增加和目标到达）。
   - **Path Highlight (轨迹高亮)**：已完成的轨迹段显示为亮黄色，未完成段保持青色，直观展示任务进度。
   - **Zone Outline (区域脉冲增强)**：无人机进入区域时，区域线框会变为亮蓝色并伴随呼吸脉冲缩放动画。
   - **Virtual Explosion (虚拟爆炸特效)**：当触发特定事件时，渲染包含中心闪光、环形波纹、多色粒子和地面发光圆环的复合视觉特效。

4. **游戏化评估与评分系统 (Game-like Evaluation & Scoring)**
   - 基于预定义的评分规则（Score Rules）对任务执行情况进行评估。
   - 规则包括：到达目标加分、进入禁飞区扣分、电量消耗扣分、超时扣分等。
   - 实时计算当前得分，并在任务结束时生成完整的任务报告（Mission Report）。

5. **轨迹回放与控制 (Trajectory Replay & Control)**
   - 支持加载高精度无人机轨迹数据（JSONL 格式）。
   - 提供回放控制功能（播放、暂停、重置、倍速调节）。
   - 在 3D 视图中实时绘制无人机模型、轨迹线和当前状态。

## 技术架构

- **UI 框架**：Qt6 Widgets
- **3D 渲染**：Qt6 OpenGL (QOpenGLExtraFunctions)
- **模型加载**：Assimp (支持 .obj, .ply 等格式)
- **数据序列化**：Qt6 JSON (QJsonDocument, QJsonObject, QJsonArray)
- **事件处理**：基于 Qt 信号槽机制 (Signals and Slots)

## 模块结构

- `scene/`: 场景加载与管理 (SceneAsset, SceneLoader)
- `mission/`: 任务配置与虚拟对象 (MissionConfig, VirtualTarget, TriggerZone)
- `event/`: 事件引擎与时间线 (EventEngine, EventRecord, EventTimeline)
- `replay/`: 轨迹回放控制 (TrajectoryReplay, ReplayFrame)
- `scoring/`: 评分规则与评估器 (ScoreRule, ScoreEvaluator, MissionReport)
- `effects/`: 特效渲染器 (Effect, GlowEffectRenderer, ParticleEffectRenderer, LabelPopup, VirtualExplosionRenderer)
- `render/`: 3D 渲染组件 (TargetRenderer, ZoneRenderer, TrajectoryRenderer 等)
- `camera/`: 轨道相机控制 (OrbitCamera)

## 编译与运行

### 依赖要求
- Qt 6.x (包含 Widgets, OpenGL, OpenGLWidgets 模块)
- Assimp 库
- CMake 3.16+
- C++17 编译器

### 编译步骤
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 运行方式
```bash
./Demo07GameEventEvaluation
```
运行后，在界面左侧面板输入任务配置文件路径（例如 `../../sample_data/demo07_game/mission_config.json`），点击 "Load Mission" 即可加载任务并开始演示。

## 安全边界声明

本程序仅用于**虚拟场景教学评估**与算法验证演示，**不涉及**任何真实无人机控制。所有目标、事件和特效均为纯软件层面的虚拟模拟，旨在展示地面站软件系统的架构设计与数据处理能力。
