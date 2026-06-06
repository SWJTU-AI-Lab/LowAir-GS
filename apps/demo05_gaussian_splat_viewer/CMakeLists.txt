cmake_minimum_required(VERSION 3.20)
project(Demo05GaussianSplatViewer VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)
find_package(OpenGL REQUIRED)

set(SOURCES
    main.cpp
    MainWindow.cpp
    RenderWidget.cpp
    gaussian/GaussianCloud.cpp
    gaussian/GaussianPlyLoader.cpp
    gaussian/GaussianSplatLoader.cpp
    render/GaussianPointRenderer.cpp
    render/GaussianBillboardRenderer.cpp
    render/GridRenderer.cpp
    render/AxisRenderer.cpp
    render/BoundingBoxRenderer.cpp
    camera/OrbitCamera.cpp
    perf/FpsCounter.cpp
)

set(HEADERS
    MainWindow.h
    RenderWidget.h
    gaussian/GaussianPoint.h
    gaussian/GaussianCloud.h
    gaussian/GaussianPlyLoader.h
    gaussian/GaussianSplatLoader.h
    render/GaussianPointRenderer.h
    render/GaussianBillboardRenderer.h
    render/GridRenderer.h
    render/AxisRenderer.h
    render/BoundingBoxRenderer.h
    camera/OrbitCamera.h
    perf/FpsCounter.h
)

add_executable(Demo05GaussianSplatViewer ${SOURCES} ${HEADERS})

target_link_libraries(Demo05GaussianSplatViewer
    PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    OpenGL::GL
)

target_include_directories(Demo05GaussianSplatViewer
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
