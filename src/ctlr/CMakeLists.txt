project(ctlr VERSION 1.0)

add_executable(ctlr src/main.cpp src/ctrl.cpp src/ctrl.h)

target_link_libraries(ctlr pthread mpilib reachi)
