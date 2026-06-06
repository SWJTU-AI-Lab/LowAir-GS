#pragma once
#include "SceneAsset.h"
#include <QString>

// 场景加载器：支持 Assimp Mesh + 简化 PLY Gaussian
class SceneLoader {
public:
    // 加载 Mesh（OBJ/PLY/GLB）
    static bool loadMesh(const QString& path, SceneAsset& asset);

    // 加载 Gaussian PLY（简化版，只读位置和颜色）
    static bool loadGaussianPly(const QString& path, SceneAsset& asset);
};
