project(scratch VERSION 1.0)

add_executable(scratch src/main.cpp)

target_link_libraries(scratch mpilib)
target_link_libraries(scratch pthread)
