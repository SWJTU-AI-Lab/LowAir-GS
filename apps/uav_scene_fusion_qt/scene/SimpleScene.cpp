#include "SimpleScene.h"
#include <QVector>

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
}

void SimpleScene::drawGrid(const QMatrix4x4 &vp)
{
    m_program->bind();
    m_program->setUniformValue("matrix", vp);
    m_program->setUniformValue("color", QVector4D(0.4f, 0.4f, 0.4f, 1.0f));

    QVector<GLfloat> vertices;
    const int gridSize = 100;
    const int step = 5;

    for (int i = -gridSize; i <= gridSize; i += step) {
        vertices.append(i); vertices.append(0); vertices.append(-gridSize);
        vertices.append(i); vertices.append(0); vertices.append(gridSize);
        
        vertices.append(-gridSize); vertices.append(0); vertices.append(i);
        vertices.append(gridSize); vertices.append(0); vertices.append(i);
    }

    m_program->enableAttributeArray("posAttr");
    m_program->setAttributeArray("posAttr", vertices.data(), 3);

    glLineWidth(1.0f);
    glDrawArrays(GL_LINES, 0, vertices.size() / 3);

    m_program->disableAttributeArray("posAttr");
    m_program->release();
}

void SimpleScene::drawAxes(const QMatrix4x4 &vp)
{
    m_program->bind();
    m_program->setUniformValue("matrix", vp);

    GLfloat vertices[] = {
        0.0f, 0.0f, 0.0f,  10.0f, 0.0f, 0.0f, // X
        0.0f, 0.0f, 0.0f,  0.0f, 10.0f, 0.0f, // Y
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 10.0f  // Z
    };

    m_program->enableAttributeArray("posAttr");
    m_program->setAttributeArray("posAttr", vertices, 3);

    glLineWidth(3.0f);
    
    m_program->setUniformValue("color", QVector4D(1.0f, 0.0f, 0.0f, 1.0f));
    glDrawArrays(GL_LINES, 0, 2);
    
    m_program->setUniformValue("color", QVector4D(0.0f, 1.0f, 0.0f, 1.0f));
    glDrawArrays(GL_LINES, 2, 2);
    
    m_program->setUniformValue("color", QVector4D(0.0f, 0.0f, 1.0f, 1.0f));
    glDrawArrays(GL_LINES, 4, 2);

    m_program->disableAttributeArray("posAttr");
    m_program->release();
}

void SimpleScene::drawBuildings(const QMatrix4x4 &vp)
{
    // 建筑1
    drawCube(vp, QVector3D(20, 10, -20), QVector3D(10, 20, 15), QVector4D(0.6f, 0.6f, 0.6f, 1.0f));
    // 建筑2
    drawCube(vp, QVector3D(-30, 15, -10), QVector3D(15, 30, 15), QVector4D(0.5f, 0.6f, 0.7f, 1.0f));
    // 起降点 (H)
    drawCube(vp, QVector3D(50, 0.1f, -50), QVector3D(10, 0.2f, 10), QVector4D(0.2f, 0.8f, 0.2f, 1.0f));
}

void SimpleScene::drawCube(const QMatrix4x4 &vp, const QVector3D &pos, const QVector3D &size, const QVector4D &color)
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
        0, 1, 2, 2, 3, 0, // Front
        1, 5, 6, 6, 2, 1, // Right
        5, 4, 7, 7, 6, 5, // Back
        4, 0, 3, 3, 7, 4, // Left
        3, 2, 6, 6, 7, 3, // Top
        4, 5, 1, 1, 0, 4  // Bottom
    };

    m_program->enableAttributeArray("posAttr");
    m_program->setAttributeArray("posAttr", vertices, 3);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, indices);

    m_program->disableAttributeArray("posAttr");
    m_program->release();
}
