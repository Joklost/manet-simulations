project(lmac VERSION 1.0)

add_executable(lmac src/lmac.cpp)

target_link_libraries(lmac mpilib)
target_link_libraries(lmac pthread)
