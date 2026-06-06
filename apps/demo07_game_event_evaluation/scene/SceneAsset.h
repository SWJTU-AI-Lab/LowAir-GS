#pragma once
#include <QString>
#include <QVector3D>
#include <vector>

// 网格顶点
struct SceneVertex {
    QVector3D position;
    QVector3D normal;
    float u = 0.0f, v = 0.0f;
};

// 网格子对象
struct SceneMesh {
    std::vector<SceneVertex> vertices;
    std::vector<unsigned int> indices;
    QVector3D color = {0.7f, 0.7f, 0.7f};
};

// 高斯点（简化版，用于场景显示）
struct SceneGaussianPoint {
    QVector3D position;
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
    float scale = 0.05f;
};

// 场景资产（Mesh + Gaussian 双源）
struct SceneAsset {
    std::vector<SceneMesh>         meshes;
    std::vector<SceneGaussianPoint> gaussians;

    QVector3D bboxMin = {-10, -10, -10};
    QVector3D bboxMax = { 10,  10,  10};
    QVector3D center  = { 0,   0,   0};

    bool meshLoaded     = false;
    bool gaussianLoaded = false;
    QString meshPath;
    QString gaussianPath;

    void computeBounds();
};
