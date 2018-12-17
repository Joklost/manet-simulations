project(lmac VERSION 1.0)

add_executable(lmac src/main.cpp)

target_link_libraries(lmac mpilib)
target_link_libraries(lmac pthread)
