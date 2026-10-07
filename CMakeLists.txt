cmake_minimum_required(VERSION 3.10)
project(Tricycle VERSION 0.1.0 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_compile_definitions(TC_DEBUG_OUTPUT TC_USE_SIMD)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/portaudio)

#tc_graphics
add_library(
    tc_graphics STATIC
    src/misc/misc.cpp
    src/graphics/buffer.cpp
    src/graphics/graphics.cpp
    src/graphics/layer.cpp
    src/graphics/rectangle_packer.cpp
    src/graphics/widgets.cpp
    lib/glad/src/glad.c)

target_include_directories(
    tc_graphics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(tc_graphics glfw)

#tc_audio
add_library(
    tc_audio STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/audio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/audio_file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/stream.cpp)

target_include_directories(
    tc_audio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/portaudio/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(tc_audio portaudio)

#tests
add_executable(test_ball tests/ball.cpp)
target_link_libraries(test_ball tc_graphics tc_audio)

add_executable(test_text tests/text.cpp)
target_link_libraries(test_text tc_graphics)

add_executable(test_sierpinski tests/sierpinski.cpp)
target_link_libraries(test_sierpinski tc_graphics)

add_executable(test_calculator tests/calculator.cpp)
target_link_libraries(test_calculator tc_graphics)
