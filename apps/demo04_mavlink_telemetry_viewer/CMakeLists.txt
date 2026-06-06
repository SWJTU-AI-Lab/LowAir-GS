cmake_minimum_required(VERSION 3.16)
project(Demo04MavlinkTelemetryViewer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Qt6 依赖
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGLWidgets Network)
find_package(OpenGL REQUIRED)

# MAVSDK 为可选依赖（优先路径）
find_package(MAVSDK QUIET)
if(MAVSDK_FOUND)
    message(STATUS "[Demo-04] MAVSDK found - MavsdkTelemetrySource enabled")
    add_compile_definitions(HAVE_MAVSDK)
else()
    message(STATUS "[Demo-04] MAVSDK not found - using MAVLink Raw fallback (MavlinkRawTelemetrySource)")
endif()

set(SOURCES
    main.cpp
    MainWindow.cpp
    RenderWidget.cpp
    geo/Wgs84Converter.cpp
    geo/CoordinateTransformer.cpp
    telemetry/MavlinkRawTelemetrySource.cpp
    telemetry/TelemetryRecorder.cpp
    render/TelemetrySceneRenderer.cpp
    render/UavMarkerRenderer.cpp
    render/TrajectoryRenderer.cpp
    camera/OrbitCamera.cpp
)

set(HEADERS
    MainWindow.h
    RenderWidget.h
    geo/GeoCoordinate.h
    geo/Wgs84Converter.h
    geo/CoordinateTransformer.h
    telemetry/DroneTelemetryState.h
    telemetry/TelemetrySource.h
    telemetry/MavlinkRawTelemetrySource.h
    telemetry/TelemetryRecorder.h
    render/TelemetrySceneRenderer.h
    render/UavMarkerRenderer.h
    render/TrajectoryRenderer.h
    camera/OrbitCamera.h
)

if(MAVSDK_FOUND)
    list(APPEND SOURCES telemetry/MavsdkTelemetrySource.cpp)
    list(APPEND HEADERS telemetry/MavsdkTelemetrySource.h)
endif()

add_executable(Demo04MavlinkTelemetryViewer ${SOURCES} ${HEADERS})

target_include_directories(Demo04MavlinkTelemetryViewer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/geo
    ${CMAKE_CURRENT_SOURCE_DIR}/telemetry
    ${CMAKE_CURRENT_SOURCE_DIR}/render
    ${CMAKE_CURRENT_SOURCE_DIR}/camera
    ${CMAKE_CURRENT_SOURCE_DIR}/config
)

target_link_libraries(Demo04MavlinkTelemetryViewer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGLWidgets
    Qt6::Network
    OpenGL::GL
)

if(MAVSDK_FOUND)
    target_link_libraries(Demo04MavlinkTelemetryViewer PRIVATE MAVSDK::mavsdk)
endif()

# 复制配置文件到构建目录
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config/demo04_mavlink_config.json
    ${CMAKE_CURRENT_BINARY_DIR}/demo04_mavlink_config.json
    COPYONLY
)
