# Demo-03: WGS84/ENU/SCENE Geo Coordinate Alignment

## 1 绪论

### 1.1 研究背景
在低空数字孪生和无人机地面站（Ground Control Station, GCS）应用中，多源数据的空间配准是核心技术挑战之一。无人机通常提供基于 WGS84 椭球的全球经纬高（Latitude, Longitude, Altitude）坐标，而 OpenGL 三维渲染引擎则使用基于笛卡尔坐标系的局部场景坐标（SCENE）。为了在三维空间中精确渲染无人机轨迹和数字资产，必须建立一套高精度的地理坐标转换链路。

### 1.2 研究内容
本项目（Demo-03）旨在实现一套完整的 WGS84 到 OpenGL 局部场景的坐标转换与可视化系统。具体包括：
(1) 实现 `WGS84 -> ECEF -> ENU -> SCENE -> OpenGL` 的标准转换链路。
(2) 开发基于 UDP 的 WGS84 JSON 遥测接收模块。
(3) 构建基于 Qt6 OpenGL 的三维可视化界面，展示地理网格、坐标轴、无人机实体及历史轨迹。
(4) 提供配套的 Python 轨迹生成模拟器，用于系统验证。

---

## 二、 技术方案正文

### 2.1 坐标转换算法

#### 2.1.1 WGS84 到 ECEF (Earth-Centered, Earth-Fixed)
将大地坐标系转换为地心地固笛卡尔坐标系。公式如下：
- $N(\phi) = \frac{a}{\sqrt{1 - e^2 \sin^2\phi}}$ （卯酉圈曲率半径）
- $X = (N + h) \cos\phi \cos\lambda$
- $Y = (N + h) \cos\phi \sin\lambda$
- $Z = (N(1 - e^2) + h) \sin\phi$

其中 $a = 6378137.0$ 为 WGS84 长半轴，$e^2 \approx 0.00669437999$ 为第一偏心率平方。

#### 2.1.2 ECEF 到 ENU (East-North-Up)
以用户指定的参考原点（$\phi_0, \lambda_0$）建立局部切平面坐标系。
- $dX = X - X_0, dY = Y - Y_0, dZ = Z - Z_0$
- $E = -\sin\lambda_0 dX + \cos\lambda_0 dY$
- $N = -\sin\phi_0 \cos\lambda_0 dX - \sin\phi_0 \sin\lambda_0 dY + \cos\phi_0 dZ$
- $U = \cos\phi_0 \cos\lambda_0 dX + \cos\phi_0 \sin\lambda_0 dY + \sin\phi_0 dZ$

#### 2.1.3 ENU 到 SCENE 到 OpenGL
- **SCENE 坐标**：直接映射 ENU（$x=E, y=N, z=U$），可附加缩放因子。
- **OpenGL 坐标**：适配图形学右手系（$X_{gl} = x_{scene}, Y_{gl} = z_{scene}, Z_{gl} = -y_{scene}$）。

### 2.2 系统架构
- `geo/`：核心坐标转换类（`CoordinateTransformer`、`Wgs84Converter`）。
- `telemetry/`：UDP WGS84 JSON 接收器。
- `render/`：OpenGL 渲染组件（场景、无人机标记、轨迹）。
- `camera/`：支持平移、旋转、缩放的轨道相机（OrbitCamera）。
- `config/`：JSON 配置文件加载与解析。

---

## 三、 项目研究结论及展望

### 3.1 运行验证截图

| 程序启动初始状态 | UDP 接收与坐标计算 | 椭圆轨迹积累 |
| :---: | :---: | :---: |
| ![][img1] | ![][img2] | ![][img3] |
| **原点校验结果** | **不同视角渲染** | **完整轨迹面板** |
| ![][img4] | ![][img5] | ![][img6] |

[img1]: ../../screenshots/demo03/01_app_start_enu_scene.png
[img2]: ../../screenshots/demo03/02_udp_started_receiving.png
[img3]: ../../screenshots/demo03/03_trajectory_ellipse_enu.png
[img4]: ../../screenshots/demo03/04_validate_origin_result.png
[img5]: ../../screenshots/demo03/05_orbit_camera_view.png
[img6]: ../../screenshots/demo03/06_full_trajectory_coord_panel.png

### 3.2 编译与运行指南
(1) 依赖安装：`sudo apt-get install qt6-base-dev libqt6openglwidgets6t64`
(2) 编译工程：
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```
(3) 启动主程序：`./Demo03GeoCoordinateAlignment`
(4) 启动模拟器（另开终端）：
```bash
cd tools/geo_telemetry_simulator
python3 send_wgs84_uav_udp.py --rate 20
```

### 3.3 结论与展望
本演示程序成功验证了高精度地理坐标转换在 Qt/OpenGL 环境下的实时应用。通过 UI 面板可直观比对 WGS84 原始数据与 ENU 局部坐标。
下一步（Demo-04）将引入真实无人机（PX4/MAVLink）的遥测数据，并在 Demo-05 中结合 3D Gaussian Splatting 模型，实现地理对齐的数字孪生渲染。
