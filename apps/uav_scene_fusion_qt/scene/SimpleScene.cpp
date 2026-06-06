#include "SimpleScene.h"
#include <QVector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SimpleScene::SimpleScene(QObject *parent)
    : m_program(nullptr)
{
    Q_UNUSED(parent);
}

SimpleScene::~SimpleScene()
{
    delete m_program;
}

void SimpleScene::initialize()
{
    initializeOpenGLFunctions();

    m_program = new QOpenGLShaderProgram();

    const char *vsrc =
        "attribute highp vec4 posAttr;\n"
        "uniform highp mat4 matrix;\n"
        "void main() {\n"
        "   gl_Position = matrix * posAttr;\n"
        "}\n";

    const char *fsrc =
        "uniform lowp vec4 color;\n"
        "void main() {\n"
        "   gl_FragColor = color;\n"
        "}\n";

    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vsrc);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fsrc);
    m_program->link();
}

void SimpleScene::render(const QMatrix4x4 &projection, const QMatrix4x4 &view)
{
    if (!m_program) return;

    QMatrix4x4 vp = projection * view;

    drawGrid(vp);
    drawAxes(vp);
    drawBuildings(vp);
    drawLandingPad(vp);
    drawRefRoute(vp);
}

// -----------------------------------------------------------------------
// 地面网格（100×100，5m 间距）
// -----------------------------------------------------------------------
void SimpleScene::drawGrid(const QMatrix4x4 &vp)
{
    m_program->bind();
    m_program->setUniformValue("matrix", vp);
    m_program->setUniformValue("color", QVector4D(0.30f, 0.32f, 0.36f, 1.0f));

    QVector<GLfloat> vertices;
    const int gridSize = 100;
    const int step = 5;

    for (int i = -gridSize; i <= gridSize; i += step) {
        vertices.append(i); vertices.append(0); vertices.append(-gridSize);
        vertices.append(i); vertices.append(0); vertices.append(gridSize);

        vertices.append(-gridSize); vertices.append(0); vertices.append(i);
        vertices.append(gridSize);  vertices.append(0); vertices.append(i);
    }

    m_program->enableAttributeArray("posAttr");
    m_program->setAttributeArray("posAttr", vertices.data(), 3);

    glLineWidth(1.0f);
    glDrawArrays(GL_LINES, 0, vertices.size() / 3);

    m_program->disableAttributeArray("posAttr");
    m_program->release();
}

// -----------------------------------------------------------------------
// XYZ 坐标轴（红 X / 绿 Y / 蓝 Z，各 15m）
// -----------------------------------------------------------------------
void SimpleScene::drawAxes(const QMatrix4x4 &vp)
{
    m_program->bind();
    m_program->setUniformValue("matrix", vp);

    GLfloat vertices[] = {
        0.0f, 0.0f, 0.0f,  15.0f,  0.0f,  0.0f,  // X 轴
        0.0f, 0.0f, 0.0f,   0.0f, 15.0f,  0.0f,  // Y 轴
        0.0f, 0.0f, 0.0f,   0.0f,  0.0f, 15.0f   // Z 轴
    };

    m_program->enableAttributeArray("posAttr");
    m_program->setAttributeArray("posAttr", vertices, 3);

    glLineWidth(3.0f);

    m_program->setUniformValue("color", QVector4D(0.9f, 0.2f, 0.2f, 1.0f)); // 红 X
    glDrawArrays(GL_LINES, 0, 2);

    m_program->setUniformValue("color", QVector4D(0.2f, 0.9f, 0.2f, 1.0f)); // 绿 Y
    glDrawArrays(GL_LINES, 2, 2);

    m_program->setUniformValue("color", QVector4D(0.2f, 0.5f, 0.9f, 1.0f)); // 蓝 Z
    glDrawArrays(GL_LINES, 4, 2);

    m_program->disableAttributeArray("posAttr");
    m_program->release();
}

// -----------------------------------------------------------------------
// 简化建筑块（5 栋，不同高度和颜色）
// -----------------------------------------------------------------------
void SimpleScene::drawBuildings(const QMatrix4x4 &vp)
{
    // 建筑 A：高层（右前方）
    drawCube(vp, QVector3D( 25.0f, 12.0f, -20.0f),
                 QVector3D( 10.0f, 24.0f,  12.0f),
                 QVector4D(0.55f, 0.58f, 0.62f, 1.0f));

    // 建筑 B：中层（左前方）
    drawCube(vp, QVector3D(-28.0f, 8.0f, -15.0f),
                 QVector3D( 14.0f, 16.0f,  14.0f),
                 QVector4D(0.48f, 0.54f, 0.60f, 1.0f));

    // 建筑 C：低层（右后方）
    drawCube(vp, QVector3D( 40.0f, 5.0f, -40.0f),
                 QVector3D( 18.0f, 10.0f,  16.0f),
                 QVector4D(0.52f, 0.50f, 0.55f, 1.0f));

    // 建筑 D：高层（左后方）
    drawCube(vp, QVector3D(-45.0f, 15.0f, -35.0f),
                 QVector3D( 12.0f, 30.0f,  12.0f),
                 QVector4D(0.45f, 0.50f, 0.58f, 1.0f));

    // 建筑 E：中层（正后方）
    drawCube(vp, QVector3D(  5.0f, 7.0f, -55.0f),
                 QVector3D( 20.0f, 14.0f,  18.0f),
                 QVector4D(0.50f, 0.52f, 0.56f, 1.0f));
}

