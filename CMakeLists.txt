cmake_minimum_required(VERSION 3.20)
project(SDL3ECS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL3 (must be installed on the system, or provided via vcpkg/FetchContent).
find_package(SDL3 REQUIRED CONFIG)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/ECS/Private/EntityManager.cpp
    src/ECS/Private/ComponentManager.cpp


)

# ECS/Public holds headers included from anywhere in the project.
target_include_directories(${PROJECT_NAME} PRIVATE
    src
    src/ECS/Public
)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# Optional: warnings on, since this is a learning/skeleton project.
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()
