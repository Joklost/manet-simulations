project(coordr VERSION 1.0)

add_executable(coordr src/main.cpp src/coordr.cpp src/coordr.h)

target_link_libraries(coordr pthread mpilib reachi common)
