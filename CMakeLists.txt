cmake_minimum_required(VERSION 3.29.2)

project(threed)

message(STATUS, "CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS, "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

set(CMAKE_PREFIX_PATH tpl/SDL)
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
message(STATUS, "SDL3_VERSION=${SDL3_VERSION}")

set(CMAKE_PREFIX_PATH tpl/cglm)
find_package(cglm REQUIRED)
message(STATUS "cglm_FOUND=${cglm_FOUND}")


add_compile_definitions(CGLM_FORCE_DEPTH_ZERO_TO_ONE)
add_compile_definitions(CGLM_FORCE_LEFT_HANDED)


if(LINUX)
    add_compile_options($<$<CONFIG:DEBUG>:-ggdb>)
    add_compile_options($<$<CONFIG:DEBUG>:-O0>)
    add_compile_options(-pedantic-errors)
    add_compile_options(-pedantic)
    add_compile_options(-Wfatal-errors)
    add_compile_options(-Wall)
    add_compile_options(-Wextra)
    add_compile_options(-Wpedantic)
    add_compile_options(-fvisibility=hidden)
endif()

if(MSVC)
    add_compile_options(-W4)
endif()


set(PROJECT_SOURCES
    src/main.c
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE tpl/
    PRIVATE tpl/cglm/include
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG      ../bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE    ../bin
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE SDL3::SDL3
)

