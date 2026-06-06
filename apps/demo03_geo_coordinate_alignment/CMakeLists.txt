cmake_minimum_required(VERSION 3.16)
project(Demo03GeoCoordinateAlignment VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 required components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGLWidgets Network)

# OpenGL
find_package(OpenGL REQUIRED)

# Optional: GeographicLib for high-precision WGS84 conversion
# If not found, falls back to built-in WGS84 ellipsoid formulas
find_package(GeographicLib QUIET)
if(GeographicLib_FOUND)
    message(STATUS "GeographicLib found: using high-precision coordinate conversion")
    add_compile_definitions(USE_GEOGRAPHICLIB)
else()
    message(STATUS "GeographicLib not found: using built-in WGS84 ellipsoid formulas (suitable for local scene demo)")
endif()

# Source files
set(SOURCES
    main.cpp
    MainWindow.cpp
    RenderWidget.cpp
    geo/CoordinateTransformer.cpp
    geo/Wgs84Converter.cpp
    telemetry/UdpWgs84Receiver.cpp
    render/GeoSceneRenderer.cpp
    render/UavMarkerRenderer.cpp
    render/TrajectoryRenderer.cpp
    camera/OrbitCamera.cpp
)

set(HEADERS
    MainWindow.h
    RenderWidget.h
    geo/GeoCoordinate.h
    geo/CoordinateTransformer.h
    geo/Wgs84Converter.h
    telemetry/GeoUavState.h
    telemetry/UdpWgs84Receiver.h
    render/GeoSceneRenderer.h
    render/UavMarkerRenderer.h
    render/TrajectoryRenderer.h
    camera/OrbitCamera.h
)

add_executable(Demo03GeoCoordinateAlignment
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(Demo03GeoCoordinateAlignment PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/geo
    ${CMAKE_CURRENT_SOURCE_DIR}/telemetry
    ${CMAKE_CURRENT_SOURCE_DIR}/render
    ${CMAKE_CURRENT_SOURCE_DIR}/camera
    ${CMAKE_CURRENT_SOURCE_DIR}/config
)

target_link_libraries(Demo03GeoCoordinateAlignment PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGLWidgets
    Qt6::Network
    OpenGL::GL
)

if(GeographicLib_FOUND)
    target_link_libraries(Demo03GeoCoordinateAlignment PRIVATE
        GeographicLib::GeographicLib
    )
endif()

# Copy config file to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config/demo03_geo_config.json
    ${CMAKE_CURRENT_BINARY_DIR}/demo03_geo_config.json
    COPYONLY
)
