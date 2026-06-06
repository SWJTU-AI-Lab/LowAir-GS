"""
Demo-05 Gaussian 示例数据生成脚本
生成两个教学用途的 Gaussian PLY 文件：
  1. tiny_colored_gaussians.ply  - 小型彩色 Gaussian 点云（200 个，球形分布）
  2. lowair_demo_gaussian_cloud.ply - 低空场景风格 Gaussian 点云（3000 个，地面+建筑+植被）

生成的数据是程序生成的教学样例，不是真实 3DGS 训练结果。
真实 3DGS 模型请通过 3DGS/Gaussian Splatting 训练流程获得，不要提交到 GitHub。
"""

import struct
import math
import random
import os
import json

OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "../../sample_data/demo05_gaussians")
os.makedirs(OUTPUT_DIR, exist_ok=True)

random.seed(42)


def sigmoid(x):
    return 1.0 / (1.0 + math.exp(-x))


def rgb_to_sh_dc(r, g, b):
    """RGB [0,1] -> f_dc (SH DC 系数)，逆变换：f_dc = (color - 0.5) / SH_C0"""
    SH_C0 = 0.28209479177387814
    return (r - 0.5) / SH_C0, (g - 0.5) / SH_C0, (b - 0.5) / SH_C0


def opacity_to_raw(alpha):
    """alpha [0,1] -> raw opacity（sigmoid 逆变换：logit）"""
    alpha = max(1e-6, min(1 - 1e-6, alpha))
    return math.log(alpha / (1 - alpha))


def scale_to_log(s):
    """线性尺度 -> 对数尺度（标准 3DGS 存储格式）"""
    return math.log(max(s, 1e-6))


def write_ply_ascii(filepath, gaussians):
    """
    写入 ASCII PLY 格式的 Gaussian 数据
    字段：x y z f_dc_0 f_dc_1 f_dc_2 opacity scale_0 scale_1 scale_2 rot_0 rot_1 rot_2 rot_3
    """
    with open(filepath, 'w') as f:
        f.write("ply\n")
        f.write("format ascii 1.0\n")
        f.write(f"element vertex {len(gaussians)}\n")
        f.write("property float x\n")
        f.write("property float y\n")
        f.write("property float z\n")
        f.write("property float f_dc_0\n")
        f.write("property float f_dc_1\n")
        f.write("property float f_dc_2\n")
        f.write("property float opacity\n")
        f.write("property float scale_0\n")
        f.write("property float scale_1\n")
        f.write("property float scale_2\n")
        f.write("property float rot_0\n")
        f.write("property float rot_1\n")
        f.write("property float rot_2\n")
        f.write("property float rot_3\n")
        f.write("end_header\n")
        for g in gaussians:
            x, y, z = g['pos']
            r, gr, b = g['color']
            fdc0, fdc1, fdc2 = rgb_to_sh_dc(r, gr, b)
            raw_opacity = opacity_to_raw(g['opacity'])
            s0 = scale_to_log(g['scale'][0])
            s1 = scale_to_log(g['scale'][1])
            s2 = scale_to_log(g['scale'][2])
            rot = g.get('rot', (1.0, 0.0, 0.0, 0.0))
            f.write(f"{x:.6f} {y:.6f} {z:.6f} "
                    f"{fdc0:.6f} {fdc1:.6f} {fdc2:.6f} "
                    f"{raw_opacity:.6f} "
                    f"{s0:.6f} {s1:.6f} {s2:.6f} "
                    f"{rot[0]:.6f} {rot[1]:.6f} {rot[2]:.6f} {rot[3]:.6f}\n")
    print(f"[generate] Written {len(gaussians)} Gaussians to {filepath}")


