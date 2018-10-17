project(lmac VERSION 1.0 LANGUAGES CXX)


add_executable(lmac src/main.cpp src/node.cpp src/node.h src/packet.h)

target_link_libraries(lmac mpi mpilib reachi)
target_link_libraries(lmac pthread)
