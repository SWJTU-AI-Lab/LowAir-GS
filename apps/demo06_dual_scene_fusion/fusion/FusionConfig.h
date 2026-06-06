#pragma once
#include <string>
#include <QVector3D>

// 双源融合配置（对应 demo06_fusion_config.json）
struct FusionConfig {
    std::string sceneName;
    std::string coordinateFrame;
    std::string unit;

    // Mesh 配置
    std::string meshPath;
    bool        meshEnabled  = true;
    float       meshOpacity  = 1.0f;
    std::string meshRenderMode = "solid";  // solid / wireframe / transparent

    // Gaussian 配置
    std::string gaussianPath;
    bool        gaussianEnabled  = true;
    float       gaussianOpacity  = 0.75f;
    std::string gaussianRenderMode = "splat";  // point / splat

    // 对齐参数
    QVector3D   alignTranslation = {0, 0, 0};
    QVector3D   alignRotationDeg = {0, 0, 0};
    float       alignScale       = 1.0f;

    // 显示设置
    std::string displayMode = "dual_fusion";
    bool showGrid           = true;
    bool showAxis           = true;
    bool showMeshBbox       = true;
    bool showGaussianBbox   = true;
    bool showWireframeOverlay = true;
    bool showFps            = true;

    // 融合渲染策略
    bool meshDepthWrite     = true;
    bool gaussianDepthTest  = true;
    bool gaussianDepthSort  = true;
    bool wireframeOverlay   = true;

    static FusionConfig defaultConfig();

    // JSON 文件序列化 / 反序列化
    static FusionConfig loadFromFile(const QString &path, bool *ok = nullptr);
    bool saveToFile(const QString &path) const;
};
