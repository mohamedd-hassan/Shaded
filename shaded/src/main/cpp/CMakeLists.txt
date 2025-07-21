cmake_minimum_required(VERSION 3.4.1)

project(blur_renderer)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-z,max-page-size=16384")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-z,max-page-size=16384")


add_library(
        blur_renderer
        SHARED
        blur_renderer.cpp
        egl_helper.cpp
        egl_helper.h
        unbounded_blur.cpp
        unbounded_blur.h
        native-lib.cpp
)

find_library(log-lib log)
find_library(glesv2-lib GLESv2)
find_library(jnigraphics-lib jnigraphics)
find_library(android-lib android)
find_library(EGL_LIB EGL)

target_link_libraries(
        blur_renderer
        ${log-lib}
        ${EGL_LIB}
        ${glesv2-lib}
        ${jnigraphics-lib}
        ${android-lib}
)
