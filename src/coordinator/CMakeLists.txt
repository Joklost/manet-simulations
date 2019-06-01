project(coordinator VERSION 1.0)

add_executable(coordinator
        $<TARGET_OBJECTS:common>
        $<TARGET_OBJECTS:geo>
        $<TARGET_OBJECTS:mpilib>
        src/main.cpp
        src/coordinator.h src/coordinator.cpp
        src/gpslog.h src/gpslog.cpp
        src/models/action.h src/models/action.cpp
        src/models/packet.h src/models/packet.cpp
        src/models/radiomodel.h src/models/radiomodel.cpp
        src/models/topology.h
        src/models/node.h src/models/node.cpp
        src/models/location.h src/models/statistics.cpp src/models/statistics.h src/models/topology.cpp)

target_link_libraries(coordinator pthread mpilib common geo)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/logs/2nodes_rssi.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/logs/gpslog_rssi.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/logs/gridlog_2x2_rssi.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/logs/gridlog_4x4_rssi.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/logs/gridlog_8x8_rssi.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
