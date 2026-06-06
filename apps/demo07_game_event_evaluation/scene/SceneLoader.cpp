#include "SceneLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <cstring>

bool SceneLoader::loadMesh(const QString& path, SceneAsset& asset) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path.toStdString(),
        aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs
    );
    if (!scene || !scene->mRootNode) return false;

    asset.meshes.clear();
    for (unsigned int mi = 0; mi < scene->mNumMeshes; ++mi) {
        const aiMesh* m = scene->mMeshes[mi];
        SceneMesh mesh;
        for (unsigned int vi = 0; vi < m->mNumVertices; ++vi) {
            SceneVertex v;
            v.position = {m->mVertices[vi].x, m->mVertices[vi].y, m->mVertices[vi].z};
            if (m->HasNormals())
                v.normal = {m->mNormals[vi].x, m->mNormals[vi].y, m->mNormals[vi].z};
            if (m->HasTextureCoords(0)) {
                v.u = m->mTextureCoords[0][vi].x;
                v.v = m->mTextureCoords[0][vi].y;
            }
            mesh.vertices.push_back(v);
        }
        for (unsigned int fi = 0; fi < m->mNumFaces; ++fi) {
            const aiFace& face = m->mFaces[fi];
            for (unsigned int idx = 0; idx < face.mNumIndices; ++idx)
                mesh.indices.push_back(face.mIndices[idx]);
        }
        // 从材质获取颜色
        if (scene->mNumMaterials > m->mMaterialIndex) {
            aiColor4D diffuse;
            if (AI_SUCCESS == aiGetMaterialColor(scene->mMaterials[m->mMaterialIndex],
                                                  AI_MATKEY_COLOR_DIFFUSE, &diffuse))
                mesh.color = {diffuse.r, diffuse.g, diffuse.b};
        }
        asset.meshes.push_back(mesh);
    }
    asset.meshLoaded = true;
    asset.meshPath   = path;
    asset.computeBounds();
    return true;
}

bool SceneLoader::loadGaussianPly(const QString& path, SceneAsset& asset) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;

    asset.gaussians.clear();

    // 解析 PLY 头部
    bool headerDone = false;
    int  numVertices = 0;
    bool isBinary = false;
    bool isBigEndian = false;

    // 字段索引
    int idxX = -1, idxY = -1, idxZ = -1;
    int idxR = -1, idxG = -1, idxB = -1;
    int idxFdcR = -1, idxFdcG = -1, idxFdcB = -1;
    int fieldCount = 0;

    QTextStream ts(&file);
    while (!ts.atEnd()) {
        QString line = ts.readLine().trimmed();
        if (line == "end_header") { headerDone = true; break; }
        if (line.startsWith("format binary_little_endian")) isBinary = true;
        if (line.startsWith("format binary_big_endian")) { isBinary = true; isBigEndian = true; }
        if (line.startsWith("element vertex")) numVertices = line.split(" ").last().toInt();
        if (line.startsWith("property float")) {
            QString name = line.split(" ").last();
            if (name == "x")     idxX    = fieldCount;
            if (name == "y")     idxY    = fieldCount;
            if (name == "z")     idxZ    = fieldCount;
            if (name == "red")   idxR    = fieldCount;
            if (name == "green") idxG    = fieldCount;
            if (name == "blue")  idxB    = fieldCount;
            if (name == "f_dc_0") idxFdcR = fieldCount;
            if (name == "f_dc_1") idxFdcG = fieldCount;
            if (name == "f_dc_2") idxFdcB = fieldCount;
            fieldCount++;
        }
        if (line.startsWith("property uchar")) {
            QString name = line.split(" ").last();
            if (name == "red")   idxR = fieldCount;
            if (name == "green") idxG = fieldCount;
            if (name == "blue")  idxB = fieldCount;
            fieldCount++;
        }
    }

    if (!headerDone || numVertices == 0) { file.close(); return false; }

    if (isBinary) {
        // 二进制读取
        QByteArray data = file.readAll();
        file.close();
        const char* ptr = data.constData();
        int stride = fieldCount * 4; // 假设全 float
        for (int i = 0; i < numVertices && (i * stride + stride) <= data.size(); ++i) {
            const float* row = reinterpret_cast<const float*>(ptr + i * stride);
            SceneGaussianPoint p;
            if (idxX >= 0) p.position.setX(row[idxX]);
            if (idxY >= 0) p.position.setY(row[idxY]);
            if (idxZ >= 0) p.position.setZ(row[idxZ]);
            // 颜色：优先 f_dc（SH 直流分量），其次 rgb
            if (idxFdcR >= 0) {
                p.r = std::max(0.0f, std::min(1.0f, 0.5f + row[idxFdcR] * 0.2821f));
                p.g = std::max(0.0f, std::min(1.0f, 0.5f + (idxFdcG >= 0 ? row[idxFdcG] : 0.0f) * 0.2821f));
                p.b = std::max(0.0f, std::min(1.0f, 0.5f + (idxFdcB >= 0 ? row[idxFdcB] : 0.0f) * 0.2821f));
            } else if (idxR >= 0) {
                p.r = row[idxR] / 255.0f;
                p.g = idxG >= 0 ? row[idxG] / 255.0f : 1.0f;
                p.b = idxB >= 0 ? row[idxB] / 255.0f : 1.0f;
            }
            asset.gaussians.push_back(p);
        }
    } else {
        // ASCII 读取
        for (int i = 0; i < numVertices && !ts.atEnd(); ++i) {
            QString line = ts.readLine().trimmed();
            QStringList parts = line.split(" ");
            if (parts.size() < fieldCount) continue;
            SceneGaussianPoint p;
            if (idxX >= 0 && idxX < parts.size()) p.position.setX(parts[idxX].toFloat());
            if (idxY >= 0 && idxY < parts.size()) p.position.setY(parts[idxY].toFloat());
            if (idxZ >= 0 && idxZ < parts.size()) p.position.setZ(parts[idxZ].toFloat());
            if (idxFdcR >= 0 && idxFdcR < parts.size()) {
                p.r = std::max(0.0f, std::min(1.0f, 0.5f + parts[idxFdcR].toFloat() * 0.2821f));
                p.g = std::max(0.0f, std::min(1.0f, 0.5f + (idxFdcG >= 0 ? parts[idxFdcG].toFloat() : 0.0f) * 0.2821f));
                p.b = std::max(0.0f, std::min(1.0f, 0.5f + (idxFdcB >= 0 ? parts[idxFdcB].toFloat() : 0.0f) * 0.2821f));
            } else if (idxR >= 0 && idxR < parts.size()) {
                p.r = parts[idxR].toFloat() / 255.0f;
                p.g = idxG >= 0 ? parts[idxG].toFloat() / 255.0f : 1.0f;
                p.b = idxB >= 0 ? parts[idxB].toFloat() / 255.0f : 1.0f;
            }
            asset.gaussians.push_back(p);
        }
        file.close();
    }

    asset.gaussianLoaded = true;
    asset.gaussianPath   = path;
    asset.computeBounds();
    return true;
}
