cmake_minimum_required(VERSION 3.16)
project(FractalEngine VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build tests" OFF)

add_library(engine STATIC
    src/Engine.cpp
    src/engine/core/FractalKernel.cpp
    src/engine/FURCMD/FURCMD.cpp
    src/engine/services/workScheduler/WSDomains.cpp
    src/engine/services/workScheduler/workScheduler.cpp
    src/engine/services/ECS/Entity/entity.cpp
    src/engine/services/ECS/Component/ComponentManager.cpp
    src/engine/services/ECS/Component/CMDomains.cpp
    src/engine/services/Event/EventBus.cpp
    src/engine/services/Event/EBDomains.cpp
    src/engine/services/ModuleLoader/ModuleLoader.cpp
    src/engine/services/ContainerLoader/ContainerLoader.cpp
    src/engine/services/SmartScheduler/SSDomains.cpp
    src/engine/services/SystemExecution/SEDomains.cpp
    src/engine/services/Clock/Clock.cpp
    src/engine/services/DataBase/SQLDomains.cpp
    src/engine/services/DataBase/SQLite.cpp
    src/engine/services/vfs/VFSDomains.cpp
    src/engine/services/vfs/VFS.cpp
    include/sqlite/sqlite3.c
)

target_include_directories(engine
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${PROJECT_SOURCE_DIR}/include/sqlite
    PUBLIC ${PROJECT_SOURCE_DIR}/src/engine
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)


set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(engine PRIVATE /W4 /permissive-)
else()
    target_compile_options(engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(fractal_app src/main.cpp)
target_link_libraries(fractal_app PRIVATE engine)

# Toolchain: Fracty
add_executable(fracty src/tools/fracty/main.cpp)
target_include_directories(fracty PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(fracty PRIVATE engine)

if(UNIX AND NOT APPLE)
    target_link_options(fractal_app PRIVATE -rdynamic)
    
    target_link_libraries(fractal_app PRIVATE dl)
endif()

add_executable(test_ecs src/tests/test_ecs.cpp)
target_link_libraries(test_ecs PRIVATE engine)

install(TARGETS fractal_app engine)
