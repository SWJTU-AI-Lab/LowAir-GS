#ifndef SIMPLESCENE_H
#define SIMPLESCENE_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>

class SimpleScene : protected QOpenGLFunctions
{
public:
    SimpleScene(QObject *parent = nullptr);
    ~SimpleScene();

    void initialize();
    void render(const QMatrix4x4 &projection, const QMatrix4x4 &view);

private:
    void drawGrid(const QMatrix4x4 &vp);
    void drawAxes(const QMatrix4x4 &vp);
    void drawBuildings(const QMatrix4x4 &vp);
    void drawCube(const QMatrix4x4 &vp, const QVector3D &pos, const QVector3D &size, const QVector4D &color);

    QOpenGLShaderProgram *m_program;
};

#endif // SIMPLESCENE_H
