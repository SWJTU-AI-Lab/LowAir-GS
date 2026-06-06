#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QWheelEvent>

class SimpleScene;
class UavRenderer;
class TrajectoryRenderer;

/**
 * @brief RenderWidget — Demo-01 三维渲染视口
 *
 * 功能范围（Demo-01）：
 *   - 程序内置简化低空演示场景（地面网格、坐标轴、建筑块、起降点、参考航线）
 *   - 无人机图标实时位置与姿态显示
 *   - 无人机历史轨迹线绘制
 *
 * 不包含（留给后续 Demo）：
 *   - OBJ/PLY/GLB 外部模型加载（Demo-02）
 *   - WGS84 / ENU / SCENE 地理坐标转换（Demo-03）
 *   - MAVLink / ROS2 真实无人机接入（Demo-04）
 *   - 3D Gaussian Splatting 场景（Demo-05）
 */
class RenderWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    RenderWidget(QWidget *parent = nullptr);
    ~RenderWidget();

    void clearTrajectory();
    void resetView();
    int getTrajectoryPointCount() const;

public slots:
    void updateUavState(const QString &uavId, double x, double y, double z,
                        double roll, double pitch, double yaw, double battery);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    SimpleScene       *m_scene;
    UavRenderer       *m_uavRenderer;
    TrajectoryRenderer *m_trajRenderer;

    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;

    // 相机控制参数
    float  m_cameraDistance;
    float  m_cameraPitch;
    float  m_cameraYaw;
    QPoint m_lastMousePos;
};

#endif // RENDERWIDGET_H
