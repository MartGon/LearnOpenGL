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
    if(GLFW_LIB_PATH)
        target_link_libraries(LearnOpenGL ${GLFW_LIB_PATH} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS} Threads::Threads)
    else()
        message(FATAL_ERROR "GLFW libs are not set")
    endif()
elseif(UNIX)
    find_package(X11 REQUIRED)
    target_link_libraries(LearnOpenGL ${X11_LIBRARIES} ${GLFW_LIB_PATH} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS} Threads::Threads)
endif()

target_include_directories(LearnOpenGL PUBLIC include ${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${GLAD_INCLUDE_DIR} ${DEPS_FOLDER})
message("Include dir is ${GLAD_INCLUDE_DIR}")