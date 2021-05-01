SET(SOURCES

    src/LearnOpenGL.cpp
)
add_executable(LearnOpenGL ${SOURCES})
target_link_libraries(LearnOpenGL GLAD)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

if(WIN32)
    SET(GLFW_LIB_PATH "" CACHE FILEPATH "Path to glfw.lib")
    SET(GLFW_INCLUDE_DIR "" CACHE PATH "Path to glfw/include")
    SET(ASSIMP_LIB_PATH "" CACHE FILEPATH "Path to libassimp.so")
    SET(ASSIMP_INCLUDE_DIR "" CACHE PATH "Path to assimp/include")
    if(NOT GLFW_LIB_PATH or NOT ASSIMP_LIB_PATH)
        message(FATAL_ERROR "GLFW/Assimp libs are not set")
    endif()
elseif(UNIX)
    find_package(X11 REQUIRED)
    target_link_libraries(LearnOpenGL ${X11_LIBRARIES})
endif()

target_link_libraries(LearnOpenGL ${ASSIMP_LIB_PATH} ${GLFW_LIB_PATH} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS} Threads::Threads)
target_include_directories(LearnOpenGL PUBLIC include ${OPENGL_INCLUDE_DIR} ${ASSIMP_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${GLAD_INCLUDE_DIR} ${DEPS_FOLDER})
message("Include dir is ${GLAD_INCLUDE_DIR}")