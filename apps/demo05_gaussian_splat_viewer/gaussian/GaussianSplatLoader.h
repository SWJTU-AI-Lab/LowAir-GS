#pragma once
#include <QString>
#include "GaussianCloud.h"

// .splat 格式 Gaussian 加载器
// .splat 是 antimatter15/splat 的紧凑二进制格式：
// 每个 Gaussian 32 字节：
//   position: 3 x float32 (12 bytes)
//   scale:    3 x float32 (12 bytes)
//   color:    4 x uint8   (4 bytes, RGBA)
//   rotation: 4 x uint8   (4 bytes, WXYZ 归一化到 [0,255])
class GaussianSplatLoader {
public:
    static bool load(const QString &filePath, GaussianCloud &cloud, QString &errorMsg);
};
