cmake_minimum_required(VERSION 3.25.1)

project(threed)

message(STATUS, "CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS, "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

set(CMAKE_PREFIX_PATH tpl/SDL)
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
message(STATUS, "SDL3_VERSION=${SDL3_VERSION}")

set(CMAKE_PREFIX_PATH tpl/cglm)
find_package(cglm REQUIRED)
message(STATUS "cglm_FOUND=${cglm_FOUND}")

find_package(Vulkan REQUIRED)
message(STATUS "Vulkan_FOUND=${Vulkan_FOUND}")


add_compile_definitions(ENABLE_DEBUG_LOG)
add_compile_definitions(ENABLE_INFO_LOG)
add_compile_definitions(ENABLE_ERROR_LOG)
add_compile_definitions(ENABLE_LOG_FILE)
add_compile_definitions(ENABLE_CHECK)
add_compile_definitions(ENABLE_TIMED_BLOCK)


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
    #show lots of warning need firther env add_compile_options(-Wall)
    # -GF if needed to enable string pooling, otherwise debug counter recording will not work.
    add_compile_options(-GF)
endif()


set(PROJECT_SOURCES
    src/main.c
    src/app.c
    src/app.h
    src/check.c
    src/check.h
    src/types.h
    src/defines.h
    src/log.c
    src/log.h
    src/debug.c
    src/debug.h
    src/vulkan.c
    src/vulkan.h
    src/math.c
    src/math.h
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
    PRIVATE Vulkan::Vulkan
)

if(LINUX)
target_link_libraries(${PROJECT_NAME}
    PRIVATE m
)
endif()
