project(coordinator VERSION 1.0)

add_executable(coordinator
        $<TARGET_OBJECTS:common>
        $<TARGET_OBJECTS:geo>
        $<TARGET_OBJECTS:mpilib>
        src/main.cpp
        src/models/coordinator.h src/models/coordinator.cpp
        src/models/gpslog.h src/models/gpslog.cpp
        src/models/action.h src/models/action.cpp
        src/models/packet.h src/models/packet.cpp
        src/models/link.h src/models/link.cpp
        src/models/radiomodel.h src/models/radiomodel.cpp
        src/models/topology.h src/models/node.cpp src/models/node.h)

target_link_libraries(coordinator pthread mpilib common geo)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/logs/gpslog_rssi.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
