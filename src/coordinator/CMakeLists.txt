project(coordinator VERSION 1.0)

add_executable(coordinator
        $<TARGET_OBJECTS:common>
        $<TARGET_OBJECTS:geo>
        $<TARGET_OBJECTS:sims>
        $<TARGET_OBJECTS:mpilib>
        src/main.cpp src/coordinator.cpp src/coordinator.h)

target_link_libraries(coordinator pthread mpilib sims common)
