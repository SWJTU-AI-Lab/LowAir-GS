#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include "../gaussian/GaussianCloud.h"

// Splat/Billboard Mode 渲染器
// 每个 Gaussian 显示为面向相机的半透明椭圆片
// 支持按相机深度排序（近似前后排序，保证透明度混合正确）
class GaussianBillboardRenderer : protected QOpenGLExtraFunctions {
public:
    GaussianBillboardRenderer() = default;
    ~GaussianBillboardRenderer();

    void initialize();
    void upload(const GaussianCloud &cloud);
    void render(const QMatrix4x4 &mvp, const QMatrix4x4 &view,
                const QVector3D &cameraPos, float splatScale, float alphaScale);
    void cleanup();

    bool isInitialized() const { return m_initialized; }
    int splatCount() const { return m_splatCount; }

private:
    void sortByDepth(const QVector3D &cameraPos);

    bool m_initialized = false;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;   // 实例化数据
    GLuint m_quadVbo = 0; // 四边形顶点
    int m_splatCount = 0;
    QOpenGLShaderProgram m_shader;

    // 缓存数据用于深度排序
    struct SplatData {
        QVector3D pos;
        QVector3D color;
        float opacity;
        QVector3D scale;
    };
    QVector<SplatData> m_splatData;
};
