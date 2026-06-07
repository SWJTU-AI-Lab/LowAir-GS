#!/bin/bash
# ==============================================================================
# LowAir-GS Demo-01 一键安装脚本
# 描述：自动安装系统依赖、从源码编译 Assimp 库、并编译 Qt6 项目
# 平台：Ubuntu 22.04 / 24.04
# ==============================================================================

set -e

# 颜色输出
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

log_info() { echo -e "${GREEN}[INFO] $1${NC}"; }
log_warn() { echo -e "${YELLOW}[WARN] $1${NC}"; }
log_error() { echo -e "${RED}[ERROR] $1${NC}"; }

# 1. 检查权限
if [ "$EUID" -ne 0 ]; then
  log_warn "建议使用 sudo 运行此脚本以安装系统依赖，或确保当前用户有 sudo 权限。"
fi

# 2. 安装系统依赖 (Qt6, CMake, 编译工具)
log_info "正在更新 apt 索引并安装基础依赖..."
sudo apt-get update -qq
sudo apt-get install -y cmake g++ make qt6-base-dev libgl1-mesa-dev wget unzip python3

# 3. 从源码编译并安装 Assimp (如果尚未安装)
if [ ! -f "/usr/local/lib/libassimp.so" ] && [ ! -f "/usr/lib/x86_64-linux-gnu/libassimp.so" ]; then
    log_info "未检测到 Assimp 库，开始从源码编译 Assimp 5.3.0..."
    
    # 创建临时构建目录
    BUILD_DIR=$(mktemp -d)
    cd "$BUILD_DIR"
    
    log_info "下载 Assimp 源码..."
    wget -q https://github.com/assimp/assimp/archive/refs/tags/v5.3.0.zip -O assimp.zip
    unzip -q assimp.zip
    cd assimp-5.3.0
    
    log_info "配置 CMake..."
    mkdir build && cd build
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DASSIMP_BUILD_TESTS=OFF \
        -DASSIMP_BUILD_SAMPLES=OFF \
        -DASSIMP_BUILD_ASSIMP_TOOLS=OFF \
        -DASSIMP_NO_EXPORT=ON \
        -DBUILD_SHARED_LIBS=ON \
        -DASSIMP_WARNINGS_AS_ERRORS=OFF
        
    log_info "编译 Assimp (这可能需要几分钟)..."
    make -j$(nproc)
    
    log_info "安装 Assimp 到系统路径..."
    sudo make install
    sudo ldconfig
    
    log_info "Assimp 编译安装完成！"
else
    log_info "检测到已安装 Assimp，跳过编译。"
fi

# 4. 编译 Qt6 应用程序
log_info "开始编译 LowAir-GS Demo-01 Qt 应用程序..."
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ ! -d "$PROJECT_DIR" ]; then
    log_error "找不到项目目录: $PROJECT_DIR"
    exit 1
fi

cd "$PROJECT_DIR"
rm -rf build
mkdir build && cd build

log_info "配置 Qt 项目..."
cmake .. -DCMAKE_BUILD_TYPE=Release

log_info "编译 Qt 项目..."
make -j$(nproc)

log_info "====================================================================="
log_info "安装与编译全部完成！"
log_info "执行文件路径: $PROJECT_DIR/build/Demo01UavSceneFusion"
log_info "====================================================================="
log_info "运行指南："
log_info "1. 启动地面站主程序: $PROJECT_DIR/build/Demo01UavSceneFusion"
log_info "2. 在程序界面点击左侧的 'Start UDP (14580)'"
log_info "3. 在另一个终端启动模拟器: python3 ../../tools/telemetry_simulator/send_uav_udp.py"
log_info "====================================================================="
