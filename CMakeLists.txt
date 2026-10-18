cmake_minimum_required(VERSION 3.10)
project(Tricycle VERSION 0.1.0 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(EXECUTABLE_OUTPUT_PATH bin)

add_compile_definitions(TC_DEBUG_OUTPUT TC_USE_SIMD) #TCT_CATCH_ON_FAIL

add_subdirectory(lib/glfw)
add_subdirectory(lib/portaudio)

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
    lib/glfw/include
    lib/glad/include
    lib/stb
    include)

target_link_libraries(tc_graphics glfw)

#tc_audio
add_library(
    tc_audio STATIC
    src/audio/audio.cpp
    src/audio/audio_file.cpp
    src/audio/stream.cpp
    lib/stb/stb_vorbis.c)

target_include_directories(
    tc_audio PUBLIC
    lib/portaudio/include
    lib/stb
    include)

target_link_libraries(tc_audio portaudio)

#tests
add_executable(test_runner tests/test.cpp tests/math.cpp)
target_link_libraries(test_runner tc_graphics tc_audio)

#examples
add_executable(ex_ball examples/ball.cpp)
target_link_libraries(ex_ball tc_graphics tc_audio)

add_executable(ex_text examples/text.cpp)
target_link_libraries(ex_text tc_graphics)

add_executable(ex_sierpinski examples/sierpinski.cpp)
target_link_libraries(ex_sierpinski tc_graphics)

add_executable(ex_calculator examples/calculator.cpp)
target_link_libraries(ex_calculator tc_graphics)
