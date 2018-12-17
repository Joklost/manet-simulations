project(controller VERSION 1.0)

add_executable(controller src/main.cpp)

target_link_libraries(controller pthread mpilib)
