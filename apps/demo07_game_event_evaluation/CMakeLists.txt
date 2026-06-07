cmake_minimum_required(VERSION 3.16)
project(Demo07GameEventEvaluation VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets Network)
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)

set(SOURCES
    main.cpp
    MainWindow.cpp
    RenderWidget.cpp

    scene/SceneAsset.cpp
    scene/SceneLoader.cpp

    mission/MissionConfig.cpp
    mission/MissionLoader.cpp
    mission/MissionState.cpp
    mission/VirtualTarget.cpp
    mission/TriggerZone.cpp

    event/EventEngine.cpp
    event/EventTimeline.cpp
    event/EventRecord.cpp

    scoring/ScoreRule.cpp
    scoring/ScoreEvaluator.cpp
    scoring/MissionReport.cpp

    replay/TrajectoryReplay.cpp
    replay/ReplayFrame.cpp

    effects/Effect.cpp
    effects/ParticleEffectRenderer.cpp
    effects/GlowEffectRenderer.cpp
    effects/LabelPopup.cpp
    effects/VirtualExplosionRenderer.cpp

    render/TargetRenderer.cpp
    render/ZoneRenderer.cpp
    render/TrajectoryRenderer.cpp
    render/HudRenderer.cpp
    render/UavMarkerRenderer.cpp
    render/GridRenderer.cpp
    render/AxisRenderer.cpp

    camera/OrbitCamera.cpp
    perf/FpsCounter.cpp
)

add_executable(Demo07GameEventEvaluation ${SOURCES})

target_include_directories(Demo07GameEventEvaluation PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(Demo07GameEventEvaluation PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Network
    OpenGL::GL
    assimp
)

target_compile_options(Demo07GameEventEvaluation PRIVATE -O2)
