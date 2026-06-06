#!/usr/bin/env python3
"""
Demo-06 示例数据生成脚本
生成：
  1. sample_data/demo06_fusion/building_mesh.obj   — 简单建筑 Mesh（OBJ 格式）
  2. sample_data/demo06_fusion/building_gaussians.ply — 对应的 Gaussian 点云（ASCII PLY）
  3. sample_data/demo06_fusion/fusion_config.json  — 融合配置文件
"""

import os
import json
import math
import struct

OUT_DIR = os.path.join(os.path.dirname(__file__), "../../sample_data/demo06_fusion")
os.makedirs(OUT_DIR, exist_ok=True)

# ─────────────────────────────────────────────────────────────────────────────
# 1. 生成建筑 Mesh（OBJ）：一个 4m×4m×6m 的方形建筑 + 屋顶
# ─────────────────────────────────────────────────────────────────────────────
def write_building_obj(path):
    """生成简单建筑 OBJ：底面 4×4，高度 6，加一个三角屋顶"""
    verts = []
    faces = []

    # 建筑主体（长方体）
    # 底面 y=0，顶面 y=6，x/z 范围 [-2, 2]
    bx, bz = 2.0, 2.0
    hy = 6.0

    # 8 个顶点
    corners = [
        (-bx, 0,  bz),  # 0
        ( bx, 0,  bz),  # 1
        ( bx, 0, -bz),  # 2
        (-bx, 0, -bz),  # 3
        (-bx, hy,  bz), # 4
        ( bx, hy,  bz), # 5
        ( bx, hy, -bz), # 6
        (-bx, hy, -bz), # 7
    ]
    for v in corners:
        verts.append(v)

    # 6 个面（每面2个三角形）
    box_faces = [
        (1,2,6,5),  # front
        (2,3,7,6),  # right
        (3,4,8,7),  # back
        (4,1,5,8),  # left
        (5,6,7,8),  # top
        (4,3,2,1),  # bottom
    ]
    for f in box_faces:
        a, b, c, d = f
        faces.append((a, b, c))
        faces.append((a, c, d))

    # 屋顶（三角棱柱）
    roof_peak_front = (0, hy + 2.0,  bz)  # 9
    roof_peak_back  = (0, hy + 2.0, -bz)  # 10
    verts.append(roof_peak_front)  # index 9
    verts.append(roof_peak_back)   # index 10

    # 屋顶三角面
    roof_faces = [
        (5, 6, 10, 9),  # 屋顶前坡（四边形 → 2 三角）
        (5, 9, 8, 4),   # 左坡
        (6, 7, 10),     # 右坡前
        (7, 8, 9, 10),  # 后坡
        (4, 8, 7, 5),   # 左侧（已有，跳过）
    ]
    # 简化：只加两个屋顶三角面
    faces.append((5, 9, 8))
    faces.append((5, 6, 9))
    faces.append((6, 10, 9))
    faces.append((6, 7, 10))

    with open(path, 'w') as f:
        f.write("# Demo-06 Building Mesh\n")
        f.write("# Simple 4x4x6 building with triangular roof\n\n")
        f.write("mtllib building_mesh.mtl\n")
        f.write("o Building\n\n")
        for v in verts:
            f.write(f"v {v[0]:.4f} {v[1]:.4f} {v[2]:.4f}\n")
        f.write("\nusemtl building_mat\n")
        for face in faces:
            f.write("f " + " ".join(str(i) for i in face) + "\n")

    # 写 MTL
    mtl_path = path.replace(".obj", ".mtl")
    with open(mtl_path, 'w') as f:
        f.write("# Demo-06 Building Material\n")
        f.write("newmtl building_mat\n")
        f.write("Kd 0.8 0.75 0.65\n")  # 米黄色
        f.write("Ka 0.2 0.2 0.2\n")
        f.write("Ks 0.1 0.1 0.1\n")
        f.write("Ns 10\n")

    print(f"[OBJ] {len(verts)} vertices, {len(faces)} triangles -> {path}")

