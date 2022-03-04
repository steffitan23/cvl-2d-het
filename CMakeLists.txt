cmake_minimum_required(VERSION 3.1)
project(simul_em2d CXX)

option(BUILD_VIEWER "build visualizations if OpenGL is available" ON)
option(OPTION_OPTIMIZATION "build for this specific CPU" ON)


# default to Release build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug, Release or RelWithDebInfo" FORCE)
endif()

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL)
find_package(GLUT)
if(NOT OPENGL_FOUND OR NOT GLUT_FOUND)
    set(BUILD_VIEWER OFF)
endif()

find_package(Threads REQUIRED)

# use ccache if available for faster compile times
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Enabled ccache: ${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

# require c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


if(NOT MSVC)
    if(OPTION_OPTIMIZATION)
        set(OPTIMIZER_FLAGS "-march=native -flto")
    endif()

    set(COMPILER_WARNINGS "-Wall -Wpedantic -Wno-sign-compare -Wno-unused-variable -Wno-missing-braces -Wmissing-field-initializers")

    set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         ${COMPILER_WARNINGS}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OPTIMIZER_FLAGS}")

else() # Fix strange windows defaults
    set(CMAKE_WIN32_EXECUTABLE 0 CACHE INTERNAL "")
    set(COMPILER_WARNINGS "/wd4018 /wd4244 /wd4305 /wd4267")
    set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} ${COMPILER_WARNINGS} /utf-8")

    add_definitions(-DUNICODE -D_UNICODE)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(CLUSTER_FILES_HPP
    "em2d_cluster/Config.hpp"
    "em2d_cluster/DiffuReact.hpp"
    "em2d_cluster/Worker.hpp"
    "em2d_cluster/threading.hpp")
set(CLUSTER_FILES_CPP
    "em2d_cluster/Config.cpp"
    "em2d_cluster/Worker.cpp"
    "em2d_cluster/DiffuReact.cpp"
    "em2d_cluster/matr.cpp")

add_executable(em2d_cluster "em2d_cluster/main.cpp" ${CLUSTER_FILES_HPP} ${CLUSTER_FILES_CPP})
target_include_directories(em2d_cluster PUBLIC ./em2d_cluster)
target_link_libraries(em2d_cluster Threads::Threads)

if(BUILD_VIEWER)
    set(VIEWER_FILES
            "em2d_viewer/Base.cpp"
            "em2d_viewer/Display.cpp"
            "em2d_viewer/WorkVisual.cpp"
            "em2d_viewer/color.cpp"
            "em2d_viewer/main.cpp")
    add_executable(em2d_viewer ${VIEWER_FILES} ${CLUSTER_FILES_HPP} ${CLUSTER_FILES_CPP})
    target_include_directories(em2d_viewer PUBLIC ./em2d_cluster ./em2d_viewer ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})
    target_link_libraries(em2d_viewer Threads::Threads ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
endif()