def gen_tiny_colored_gaussians():
    """生成 200 个彩色 Gaussian，球形分布，颜色按方位角变化"""
    gaussians = []
    N = 200
    for i in range(N):
        # 球面均匀分布
        theta = random.uniform(0, 2 * math.pi)
        phi = math.acos(random.uniform(-1, 1))
        r = random.uniform(0.5, 2.0)
        x = r * math.sin(phi) * math.cos(theta)
        y = r * math.sin(phi) * math.sin(theta)
        z = r * math.cos(phi)

        # 颜色按方位角变化（彩虹效果）
        hue = theta / (2 * math.pi)
        # HSV to RGB (简化)
        h = hue * 6
        c = 0.8
        x_c = c * (1 - abs(h % 2 - 1))
        if h < 1: rc, gc, bc = c, x_c, 0
        elif h < 2: rc, gc, bc = x_c, c, 0
        elif h < 3: rc, gc, bc = 0, c, x_c
        elif h < 4: rc, gc, bc = 0, x_c, c
        elif h < 5: rc, gc, bc = x_c, 0, c
        else: rc, gc, bc = c, 0, x_c
        rc += 0.2; gc += 0.2; bc += 0.2

        scale = random.uniform(0.03, 0.12)
        gaussians.append({
            'pos': (x, y, z),
            'color': (min(rc, 1.0), min(gc, 1.0), min(bc, 1.0)),
            'opacity': random.uniform(0.7, 1.0),
            'scale': (scale, scale * random.uniform(0.5, 1.5), scale * random.uniform(0.5, 1.5)),
            'rot': (1.0, 0.0, 0.0, 0.0),
        })
    return gaussians


def gen_lowair_demo_gaussian_cloud():
    """
    生成 3000 个低空场景风格 Gaussian 点云
    包含：地面层（绿色/棕色）、建筑层（灰色/白色）、植被层（深绿色）
    模拟低空无人机视角下的典型场景语义
    """
    gaussians = []

    # 地面层：1200 个，平铺在 z=0 附近
    for _ in range(1200):
        x = random.uniform(-8, 8)
        y = random.uniform(-8, 8)
        z = random.gauss(0, 0.05)
        # 地面颜色：绿色/棕色混合
        if random.random() < 0.6:
            r = random.uniform(0.3, 0.5)
            g = random.uniform(0.5, 0.7)
            b = random.uniform(0.1, 0.3)
        else:
            r = random.uniform(0.5, 0.7)
            g = random.uniform(0.4, 0.6)
            b = random.uniform(0.2, 0.4)
        scale_xy = random.uniform(0.05, 0.2)
        scale_z = random.uniform(0.01, 0.05)
        gaussians.append({
            'pos': (x, y, z),
            'color': (r, g, b),
            'opacity': random.uniform(0.8, 1.0),
            'scale': (scale_xy, scale_xy, scale_z),
            'rot': (1.0, 0.0, 0.0, 0.0),
        })

    # 建筑层：600 个，分布在几个矩形区域，高度 1~4m
    building_centers = [(-4, -4), (4, -4), (-4, 4), (4, 4), (0, 0)]
    for _ in range(600):
        cx, cy = random.choice(building_centers)
        x = cx + random.uniform(-1.5, 1.5)
        y = cy + random.uniform(-1.5, 1.5)
        z = random.uniform(0.5, 4.0)
        # 建筑颜色：灰色/白色
        v = random.uniform(0.55, 0.85)
        r, g, b = v, v, v + random.uniform(-0.05, 0.05)
        scale = random.uniform(0.04, 0.15)
        gaussians.append({
            'pos': (x, y, z),
            'color': (min(r, 1.0), min(g, 1.0), min(b, 1.0)),
            'opacity': random.uniform(0.85, 1.0),
            'scale': (scale, scale, scale * random.uniform(0.5, 2.0)),
            'rot': (1.0, 0.0, 0.0, 0.0),
        })

    # 植被层：800 个，树冠形状（球形簇）
    tree_positions = [
        (-6, 2), (6, 2), (-2, 6), (2, -6), (-5, -5), (5, 5), (0, -7), (7, 0)
    ]
    for _ in range(800):
        tx, ty = random.choice(tree_positions)
        # 树冠：球形分布
        r_tree = random.uniform(0, 1.2)
        theta = random.uniform(0, 2 * math.pi)
        phi = math.acos(random.uniform(0, 1))  # 只取上半球
        x = tx + r_tree * math.sin(phi) * math.cos(theta)
        y = ty + r_tree * math.sin(phi) * math.sin(theta)
        z = 1.5 + r_tree * math.cos(phi)
        # 植被颜色：深绿
        r_c = random.uniform(0.05, 0.25)
        g_c = random.uniform(0.35, 0.65)
        b_c = random.uniform(0.05, 0.20)
        scale = random.uniform(0.06, 0.18)
        gaussians.append({
            'pos': (x, y, z),
            'color': (r_c, g_c, b_c),
            'opacity': random.uniform(0.6, 0.95),
            'scale': (scale, scale, scale),
            'rot': (1.0, 0.0, 0.0, 0.0),
        })

    # 天空/背景层：400 个，高空稀疏
    for _ in range(400):
        x = random.uniform(-10, 10)
        y = random.uniform(-10, 10)
        z = random.uniform(5, 10)
        r_c = random.uniform(0.6, 0.9)
        g_c = random.uniform(0.7, 0.95)
        b_c = random.uniform(0.85, 1.0)
        scale = random.uniform(0.1, 0.4)
        gaussians.append({
            'pos': (x, y, z),
            'color': (r_c, g_c, b_c),
            'opacity': random.uniform(0.1, 0.4),
            'scale': (scale, scale, scale * 0.3),
            'rot': (1.0, 0.0, 0.0, 0.0),
        })

    random.shuffle(gaussians)
    return gaussians


