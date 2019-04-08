project(lmc VERSION 1.0)

add_executable(lmc
        $<TARGET_OBJECTS:sims>
        $<TARGET_OBJECTS:mpilib>
        $<TARGET_OBJECTS:geo>
        $<TARGET_OBJECTS:common>
        src/main.cpp src/lmc.cpp src/lmc.h)

target_link_libraries(lmc pthread mpilib geo sims common)

if(INSTALL_HTTP)
    target_link_libraries(lmc http)
endif()
