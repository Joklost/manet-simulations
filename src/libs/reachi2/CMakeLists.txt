project(reachi2 VERSION 1.0)

add_library(reachi2 src/linkmodel.cpp src/datagen.cpp src/radiomodel.cpp)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(reachi2 PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE src test)

# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS reachi2 EXPORT Reachi2LibraryConfig
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

target_link_libraries(reachi2 geo eigen)
enable_testing()

add_subdirectory(test)
