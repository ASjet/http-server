cmake_minimum_required(VERSION 3.5)

project(server)


include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

add_library(
    ${PROJECT_NAME}
    src/uuid.cpp
    src/file.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    zlib
    cppsocket
    http
)

if(WIN32)
elseif(UNIX)
    include_directories(
        /usr/include/uuid
    )
    target_link_libraries(
        ${PROJECT_NAME}
        uuid
    )
elseif(APPLE)
    message(STATUS "Unsupported platform: Darwin.")
elseif(ANDROID)
    message(STATUS "Unsupported platform: Andriod.")
endif(WIN32)

add_executable(
    web
    src/main.cpp
)


target_link_libraries(
    web
    ${PROJECT_NAME}
)