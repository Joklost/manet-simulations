project(lmac VERSION 1.0)

add_executable(lmac
        $<TARGET_OBJECTS:mpilib>
        src/lmac.cpp src/models.h)

target_link_libraries(lmac mpilib)
target_link_libraries(lmac pthread)
