project(lmc VERSION 1.0)

add_executable(lmc src/main.cpp src/lmc.cpp src/lmc.h)

target_link_libraries(lmc pthread mpilib sims)

if(INSTALL_HTTP)
    target_link_libraries(lmc http)
endif()
