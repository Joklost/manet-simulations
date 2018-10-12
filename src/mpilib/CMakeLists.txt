project(mpilib VERSION 1.0 LANGUAGES CXX)

add_library(mpilib src/unpacker.cpp src/geomath.cpp src/link.cpp src/node.cpp)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(mpilib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE src test)

# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS mpilib EXPORT MPILibLibraryConfig
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

target_link_libraries(mpilib spdlog linkmodel)

enable_testing()

add_subdirectory(test)