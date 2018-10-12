project(linkmodel VERSION 1.0 LANGUAGES CXX)

add_library(linkmodel src/math.cpp)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(linkmodel PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE src)

# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS linkmodel EXPORT LinkmodelLibraryConfig
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

target_link_libraries(linkmodel mpilib)

enable_testing()

add_subdirectory(test)