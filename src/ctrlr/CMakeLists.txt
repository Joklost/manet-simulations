project(ctrlr VERSION 1.0)

add_executable(ctrlr src/main.cpp src/ctrlr.cpp src/ctrlr.h)

target_link_libraries(ctrlr pthread mpilib)
