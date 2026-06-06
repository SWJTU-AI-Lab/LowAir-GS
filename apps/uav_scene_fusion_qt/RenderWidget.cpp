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
    , m_cameraDistance(100.0f)
    , m_cameraPitch(30.0f)
    , m_cameraYaw(45.0f)
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
    
    // 设置深色背景
    glClearColor(0.18f, 0.18f, 0.20f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_scene = new SimpleScene(this);
    m_uavRenderer = new UavRenderer(this);
    m_trajRenderer = new TrajectoryRenderer(this);

    m_scene->initialize();
    m_uavRenderer->initialize();
    m_trajRenderer->initialize();
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
    m_view.rotate(m_cameraYaw, 0.0f, 1.0f, 0.0f);
    m_view.translate(-50.0f, 0.0f, -50.0f); // 中心点偏移

    m_scene->render(m_projection, m_view);
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
        m_cameraYaw += dx * 0.5f;
        m_cameraPitch += dy * 0.5f;
        
        // 限制Pitch
        if (m_cameraPitch < 0.0f) m_cameraPitch = 0.0f;
        if (m_cameraPitch > 90.0f) m_cameraPitch = 90.0f;
        
        update();
    }
    m_lastMousePos = event->position().toPoint();
}

void RenderWidget::wheelEvent(QWheelEvent *event)
{
    m_cameraDistance -= event->angleDelta().y() * 0.05f;
    if (m_cameraDistance < 10.0f) m_cameraDistance = 10.0f;
    if (m_cameraDistance > 500.0f) m_cameraDistance = 500.0f;
    update();
}

void RenderWidget::updateUavState(const QString &uavId, double x, double y, double z, double roll, double pitch, double yaw, double battery)
{
    Q_UNUSED(uavId);
    Q_UNUSED(battery);

    QVector3D pos(x, z, -y); // 简单坐标转换 (假设输入为 SCENE: x右, y前, z上 -> OpenGL: x右, y上, z后)
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
    m_cameraDistance = 100.0f;
    m_cameraPitch = 30.0f;
    m_cameraYaw = 45.0f;
    update();
}

int RenderWidget::getTrajectoryPointCount() const
{
    return m_trajRenderer->getPointCount();
}