// -----------------------------------------------------------------------
// 起降点：地面圆环 + H 标记（绿色，位于场景中心偏右）
// -----------------------------------------------------------------------
void SimpleScene::drawLandingPad(const QMatrix4x4 &vp)
{
    m_program->bind();
    m_program->setUniformValue("matrix", vp);

    // 外圆环（半径 6m，64 段）
    const int segments = 64;
    const float radius = 6.0f;
    const float cx = 10.0f, cz = 10.0f, cy = 0.05f;

    QVector<GLfloat> circle;
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * (float)M_PI * i / segments;
        circle.append(cx + radius * cosf(angle));
        circle.append(cy);
        circle.append(cz + radius * sinf(angle));
    }

    m_program->enableAttributeArray("posAttr");
    m_program->setAttributeArray("posAttr", circle.data(), 3);
    m_program->setUniformValue("color", QVector4D(0.2f, 0.85f, 0.3f, 1.0f));
    glLineWidth(2.5f);
    glDrawArrays(GL_LINE_STRIP, 0, circle.size() / 3);

    // 内圆（半径 3m）
    QVector<GLfloat> innerCircle;
    const float r2 = 3.0f;
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * (float)M_PI * i / segments;
        innerCircle.append(cx + r2 * cosf(angle));
        innerCircle.append(cy);
        innerCircle.append(cz + r2 * sinf(angle));
    }
    m_program->setAttributeArray("posAttr", innerCircle.data(), 3);
    glDrawArrays(GL_LINE_STRIP, 0, innerCircle.size() / 3);

    // H 横线
    GLfloat hLines[] = {
        cx - 2.5f, cy, cz,      cx + 2.5f, cy, cz,   // 横
        cx - 2.5f, cy, cz - 3.0f, cx - 2.5f, cy, cz + 3.0f, // 左竖
        cx + 2.5f, cy, cz - 3.0f, cx + 2.5f, cy, cz + 3.0f  // 右竖
    };
    m_program->setAttributeArray("posAttr", hLines, 3);
    glLineWidth(3.0f);
    glDrawArrays(GL_LINES, 0, 6);

    m_program->disableAttributeArray("posAttr");
    m_program->release();
}

// -----------------------------------------------------------------------
// 参考航线：预设椭圆航迹 + 目标点（黄色虚线风格）
// -----------------------------------------------------------------------
void SimpleScene::drawRefRoute(const QMatrix4x4 &vp)
{
    m_program->bind();
    m_program->setUniformValue("matrix", vp);
    m_program->setUniformValue("color", QVector4D(0.9f, 0.75f, 0.1f, 0.7f));

    // 椭圆参考航线（与 Python 模拟器轨迹一致：x=cos*30, z=sin*20, y=15）
    const int segments = 60;
    QVector<GLfloat> route;
    for (int i = 0; i <= segments; ++i) {
        float t = 2.0f * (float)M_PI * i / segments;
        float rx =  30.0f * cosf(t);   // SCENE x → OpenGL x
        float ry =  15.0f;             // SCENE z → OpenGL y（高度固定参考）
        float rz = -20.0f * sinf(t);   // SCENE y → OpenGL -z
        route.append(rx);
        route.append(ry);
        route.append(rz);
    }

    m_program->enableAttributeArray("posAttr");
    m_program->setAttributeArray("posAttr", route.data(), 3);
    glLineWidth(1.5f);
    glDrawArrays(GL_LINE_STRIP, 0, route.size() / 3);

    // 目标点（小十字，位于参考航线起点）
    GLfloat cross[] = {
        28.0f, 15.0f,  1.5f,   32.0f, 15.0f, -1.5f,  // 横
        30.0f, 13.5f,  0.0f,   30.0f, 16.5f,  0.0f   // 竖
    };
    m_program->setUniformValue("color", QVector4D(1.0f, 0.4f, 0.1f, 1.0f));
    m_program->setAttributeArray("posAttr", cross, 3);
    glLineWidth(2.5f);
    glDrawArrays(GL_LINES, 0, 4);

    m_program->disableAttributeArray("posAttr");
    m_program->release();
}

// -----------------------------------------------------------------------
// 通用立方体渲染（中心点 + 尺寸 + 颜色）
// -----------------------------------------------------------------------
void SimpleScene::drawCube(const QMatrix4x4 &vp, const QVector3D &pos,
                            const QVector3D &size, const QVector4D &color)
{
    m_program->bind();

    QMatrix4x4 model;
    model.translate(pos);
    model.scale(size);

    m_program->setUniformValue("matrix", vp * model);
    m_program->setUniformValue("color", color);

    GLfloat vertices[] = {
        // Front
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        // Back
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f
    };

    GLuint indices[] = {
        0, 1, 2, 2, 3, 0,  // Front
        1, 5, 6, 6, 2, 1,  // Right
        5, 4, 7, 7, 6, 5,  // Back
        4, 0, 3, 3, 7, 4,  // Left
        3, 2, 6, 6, 7, 3,  // Top
        4, 5, 1, 1, 0, 4   // Bottom
    };

    m_program->enableAttributeArray("posAttr");
    m_program->setAttributeArray("posAttr", vertices, 3);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, indices);

    m_program->disableAttributeArray("posAttr");
    m_program->release();
}
