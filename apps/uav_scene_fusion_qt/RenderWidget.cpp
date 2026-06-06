#include "RenderWidget.h"
#include "scene/SimpleScene.h"
#include "render/UavRenderer.h"
#include "render/TrajectoryRenderer.h"
#include <QDebug>

RenderWidget::RenderWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_scene(nullptr)
    , m_uavRenderer(nullptr)
    , m_trajRenderer(nullptr)
    , m_cameraDistance(90.0f)
    , m_cameraPitch(40.0f)
    , m_cameraYaw(25.0f)
{
}

RenderWidget::~RenderWidget()
{
    makeCurrent();
    delete m_scene;
    delete m_uavRenderer;
    delete m_trajRenderer;
    doneCurrent();
}

void RenderWidget::initializeGL()
{
    initializeOpenGLFunctions();

    // 深色背景（GCS 标准深蓝灰）
    glClearColor(0.12f, 0.14f, 0.18f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 初始化程序内置简化场景（Demo-01 唯一场景来源）
    m_scene = new SimpleScene(this);
    m_uavRenderer = new UavRenderer(this);
    m_trajRenderer = new TrajectoryRenderer(this);

    m_scene->initialize();
    m_uavRenderer->initialize();
    m_trajRenderer->initialize();

    qDebug() << "[Demo-01] Scene initialized: built-in simplified low-altitude scene";
    qDebug() << "[Demo-01] No external model files required for this demo";
}

void RenderWidget::resizeGL(int w, int h)
{
    m_projection.setToIdentity();
    m_projection.perspective(45.0f, float(w) / float(h ? h : 1), 0.1f, 1000.0f);
}

void RenderWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_view.setToIdentity();
    m_view.translate(0.0f, 0.0f, -m_cameraDistance);
    m_view.rotate(m_cameraPitch, 1.0f, 0.0f, 0.0f);
    m_view.rotate(m_cameraYaw,   0.0f, 1.0f, 0.0f);

    // 渲染程序内置简化场景
    m_scene->render(m_projection, m_view);

    // 渲染无人机轨迹线和无人机图标
    m_trajRenderer->render(m_projection, m_view);
    m_uavRenderer->render(m_projection, m_view);
}

void RenderWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePos = event->position().toPoint();
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = (int)event->position().x() - m_lastMousePos.x();
    int dy = (int)event->position().y() - m_lastMousePos.y();

    if (event->buttons() & Qt::LeftButton) {
        m_cameraYaw   += dx * 0.5f;
        m_cameraPitch += dy * 0.5f;

        if (m_cameraPitch <  0.0f) m_cameraPitch =  0.0f;
        if (m_cameraPitch > 90.0f) m_cameraPitch = 90.0f;

        update();
    }
    m_lastMousePos = event->position().toPoint();
}

void RenderWidget::wheelEvent(QWheelEvent *event)
{
    m_cameraDistance -= event->angleDelta().y() * 0.05f;
    if (m_cameraDistance <  10.0f) m_cameraDistance =  10.0f;
    if (m_cameraDistance > 500.0f) m_cameraDistance = 500.0f;
    update();
}

void RenderWidget::updateUavState(const QString &uavId, double x, double y, double z,
                                   double roll, double pitch, double yaw, double battery)
{
    Q_UNUSED(uavId);
    Q_UNUSED(battery);

    // Demo-01 坐标约定：
    //   输入 x/y/z 为 SCENE 局部演示坐标（单位：米），由 Python 模拟器生成
    //   SCENE 坐标系：x 向右，y 向前，z 向上
    //   OpenGL 坐标系：x 向右，y 向上，z 向后（右手系）
    //   映射关系：OpenGL(x, y, z) = SCENE(x, z, -y)
    //
    //   注意：Demo-01 不进行任何真实地理坐标转换（WGS84/ENU/SCENE）
    //   真实坐标对齐将在 Demo-03 中实现（Reserved for Demo-03）
    QVector3D pos(static_cast<float>(x),
                  static_cast<float>(z),
                  static_cast<float>(-y));

    m_uavRenderer->updateState(pos, roll, pitch, yaw);
    m_trajRenderer->addPoint(pos);
    update();
}

void RenderWidget::clearTrajectory()
{
    m_trajRenderer->clear();
    update();
}

void RenderWidget::resetView()
{
    m_cameraDistance = 90.0f;
    m_cameraPitch    = 40.0f;
    m_cameraYaw      = 25.0f;
    update();
}

int RenderWidget::getTrajectoryPointCount() const
{
    return m_trajRenderer->getPointCount();
}
