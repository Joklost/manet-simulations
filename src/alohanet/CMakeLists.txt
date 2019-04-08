project(alohanet VERSION 1.0)

add_executable(alohanet
        $<TARGET_OBJECTS:geo>
        $<TARGET_OBJECTS:mpilib>
        src/alohanet.cpp)

target_link_libraries(alohanet mpilib geo)
target_link_libraries(alohanet pthread)
