cmake_minimum_required(VERSION 3.16)
project(UavSceneFusionQt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGLWidgets Network)

# -----------------------------------------------------------------------
# Demo-01 核心依赖：仅 Qt6，无需 Assimp 或其他外部三维库
# OBJ/PLY 模型加载（ObjModelRenderer + Assimp）将在 Demo-02 中实现
# -----------------------------------------------------------------------

set(SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    RenderWidget.cpp
    RenderWidget.h
    telemetry/UavState.h
    telemetry/UdpJsonReceiver.cpp
    telemetry/UdpJsonReceiver.h
    render/UavRenderer.cpp
    render/UavRenderer.h
    render/TrajectoryRenderer.cpp
    render/TrajectoryRenderer.h
    scene/SimpleScene.cpp
    scene/SimpleScene.h
    geo/CoordinateManager.cpp
    geo/CoordinateManager.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGLWidgets
    Qt6::Network
)