def write_metadata(filepath, tiny_path, lowair_path):
    meta = {
        "description": "Demo-05 教学用途 Gaussian 示例数据",
        "note": "本数据由脚本程序生成，不是真实 3DGS 训练结果，仅用于教学演示",
        "relation_to_photogrammetry_mesh": {
            "demo02_mesh": "Demo-02 加载的摄影测量 Mesh（OBJ/PLY 格式）",
            "demo05_gaussian": "Demo-05 加载的 Gaussian 点云（PLY 格式）",
            "semantic_relation": "两者表示同一低空场景的不同三维表达方式：Mesh 是显式几何，Gaussian 是隐式辐射场",
            "fusion_status": "prepared_for_demo06_alignment",
            "fusion_note": "Demo-05 不做 Mesh+3DGS 双源融合，融合功能在 Demo-06 中实现"
        },
        "future_alignment": {
            "status": "prepared_for_demo06_alignment",
            "transform": {
                "translation": [0.0, 0.0, 0.0],
                "rotation_deg": [0.0, 0.0, 0.0],
                "scale": 1.0
            },
            "reference_frame": "LOCAL_SCENE",
            "note": "Demo-06 将在此字段基础上实现 Gaussian 与 Mesh 的坐标对齐"
        },
        "files": {
            "tiny_colored_gaussians.ply": {
                "description": "200 个彩色 Gaussian，球形分布，用于验证 PLY 加载和颜色渲染",
                "gaussians": 200,
                "format": "Standard 3DGS PLY (ASCII)"
            },
            "lowair_demo_gaussian_cloud.ply": {
                "description": "3000 个低空场景风格 Gaussian，包含地面/建筑/植被/天空语义层",
                "gaussians": 3000,
                "format": "Standard 3DGS PLY (ASCII)"
            }
        }
    }
    with open(filepath, 'w', encoding='utf-8') as f:
        json.dump(meta, f, ensure_ascii=False, indent=2)
    print(f"[generate] Written metadata to {filepath}")


if __name__ == "__main__":
    print("[generate] Generating Demo-05 Gaussian sample data...")

    tiny = gen_tiny_colored_gaussians()
    tiny_path = os.path.join(OUTPUT_DIR, "tiny_colored_gaussians.ply")
    write_ply_ascii(tiny_path, tiny)

    lowair = gen_lowair_demo_gaussian_cloud()
    lowair_path = os.path.join(OUTPUT_DIR, "lowair_demo_gaussian_cloud.ply")
    write_ply_ascii(lowair_path, lowair)

    meta_path = os.path.join(OUTPUT_DIR, "metadata.json")
    write_metadata(meta_path, tiny_path, lowair_path)

    print(f"[generate] Done. Files written to: {OUTPUT_DIR}")
    print(f"  - tiny_colored_gaussians.ply: {len(tiny)} Gaussians")
    print(f"  - lowair_demo_gaussian_cloud.ply: {len(lowair)} Gaussians")
    print(f"  - metadata.json")
