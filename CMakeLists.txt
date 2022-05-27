cmake_minimum_required(VERSION 3.5)

project(webserver)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/zlib
)

add_subdirectory(
    zlib
)

add_subdirectory(
    cppsocket
)

add_subdirectory(
    http
)

add_subdirectory(
    server
)


SET(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(
    -Wall
    -g
    -O0
    -fsanitize=address
)