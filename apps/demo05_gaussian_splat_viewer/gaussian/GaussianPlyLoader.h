#pragma once
#include <QString>
#include <QStringList>
#include "GaussianCloud.h"

// PLY 格式 Gaussian 加载器
// 支持 ASCII PLY 和 Binary Little Endian PLY
// 支持标准 3DGS 字段（f_dc_0/1/2, opacity, scale_0/1/2, rot_0/1/2/3）
// 支持教学简化字段（x/y/z/red/green/blue/alpha/radius）
class GaussianPlyLoader {
public:
    // 加载 PLY 文件，返回是否成功
    static bool load(const QString &filePath, GaussianCloud &cloud, QString &errorMsg);

private:
    // sigmoid 函数：将 raw opacity 转换为 [0,1]
    static float sigmoid(float x) {
        return 1.0f / (1.0f + std::exp(-x));
    }

    // SH DC 颜色转换：f_dc -> RGB [0,1]
    // 标准 3DGS: color = 0.5 + SH_C0 * f_dc，其中 SH_C0 = 0.28209479177387814
    static float shDcToRgb(float f_dc) {
        const float SH_C0 = 0.28209479177387814f;
        float val = 0.5f + SH_C0 * f_dc;
        return qBound(0.0f, val, 1.0f);
    }
};
