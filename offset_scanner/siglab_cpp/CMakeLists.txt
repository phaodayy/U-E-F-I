cmake_minimum_required(VERSION 3.20)
project(siglab_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(siglab
    src/main.cpp
    src/pe_image.cpp
    src/signature.cpp
)

target_include_directories(siglab PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../PUBG-2"
)

if(MSVC)
    target_compile_options(siglab PRIVATE /W4 /permissive-)
else()
    target_compile_options(siglab PRIVATE -Wall -Wextra -Wpedantic)
endif()
