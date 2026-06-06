#include "GaussianPlyLoader.h"
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QDebug>
#include <cmath>
#include <cstring>

bool GaussianPlyLoader::load(const QString &filePath, GaussianCloud &cloud, QString &errorMsg) {
    QElapsedTimer timer;
    timer.start();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        errorMsg = QString("Cannot open file: %1").arg(filePath);
        return false;
    }

    QFileInfo fi(filePath);
    cloud.clear();
    cloud.filePath = filePath;
    cloud.fileSizeBytes = fi.size();

    // 解析 PLY 头部
    bool headerDone = false;
    int vertexCount = 0;
    bool isBinaryLE = false;
    bool isAscii = false;

    // 字段索引映射
    struct FieldDef {
        QString name;
        int index = -1;
        QString type; // float, uchar, int, double
    };

    QVector<FieldDef> fields;
    int fieldCount = 0;

    QByteArray headerData;
    while (!file.atEnd()) {
        QByteArray line = file.readLine().trimmed();
        headerData.append(line);
        headerData.append('\n');
        QString lineStr = QString::fromLatin1(line);

        if (lineStr.startsWith("format ascii")) {
            isAscii = true;
        } else if (lineStr.startsWith("format binary_little_endian")) {
            isBinaryLE = true;
        } else if (lineStr.startsWith("element vertex")) {
            QStringList parts = lineStr.split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                vertexCount = parts[2].toInt();
            }
        } else if (lineStr.startsWith("property")) {
            QStringList parts = lineStr.split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                FieldDef fd;
                fd.type = parts[1];
                fd.name = parts[2];
                fd.index = fieldCount++;
                fields.append(fd);
            }
        } else if (lineStr == "end_header") {
            headerDone = true;
            break;
        }
    }

    if (!headerDone) {
        errorMsg = "Invalid PLY file: no end_header found";
        return false;
    }

    if (vertexCount <= 0) {
        errorMsg = "PLY file has no vertices";
        return false;
    }

    // 建立字段名到索引的映射
    auto findField = [&](const QString &name) -> int {
        for (const auto &f : fields) {
            if (f.name == name) return f.index;
        }
        return -1;
    };

    int idx_x = findField("x");
    int idx_y = findField("y");
    int idx_z = findField("z");
    int idx_r = findField("red");
    int idx_g = findField("green");
    int idx_b = findField("blue");
    int idx_a = findField("alpha");
    int idx_radius = findField("radius");
    int idx_opacity = findField("opacity");
    int idx_scale0 = findField("scale_0");
    int idx_scale1 = findField("scale_1");
    int idx_scale2 = findField("scale_2");
    int idx_rot0 = findField("rot_0");
    int idx_rot1 = findField("rot_1");
    int idx_rot2 = findField("rot_2");
    int idx_rot3 = findField("rot_3");
    int idx_fdc0 = findField("f_dc_0");
    int idx_fdc1 = findField("f_dc_1");
    int idx_fdc2 = findField("f_dc_2");

    bool hasFullGS = (idx_fdc0 >= 0 && idx_opacity >= 0 && idx_scale0 >= 0 && idx_rot0 >= 0);
    bool hasSimple = (idx_r >= 0 && idx_g >= 0 && idx_b >= 0);

    cloud.hasOpacity = (idx_opacity >= 0 || idx_a >= 0);
    cloud.hasScale = (idx_scale0 >= 0 || idx_radius >= 0);
    cloud.hasRotation = (idx_rot0 >= 0);
    cloud.hasSH = (idx_fdc0 >= 0);

    if (hasFullGS) {
        cloud.formatType = "Standard 3DGS PLY";
    } else if (hasSimple) {
        cloud.formatType = "Simplified Gaussian PLY";
    } else {
        cloud.formatType = "Generic PLY";
    }

    cloud.points().reserve(vertexCount);

    // 计算每个顶点的字节大小（用于二进制解析）
    auto fieldByteSize = [](const QString &type) -> int {
        if (type == "float") return 4;
        if (type == "double") return 8;
        if (type == "uchar" || type == "uint8") return 1;
        if (type == "int" || type == "uint" || type == "int32" || type == "uint32") return 4;
        if (type == "short" || type == "int16") return 2;
        if (type == "ushort" || type == "uint16") return 2;
        return 4; // 默认 float
    };

    int vertexByteSize = 0;
    QVector<int> fieldOffsets(fields.size());
    for (int i = 0; i < fields.size(); ++i) {
        fieldOffsets[i] = vertexByteSize;
        vertexByteSize += fieldByteSize(fields[i].type);
    }

    auto readFloat = [&](const QByteArray &buf, int offset, const QString &type) -> float {
        if (type == "float") {
            float v;
            memcpy(&v, buf.constData() + offset, 4);
            return v;
        } else if (type == "double") {
            double v;
            memcpy(&v, buf.constData() + offset, 8);
            return (float)v;
        } else if (type == "uchar" || type == "uint8") {
            unsigned char v = (unsigned char)buf[offset];
            return v / 255.0f;
        } else if (type == "int" || type == "int32") {
            int v;
            memcpy(&v, buf.constData() + offset, 4);
            return (float)v;
        }
        float v;
        memcpy(&v, buf.constData() + offset, 4);
        return v;
    };

    int loaded = 0;
    int warnings = 0;

    if (isAscii) {
        // ASCII 解析
        for (int vi = 0; vi < vertexCount && !file.atEnd(); ++vi) {
            QByteArray line = file.readLine().trimmed();
            QStringList tokens = QString::fromLatin1(line).split(' ', Qt::SkipEmptyParts);

            if (tokens.size() < fields.size()) {
                if (warnings < 5) {
                    qWarning() << "[GaussianPlyLoader] Warning: vertex" << vi
                               << "has fewer tokens than expected, using defaults";
                    ++warnings;
                }
            }

            auto getVal = [&](int idx) -> float {
                if (idx < 0 || idx >= tokens.size()) return 0.0f;
                return tokens[idx].toFloat();
            };

            GaussianPoint pt;
            pt.position = QVector3D(
                idx_x >= 0 ? getVal(idx_x) : 0.0f,
                idx_y >= 0 ? getVal(idx_y) : 0.0f,
                idx_z >= 0 ? getVal(idx_z) : 0.0f
            );

            // 颜色
            if (idx_fdc0 >= 0) {
                pt.color = QVector3D(
                    shDcToRgb(getVal(idx_fdc0)),
                    shDcToRgb(idx_fdc1 >= 0 ? getVal(idx_fdc1) : 0.0f),
                    shDcToRgb(idx_fdc2 >= 0 ? getVal(idx_fdc2) : 0.0f)
                );
                pt.hasSH = true;
            } else if (idx_r >= 0) {
                // uchar [0,255] 或 float [0,1]
                float rv = getVal(idx_r);
                float gv = idx_g >= 0 ? getVal(idx_g) : 0.0f;
                float bv = idx_b >= 0 ? getVal(idx_b) : 0.0f;
                // 判断是否是 uchar 类型
                if (fields[idx_r].type == "uchar" || fields[idx_r].type == "uint8") {
                    // 已经在 getVal 中归一化了，但 ASCII 读到的是原始整数
                    // ASCII 模式下直接读整数再除以255
                    rv = qBound(0.0f, rv / 255.0f, 1.0f);
                    gv = qBound(0.0f, gv / 255.0f, 1.0f);
                    bv = qBound(0.0f, bv / 255.0f, 1.0f);
                }
                pt.color = QVector3D(rv, gv, bv);
            } else {
                pt.color = QVector3D(0.8f, 0.8f, 0.8f);
            }

            // 透明度
            if (idx_opacity >= 0) {
                pt.opacity = sigmoid(getVal(idx_opacity));
                pt.hasOpacity = true;
            } else if (idx_a >= 0) {
                float av = getVal(idx_a);
                if (fields[idx_a].type == "uchar" || fields[idx_a].type == "uint8") {
                    av = av / 255.0f;
                }
                pt.opacity = qBound(0.0f, av, 1.0f);
                pt.hasOpacity = true;
            }

            // 尺度
            if (idx_scale0 >= 0) {
                // 标准 3DGS: scale 在对数空间，需要 exp
                float s0 = std::exp(getVal(idx_scale0));
                float s1 = idx_scale1 >= 0 ? std::exp(getVal(idx_scale1)) : s0;
                float s2 = idx_scale2 >= 0 ? std::exp(getVal(idx_scale2)) : s0;
                pt.scale = QVector3D(s0, s1, s2);
                pt.hasScale = true;
            } else if (idx_radius >= 0) {
                float r = getVal(idx_radius);
                pt.scale = QVector3D(r, r, r);
                pt.hasScale = true;
            }

            // 旋转
            if (idx_rot0 >= 0) {
                float w = getVal(idx_rot0);
                float rx = idx_rot1 >= 0 ? getVal(idx_rot1) : 0.0f;
                float ry = idx_rot2 >= 0 ? getVal(idx_rot2) : 0.0f;
                float rz = idx_rot3 >= 0 ? getVal(idx_rot3) : 0.0f;
                pt.rotation = QQuaternion(w, rx, ry, rz).normalized();
                pt.hasRotation = true;
            }

            pt.hasFullGaussianParams = hasFullGS;
            cloud.addPoint(pt);
            ++loaded;
        }
    } else if (isBinaryLE) {
        // Binary Little Endian 解析
        QByteArray allData = file.readAll();
        const char *ptr = allData.constData();
        int totalBytes = allData.size();

        for (int vi = 0; vi < vertexCount; ++vi) {
            int base = vi * vertexByteSize;
            if (base + vertexByteSize > totalBytes) {
                qWarning() << "[GaussianPlyLoader] Warning: unexpected end of binary data at vertex" << vi;
                break;
            }

            QByteArray vertBuf(ptr + base, vertexByteSize);

            auto getValB = [&](int idx) -> float {
                if (idx < 0 || idx >= fields.size()) return 0.0f;
                return readFloat(vertBuf, fieldOffsets[idx], fields[idx].type);
            };

            GaussianPoint pt;
            pt.position = QVector3D(getValB(idx_x), getValB(idx_y), getValB(idx_z));

            if (idx_fdc0 >= 0) {
                pt.color = QVector3D(
                    shDcToRgb(getValB(idx_fdc0)),
                    shDcToRgb(idx_fdc1 >= 0 ? getValB(idx_fdc1) : 0.0f),
                    shDcToRgb(idx_fdc2 >= 0 ? getValB(idx_fdc2) : 0.0f)
                );
                pt.hasSH = true;
            } else if (idx_r >= 0) {
                pt.color = QVector3D(
                    qBound(0.0f, getValB(idx_r), 1.0f),
                    qBound(0.0f, idx_g >= 0 ? getValB(idx_g) : 0.0f, 1.0f),
                    qBound(0.0f, idx_b >= 0 ? getValB(idx_b) : 0.0f, 1.0f)
                );
            } else {
                pt.color = QVector3D(0.8f, 0.8f, 0.8f);
            }

            if (idx_opacity >= 0) {
                pt.opacity = sigmoid(getValB(idx_opacity));
                pt.hasOpacity = true;
            } else if (idx_a >= 0) {
                pt.opacity = qBound(0.0f, getValB(idx_a), 1.0f);
                pt.hasOpacity = true;
            }

            if (idx_scale0 >= 0) {
                float s0 = std::exp(getValB(idx_scale0));
                float s1 = idx_scale1 >= 0 ? std::exp(getValB(idx_scale1)) : s0;
                float s2 = idx_scale2 >= 0 ? std::exp(getValB(idx_scale2)) : s0;
                pt.scale = QVector3D(s0, s1, s2);
                pt.hasScale = true;
            } else if (idx_radius >= 0) {
                float r = getValB(idx_radius);
                pt.scale = QVector3D(r, r, r);
                pt.hasScale = true;
            }

            if (idx_rot0 >= 0) {
                float w = getValB(idx_rot0);
                float rx = idx_rot1 >= 0 ? getValB(idx_rot1) : 0.0f;
                float ry = idx_rot2 >= 0 ? getValB(idx_rot2) : 0.0f;
                float rz = idx_rot3 >= 0 ? getValB(idx_rot3) : 0.0f;
                pt.rotation = QQuaternion(w, rx, ry, rz).normalized();
                pt.hasRotation = true;
            }

            pt.hasFullGaussianParams = hasFullGS;
            cloud.addPoint(pt);
            ++loaded;
        }
    } else {
        errorMsg = "Unsupported PLY format (only ASCII and binary_little_endian supported)";
        return false;
    }

    cloud.computeBoundingBox();
    cloud.loadTimeMs = timer.elapsed();

    qDebug() << "[GaussianPlyLoader] Loaded" << loaded << "Gaussians from" << filePath
             << "in" << cloud.loadTimeMs << "ms";
    return true;
}
