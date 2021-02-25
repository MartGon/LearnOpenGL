# Set minimum CMake version
cmake_minimum_required(VERSION 3.1...3.16)

# Set project name
project(LearnOpenGLProj VERSION 1.0 LANGUAGES C CXX)

# Set C++ version
set (CMAKE_CXX_STANDARD 17)

set(DEPS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/deps)

add_subdirectory(deps)
add_subdirectory(src)
