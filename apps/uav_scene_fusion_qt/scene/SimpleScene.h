#ifndef SIMPLESCENE_H
#define SIMPLESCENE_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>

/**
 * @brief SimpleScene — Demo-01 程序内置简化低空演示场景
 *
 * 包含以下元素（全部由程序内部生成，不依赖外部文件）：
 *   - 地面网格（100×100m，5m 间距）
 *   - XYZ 坐标轴（红/绿/蓝，各 15m）
 *   - 5 栋简化建筑块（不同高度和颜色）
 *   - 起降点圆环 + H 标记（绿色）
 *   - 参考椭圆航线 + 目标点（黄色）
 *
 * 不包含：
 *   - 外部 OBJ/PLY/GLB 模型加载（Demo-02）
 *   - 真实地理坐标（Demo-03）
 */
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
    void drawLandingPad(const QMatrix4x4 &vp);
    void drawRefRoute(const QMatrix4x4 &vp);
    void drawCube(const QMatrix4x4 &vp, const QVector3D &pos,
                  const QVector3D &size, const QVector4D &color);

    QOpenGLShaderProgram *m_program;
};

#endif // SIMPLESCENE_H
