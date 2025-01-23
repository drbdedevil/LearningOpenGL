# general info
cmake_minimum_required(VERSION 3.30)
set(PROJECT_NAME MaxOpenGL)
project(${PROJECT_NAME} VERSION 1.0.0)

# common options
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_SUPPRESS_REGENERATION TRUE)

# cpp options
set(CMAKE_CXX_STANDARD 20)

# sources
set(SOURCES source/LearningOpenGL/main.cpp)

set(EXPANSION_SOURCES source/ExpansionGame/main.cpp)

# My libs
add_library(ExpansionLib STATIC source/ExpansionGame/Game/ExpansionGame.cpp source/ExpansionGame/Game/ExpansionGame.h)
add_library(EngineLib STATIC source/Engine/CameraManager.cpp source/Engine/CameraManager.h)

# find libs
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(glew REQUIRED)
find_package(VulkanLoader REQUIRED)

# targets
add_executable(${PROJECT_NAME} ${SOURCES})
add_executable(${PROJECT_NAME}ExpansionGame ${EXPANSION_SOURCES})

# targets
target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL glm::glm GLEW::GLEW Vulkan::Loader)
target_link_libraries(${PROJECT_NAME}ExpansionGame PRIVATE ExpansionLib)
# target_link_libraries(${PROJECT_NAME}ExpansionGame PUBLIC glfw OpenGL::GL glm::glm GLEW::GLEW)
target_link_libraries(ExpansionLib PUBLIC EngineLib glfw OpenGL::GL glm::glm GLEW::GLEW Vulkan::Loader)

target_link_libraries(EngineLib PRIVATE glfw OpenGL::GL glm::glm GLEW::GLEW Vulkan::Loader)
target_link_libraries(ExpansionLib PUBLIC EngineLib)
target_link_libraries(${PROJECT_NAME}ExpansionGame PRIVATE ExpansionLib)

# inlcudes
set(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}ExpansionGame ExpansionLib)
foreach(target IN LISTS TARGETS)
    target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source)
endforeach()

# if (WIN32)
    # target_link_options(${PROJECT_NAME} PRIVATE /SUBSYSTEM:WINDOWS)
# endif()