#!/usr/bin/env python3
"""
send_wgs84_uav_udp.py
WGS84 无人机遥测模拟器

功能：
- 在 ENU 空间中生成椭圆轨迹，反算为 WGS84 经纬高
- 以 UDP JSON 格式发送到指定地址和端口
- 支持命令行参数配置

注意：
- 本模拟器使用近似公式将 ENU 反算为 WGS84，适用于教学演示和局部场景（半径 < 500m）
- 真实无人机接入在 Demo-04 中实现

用法：
  python3 send_wgs84_uav_udp.py
  python3 send_wgs84_uav_udp.py --origin_lat 30.7 --origin_lon 104.0 --origin_alt 500.0 --rate 20
"""

import socket
import json
import time
import math
import argparse

# WGS84 椭球参数
WGS84_A = 6378137.0          # 长半轴 (m)
WGS84_F = 1.0 / 298.257223563
WGS84_B = WGS84_A * (1.0 - WGS84_F)
WGS84_E2 = 2.0 * WGS84_F - WGS84_F * WGS84_F


def enu_to_wgs84_approx(e, n, u, origin_lat, origin_lon, origin_alt):
    """
    ENU -> WGS84 近似反算
    适用于局部场景（半径 < 500m），教学演示用途
    lat = origin_lat + N / (a * (1-e²) / (1-e²sin²φ₀)^1.5) * 180/π
    lon = origin_lon + E / (N(φ₀) * cos(φ₀)) * 180/π
    alt = origin_alt + U
    """
    lat0_rad = math.radians(origin_lat)
    sin_lat0 = math.sin(lat0_rad)

    # 卯酉圈曲率半径
    N0 = WGS84_A / math.sqrt(1.0 - WGS84_E2 * sin_lat0 * sin_lat0)
    # 子午圈曲率半径
    M0 = WGS84_A * (1.0 - WGS84_E2) / ((1.0 - WGS84_E2 * sin_lat0 * sin_lat0) ** 1.5)

    lat = origin_lat + math.degrees(n / M0)
    lon = origin_lon + math.degrees(e / (N0 * math.cos(lat0_rad)))
    alt = origin_alt + u

    return lat, lon, alt


def generate_ellipse_trajectory(t, origin_lat, origin_lon, origin_alt):
    """
    在 ENU 空间中生成椭圆轨迹，反算为 WGS84
    E = 60 * cos(t)
    N = 40 * sin(t)
    U = 25 + 5 * sin(0.5t)
    """
    e = 60.0 * math.cos(t)
    n = 40.0 * math.sin(t)
    u = 25.0 + 5.0 * math.sin(0.5 * t)

    # yaw：运动方向（切线方向）
    de_dt = -60.0 * math.sin(t)
    dn_dt =  40.0 * math.cos(t)
    yaw = math.degrees(math.atan2(de_dt, dn_dt))  # 从北方向顺时针
    if yaw < 0:
        yaw += 360.0

    lat, lon, alt = enu_to_wgs84_approx(e, n, u, origin_lat, origin_lon, origin_alt)
    return lat, lon, alt, yaw, e, n, u


def main():
    parser = argparse.ArgumentParser(description="WGS84 UAV UDP Simulator for Demo-03")
    parser.add_argument("--host",        default="127.0.0.1",  help="Target host (default: 127.0.0.1)")
    parser.add_argument("--port",        type=int, default=14581, help="Target UDP port (default: 14581)")
    parser.add_argument("--uav_id",      default="geo_sim_01", help="UAV ID")
    parser.add_argument("--origin_lat",  type=float, default=30.700000, help="Origin latitude (degree)")
    parser.add_argument("--origin_lon",  type=float, default=104.000000, help="Origin longitude (degree)")
    parser.add_argument("--origin_alt",  type=float, default=500.0, help="Origin altitude (meter)")
    parser.add_argument("--rate",        type=float, default=10.0, help="Send rate in Hz (default: 10)")
    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    interval = 1.0 / args.rate
    t = 0.0
    dt = interval * 0.5  # 轨迹参数步长（控制速度）

    print(f"[WGS84 Simulator] Sending to {args.host}:{args.port} at {args.rate}Hz")
    print(f"[WGS84 Simulator] Origin: lat={args.origin_lat}, lon={args.origin_lon}, alt={args.origin_alt}")
    print(f"[WGS84 Simulator] UAV ID: {args.uav_id}")
    print(f"[WGS84 Simulator] Trajectory: ENU ellipse E=60cos(t), N=40sin(t), U=25+5sin(0.5t)")
    print("[WGS84 Simulator] Press Ctrl+C to stop\n")

    seq = 0
    try:
        while True:
            lat, lon, alt, yaw, e, n, u = generate_ellipse_trajectory(
                t, args.origin_lat, args.origin_lon, args.origin_alt
            )

            timestamp_ms = int(time.time() * 1000)
            battery = max(20.0, 100.0 - seq * 0.01)  # 缓慢消耗电量

            payload = {
                "uav_id":       args.uav_id,
                "timestamp_ms": timestamp_ms,
                "frame":        "WGS84",
                "lat":          round(lat, 8),
                "lon":          round(lon, 8),
                "alt":          round(alt, 2),
                "roll":         round(math.sin(t * 0.3) * 3.0, 2),
                "pitch":        round(math.cos(t * 0.2) * 2.0, 2),
                "yaw":          round(yaw, 1),
                "battery":      round(battery, 1)
            }

            data = json.dumps(payload).encode("utf-8")
            sock.sendto(data, (args.host, args.port))

            if seq % 20 == 0:
                print(f"[seq={seq:5d}] lat={lat:.6f} lon={lon:.6f} alt={alt:.1f}m "
                      f"| ENU=({e:.1f},{n:.1f},{u:.1f})m | yaw={yaw:.1f}°")

            t += dt
            seq += 1
            time.sleep(interval)

    except KeyboardInterrupt:
        print(f"\n[WGS84 Simulator] Stopped after {seq} packets")
    finally:
        sock.close()


if __name__ == "__main__":
    main()