# ─────────────────────────────────────────────────────────────────────────────
# 2. 生成 Gaussian 点云（ASCII PLY）
#    点云覆盖建筑表面，带颜色和简单缩放
# ─────────────────────────────────────────────────────────────────────────────
def generate_building_gaussians(n_per_face=200):
    """在建筑各面上均匀采样 Gaussian 点"""
    import random
    random.seed(42)

    points = []
    bx, bz = 2.0, 2.0
    hy = 6.0

    def rand():
        return random.random()

    # 前面 (z=bz)：米黄色
    for _ in range(n_per_face):
        x = -bx + rand() * 2 * bx
        y = rand() * hy
        z = bz
        points.append((x, y, z, 0.85, 0.80, 0.65, 0.9, 0.08, 0.08, 0.08))

    # 后面 (z=-bz)：米黄色
    for _ in range(n_per_face):
        x = -bx + rand() * 2 * bx
        y = rand() * hy
        z = -bz
        points.append((x, y, z, 0.85, 0.80, 0.65, 0.9, 0.08, 0.08, 0.08))

    # 左面 (x=-bx)：浅灰色
    for _ in range(n_per_face):
        x = -bx
        y = rand() * hy
        z = -bz + rand() * 2 * bz
        points.append((x, y, z, 0.75, 0.75, 0.75, 0.9, 0.08, 0.08, 0.08))

    # 右面 (x=bx)：浅灰色
    for _ in range(n_per_face):
        x = bx
        y = rand() * hy
        z = -bz + rand() * 2 * bz
        points.append((x, y, z, 0.75, 0.75, 0.75, 0.9, 0.08, 0.08, 0.08))

    # 顶面 (y=hy)：深灰色
    for _ in range(n_per_face):
        x = -bx + rand() * 2 * bx
        y = hy
        z = -bz + rand() * 2 * bz
        points.append((x, y, z, 0.55, 0.55, 0.55, 0.85, 0.12, 0.12, 0.04))

    # 屋顶（斜面）：红棕色
    for _ in range(n_per_face):
        t = rand()
        side = random.choice([-1, 1])
        x = side * (bx * (1 - t))
        y = hy + t * 2.0
        z = -bz + rand() * 2 * bz
        points.append((x, y, z, 0.72, 0.35, 0.25, 0.9, 0.10, 0.10, 0.06))

    # 地面周围（绿色草地）
    for _ in range(n_per_face // 2):
        x = -bx * 2 + rand() * 4 * bx
        y = 0.0
        z = -bz * 2 + rand() * 4 * bz
        points.append((x, y, z, 0.35, 0.65, 0.30, 0.8, 0.15, 0.15, 0.03))

    return points

def write_gaussian_ply(path, points):
    """写入 ASCII PLY 格式"""
    n = len(points)
    with open(path, 'w') as f:
        f.write("ply\n")
        f.write("format ascii 1.0\n")
        f.write(f"element vertex {n}\n")
        f.write("property float x\n")
        f.write("property float y\n")
        f.write("property float z\n")
        f.write("property float red\n")
        f.write("property float green\n")
        f.write("property float blue\n")
        f.write("property float opacity\n")
        f.write("property float scale_0\n")
        f.write("property float scale_1\n")
        f.write("property float scale_2\n")
        f.write("end_header\n")
        for p in points:
            f.write(f"{p[0]:.4f} {p[1]:.4f} {p[2]:.4f} "
                    f"{p[3]:.3f} {p[4]:.3f} {p[5]:.3f} "
                    f"{p[6]:.3f} {p[7]:.4f} {p[8]:.4f} {p[9]:.4f}\n")
    print(f"[PLY] {n} Gaussians -> {path}")

# ─────────────────────────────────────────────────────────────────────────────
# 3. 生成融合配置文件
# ─────────────────────────────────────────────────────────────────────────────
def write_fusion_config(path):
    config = {
        "mesh_file": "building_mesh.obj",
        "gaussian_file": "building_gaussians.ply",
        "fusion": {
            "translation": [0.0, 0.0, 0.0],
            "rotation_deg": [0.0, 0.0, 0.0],
            "scale": 1.0,
            "mesh_opacity": 0.7,
            "gaussian_opacity": 0.85,
            "display_mode": "DualFusion"
        },
        "scene": {
            "origin_lat": 30.5728,
            "origin_lon": 104.0668,
            "origin_alt": 500.0,
            "description": "Demo-06 building fusion sample"
        }
    }
    with open(path, 'w') as f:
        json.dump(config, f, indent=2, ensure_ascii=False)
    print(f"[JSON] fusion config -> {path}")

# ─────────────────────────────────────────────────────────────────────────────
# 主程序
# ─────────────────────────────────────────────────────────────────────────────
if __name__ == "__main__":
    obj_path  = os.path.join(OUT_DIR, "building_mesh.obj")
    ply_path  = os.path.join(OUT_DIR, "building_gaussians.ply")
    json_path = os.path.join(OUT_DIR, "fusion_config.json")

    write_building_obj(obj_path)
    pts = generate_building_gaussians(n_per_face=200)
    write_gaussian_ply(ply_path, pts)
    write_fusion_config(json_path)

    print(f"\nDemo-06 sample data generated in: {OUT_DIR}")
    print(f"  Mesh:    {os.path.basename(obj_path)}")
    print(f"  Gaussian:{os.path.basename(ply_path)} ({len(pts)} points)")
    print(f"  Config:  {os.path.basename(json_path)}")
