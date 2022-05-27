cmake_minimum_required(VERSION 3.5)

project(http)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

add_library(
    ${PROJECT_NAME}
    src/http.cpp
    src/gzip.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    cppsocket
    zlib
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
