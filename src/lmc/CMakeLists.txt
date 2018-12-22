project(linkmodel_interface VERSION 1.0)

add_executable(linkmodel_interface src/main.cpp src/lmc.cpp src/lmc.h)

target_link_libraries(linkmodel_interface pthread mpilib reachi)
