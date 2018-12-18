project(controller VERSION 1.0)

add_executable(controller src/main.cpp src/ctrl.cpp src/ctrl.h)

target_link_libraries(controller pthread mpilib)
