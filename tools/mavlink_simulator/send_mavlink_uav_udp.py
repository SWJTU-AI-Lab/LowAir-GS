#!/usr/bin/env python3
"""
Demo-04 MAVLink Raw UDP 模拟器
发送 MAVLink v1 原始消息（HEARTBEAT + GLOBAL_POSITION_INT + ATTITUDE + SYS_STATUS）
模拟 PX4 SITL 无人机在成都双流机场附近飞行椭圆轨迹

用法：
    python3 send_mavlink_uav_udp.py [--host 127.0.0.1] [--port 14550] [--hz 10]
"""

import socket
import struct
import math
import time
import argparse

# MAVLink v1 常量
MAVLINK_STX    = 0xFE
SYS_ID         = 1
COMP_ID        = 1

# 消息 ID
MSG_HEARTBEAT           = 0
MSG_SYS_STATUS          = 1
MSG_ATTITUDE            = 30
MSG_GLOBAL_POSITION_INT = 33

# 地理原点（成都双流机场附近）
ORIGIN_LAT = 30.700000   # 度
ORIGIN_LON = 104.000000  # 度
ORIGIN_ALT = 500.0       # 米（绝对高度）

# 椭圆轨迹参数
ELLIPSE_A   = 0.0010   # 经度半轴（约 100m）
ELLIPSE_B   = 0.0006   # 纬度半轴（约 67m）
CLIMB_AMP   = 20.0     # 高度振幅（米）
ORBIT_SPEED = 0.3      # 角速度（rad/s）


def pack_mavlink_v1(msg_id: int, payload: bytes, seq: int) -> bytes:
    """打包 MAVLink v1 帧（不含 CRC，演示用）"""
    length = len(payload)
    header = bytes([MAVLINK_STX, length, seq & 0xFF, SYS_ID, COMP_ID, msg_id])
    # 简化 CRC（X25 CRC 占位）
    frame = header + payload + bytes([0x00, 0x00])
    return frame


def pack_heartbeat(seq: int, custom_mode: int = 0x00030000) -> bytes:
    """HEARTBEAT (#0): type=2(quad), autopilot=12(PX4), base_mode=0x89(armed+guided), status=4"""
    armed_flag = 0x89  # MAV_MODE_FLAG_SAFETY_ARMED | MAV_MODE_FLAG_GUIDED_ENABLED | MAV_MODE_FLAG_STABILIZE_ENABLED
    payload = struct.pack('<IBBBBB', custom_mode, 2, 12, armed_flag, 4, 3)
    return pack_mavlink_v1(MSG_HEARTBEAT, payload, seq)


def pack_global_position_int(seq: int, lat: float, lon: float, alt: float,
                               rel_alt: float, vx: float, vy: float, vz: float,
                               hdg: float, t_ms: int) -> bytes:
    """GLOBAL_POSITION_INT (#33)"""
    lat_i32  = int(lat  * 1e7)
    lon_i32  = int(lon  * 1e7)
    alt_i32  = int(alt  * 1e3)
    ralt_i32 = int(rel_alt * 1e3)
    vx_i16   = int(vx * 100)
    vy_i16   = int(vy * 100)
    vz_i16   = int(vz * 100)
    hdg_u16  = int(hdg * 100) % 36000
    payload  = struct.pack('<IiiiihhhH', t_ms, lat_i32, lon_i32, alt_i32, ralt_i32,
                           vx_i16, vy_i16, vz_i16, hdg_u16)
    return pack_mavlink_v1(MSG_GLOBAL_POSITION_INT, payload, seq)


def pack_attitude(seq: int, roll: float, pitch: float, yaw: float, t_ms: int) -> bytes:
    """ATTITUDE (#30): 角度单位 rad"""
    payload = struct.pack('<Iffffff', t_ms, roll, pitch, yaw, 0.0, 0.0, 0.0)
    return pack_mavlink_v1(MSG_ATTITUDE, payload, seq)


def pack_sys_status(seq: int, battery_pct: int) -> bytes:
    """SYS_STATUS (#1): 简化版，battery_remaining 在 offset 30"""
    # 31 字节 payload，battery_remaining 在最后
    payload = bytearray(31)
    struct.pack_into('<HH', payload, 14, 12600, 0)  # 12.6V, 0 current
    payload[30] = battery_pct & 0xFF
    return pack_mavlink_v1(MSG_SYS_STATUS, bytes(payload), seq)


def main():
    parser = argparse.ArgumentParser(description='Demo-04 MAVLink Raw UDP Simulator')
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('--port', type=int, default=14550)
    parser.add_argument('--hz',   type=float, default=10.0)
    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    interval = 1.0 / args.hz
    seq = 0
    t0  = time.time()

    print(f"[MAVLink Sim] Sending to {args.host}:{args.port} @ {args.hz} Hz")
    print(f"[MAVLink Sim] Origin: {ORIGIN_LAT}°N, {ORIGIN_LON}°E, alt={ORIGIN_ALT}m")
    print("[MAVLink Sim] Press Ctrl+C to stop")

    try:
        while True:
            t = time.time() - t0
            angle = t * ORBIT_SPEED

            # 椭圆轨迹（WGS84）
            lat = ORIGIN_LAT + ELLIPSE_B * math.sin(angle)
            lon = ORIGIN_LON + ELLIPSE_A * math.cos(angle)
            alt = ORIGIN_ALT + CLIMB_AMP * math.sin(angle * 0.5) + 30.0  # 相对高度 30m 基准

            # 速度（ENU 近似）
            vn = ELLIPSE_B * 111320.0 * ORBIT_SPEED * math.cos(angle)
            ve = ELLIPSE_A * 111320.0 * ORBIT_SPEED * (-math.sin(angle))

            # 航向角（yaw，从北顺时针，单位：度）
            yaw_deg = math.degrees(math.atan2(ve, vn)) % 360.0
            yaw_rad = math.radians(yaw_deg)

            # 姿态（模拟倾斜）
            roll  = math.radians(5.0 * math.sin(angle * 2))
            pitch = math.radians(3.0 * math.cos(angle))

            # 电量（缓慢下降）
            battery_pct = max(20, int(95 - t * 0.1))

            t_ms = int(t * 1000)
            rel_alt = alt - ORIGIN_ALT

            # 发送 HEARTBEAT（每 1 秒一次）
            if seq % max(1, int(args.hz)) == 0:
                sock.sendto(pack_heartbeat(seq), (args.host, args.port))

            # 发送 GLOBAL_POSITION_INT
            sock.sendto(pack_global_position_int(seq, lat, lon, alt, rel_alt,
                                                  vn, ve, 0.0, yaw_deg, t_ms),
                        (args.host, args.port))

            # 发送 ATTITUDE
            sock.sendto(pack_attitude(seq, roll, pitch, yaw_rad, t_ms),
                        (args.host, args.port))

            # 发送 SYS_STATUS（每 2 秒一次）
            if seq % max(1, int(args.hz * 2)) == 0:
                sock.sendto(pack_sys_status(seq, battery_pct), (args.host, args.port))

            seq = (seq + 1) & 0xFF

            if seq % 50 == 0:
                print(f"[t={t:.1f}s] lat={lat:.6f} lon={lon:.6f} alt={alt:.1f}m "
                      f"yaw={yaw_deg:.1f}° bat={battery_pct}%")

            time.sleep(interval)

    except KeyboardInterrupt:
        print("\n[MAVLink Sim] Stopped")
    finally:
        sock.close()


if __name__ == '__main__':
    main()
