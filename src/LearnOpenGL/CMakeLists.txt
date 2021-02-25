SET(SOURCES

    src/LearnOpenGL.cpp
)

find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(Threads REQUIRED)

add_executable(LearnOpenGL ${SOURCES})
target_link_libraries(LearnOpenGL GLAD ${X11_LIBRARIES} ${GLFW_LIB_PATH} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS} Threads::Threads)
target_include_directories(LearnOpenGL PUBLIC include ${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${GLAD_INCLUDE_DIR})
message("Include dir is ${GLAD_INCLUDE_DIR}")