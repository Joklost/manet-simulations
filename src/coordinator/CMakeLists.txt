project(coordinator VERSION 1.0)

find_package(MPI REQUIRED)

add_executable(coordinator
        src/main.cpp
        src/gpslog.cpp
        src/gpslog.h
        src/coordinator.cpp
        src/coordinator.h
        src/models/location.h
        src/models/action.cpp
        src/models/action.h
        src/models/packet.cpp
        src/models/packet.h
        src/models/radiomodel.cpp
        src/models/radiomodel.h
        src/models/topology.cpp
        src/models/topology.h
        src/models/node.cpp
        src/models/node.h
        src/models/statistics.cpp
        src/models/statistics.h)

target_include_directories(coordinator PRIVATE ${MPI_CXX_INCLUDE_PATH})
target_compile_options(coordinator PRIVATE ${MPI_CXX_COMPILE_FLAGS})
target_link_libraries(coordinator ${MPI_CXX_LIBRARIES} ${MPI_CXX_LINK_FLAGS} mpilib pthread common geo)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/logs/2nodes_rssi.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/logs/gpslog_rssi.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/logs/gridlog_2x2_rssi.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/logs/gridlog_4x4_rssi.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/logs/gridlog_8x8_rssi.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
