#include "GaussianSplatLoader.h"
#include <QFile>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QDebug>
#include <cstring>

bool GaussianSplatLoader::load(const QString &filePath, GaussianCloud &cloud, QString &errorMsg) {
    QElapsedTimer timer;
    timer.start();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        errorMsg = QString("Cannot open .splat file: %1").arg(filePath);
        return false;
    }

    QFileInfo fi(filePath);
    cloud.clear();
    cloud.filePath = filePath;
    cloud.fileSizeBytes = fi.size();
    cloud.formatType = "Splat Binary";
    cloud.hasOpacity = true;
    cloud.hasScale = true;
    cloud.hasRotation = true;
    cloud.hasSH = false;

    const int SPLAT_RECORD_SIZE = 32; // 每个 Gaussian 32 字节
    qint64 fileSize = fi.size();
    int count = (int)(fileSize / SPLAT_RECORD_SIZE);

    if (count <= 0) {
        errorMsg = "Empty or invalid .splat file";
        return false;
    }

    QByteArray data = file.readAll();
    const char *ptr = data.constData();
    cloud.points().reserve(count);

    for (int i = 0; i < count; ++i) {
        const char *rec = ptr + i * SPLAT_RECORD_SIZE;

        float px, py, pz;
        float sx, sy, sz;
        unsigned char cr, cg, cb, ca;
        unsigned char rw, rx, ry, rz;

        memcpy(&px, rec + 0,  4);
        memcpy(&py, rec + 4,  4);
        memcpy(&pz, rec + 8,  4);
        memcpy(&sx, rec + 12, 4);
        memcpy(&sy, rec + 16, 4);
        memcpy(&sz, rec + 20, 4);
        cr = (unsigned char)rec[24];
        cg = (unsigned char)rec[25];
        cb = (unsigned char)rec[26];
        ca = (unsigned char)rec[27];
        rw = (unsigned char)rec[28];
        rx = (unsigned char)rec[29];
        ry = (unsigned char)rec[30];
        rz = (unsigned char)rec[31];

        GaussianPoint pt;
        pt.position = QVector3D(px, py, pz);
        pt.color = QVector3D(cr / 255.0f, cg / 255.0f, cb / 255.0f);
        pt.opacity = ca / 255.0f;
        pt.scale = QVector3D(sx, sy, sz);

        // 旋转：[0,255] -> [-1,1]
        float qw = (rw / 128.0f) - 1.0f;
        float qx = (rx / 128.0f) - 1.0f;
        float qy = (ry / 128.0f) - 1.0f;
        float qz = (rz / 128.0f) - 1.0f;
        pt.rotation = QQuaternion(qw, qx, qy, qz).normalized();

        pt.hasOpacity = true;
        pt.hasScale = true;
        pt.hasRotation = true;
        pt.hasFullGaussianParams = true;
        cloud.addPoint(pt);
    }

    cloud.computeBoundingBox();
    cloud.loadTimeMs = timer.elapsed();

    qDebug() << "[GaussianSplatLoader] Loaded" << count << "Gaussians from" << filePath
             << "in" << cloud.loadTimeMs << "ms";
    return true;
}
