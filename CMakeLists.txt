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

# find libs
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(glew REQUIRED)

# targets
add_executable(${PROJECT_NAME} ${SOURCES})

# targets
target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL glm::glm GLEW::GLEW)

# if (WIN32)
    # target_link_options(${PROJECT_NAME} PRIVATE /SUBSYSTEM:WINDOWS)
# endif()