#pragma once
#include <QVector3D>
#include <QQuaternion>

// 单个 Gaussian 点的数据结构
// 支持教学简化字段（x/y/z/red/green/blue/alpha/radius）
// 和标准 3DGS 字段（f_dc_0/1/2, opacity, scale_0/1/2, rot_0/1/2/3）
struct GaussianPoint {
    QVector3D position;                           // 世界坐标 (x, y, z)
    QVector3D color;                              // 归一化 RGB [0,1]
    float opacity = 1.0f;                         // 不透明度 [0,1]

    QVector3D scale = QVector3D(0.05f, 0.05f, 0.05f); // 各轴缩放（对数空间或线性）
    QQuaternion rotation;                         // 旋转四元数（单位四元数为默认）

    bool hasFullGaussianParams = false;           // 是否包含完整 3DGS 参数
    bool hasOpacity = false;                      // 是否有 opacity 字段
    bool hasScale = false;                        // 是否有 scale 字段
    bool hasRotation = false;                     // 是否有 rotation 字段
    bool hasSH = false;                           // 是否有球谐 / f_dc 字段

    GaussianPoint() {
        rotation = QQuaternion(1.0f, 0.0f, 0.0f, 0.0f); // 单位四元数
    }
};
