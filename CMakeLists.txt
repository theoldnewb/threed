cmake_minimum_required(VERSION 3.29.2)

project(threed)

message(STATUS, "CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS, "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

set(CMAKE_PREFIX_PATH tpl/SDL)
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
message(STATUS, "SDL3_VERSION=${SDL3_VERSION}")

set(PROJECT_SOURCES
    src/main.c
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE tpl/
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG      ../bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE    ../bin
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE SDL3::SDL3
)

