#include "SceneAsset.h"
#include <limits>

void SceneAsset::computeBounds() {
    float minX = std::numeric_limits<float>::max();
    float minY = minX, minZ = minX;
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = maxX, maxZ = maxX;

    for (const auto& mesh : meshes) {
        for (const auto& v : mesh.vertices) {
            minX = std::min(minX, v.position.x()); maxX = std::max(maxX, v.position.x());
            minY = std::min(minY, v.position.y()); maxY = std::max(maxY, v.position.y());
            minZ = std::min(minZ, v.position.z()); maxZ = std::max(maxZ, v.position.z());
        }
    }
    for (const auto& g : gaussians) {
        minX = std::min(minX, g.position.x()); maxX = std::max(maxX, g.position.x());
        minY = std::min(minY, g.position.y()); maxY = std::max(maxY, g.position.y());
        minZ = std::min(minZ, g.position.z()); maxZ = std::max(maxZ, g.position.z());
    }

    if (minX <= maxX) {
        bboxMin = {minX, minY, minZ};
        bboxMax = {maxX, maxY, maxZ};
        center  = (bboxMin + bboxMax) * 0.5f;
    }
}
