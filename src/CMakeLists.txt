cmake_minimum_required(VERSION 3.12)
project(manetsims VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -pthread)

include(GNUInstallDirs)
include(ExternalProject)

set(FETCH_EXTERNAL_CATCH
        URL https://github.com/catchorg/Catch2/archive/v2.4.1.tar.gz
        URL_HASH MD5=b0564e8cec53f597a0c67181fc15a3f3)

set(FETCH_EXTERNAL_SPDLOG
        URL https://github.com/gabime/spdlog/archive/v1.2.0.tar.gz
        URL_HASH MD5=f3828e57320560348e59e0f575b66f8a)

ExternalProject_Add(catch2_external
        PREFIX ${CMAKE_BINARY_DIR}/external/Catch
        ${FETCH_EXTERNAL_CATCH}
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/external/Catch/src/catch2_external/single_include/catch2/catch.hpp ${CMAKE_BINARY_DIR}/external/Catch/include/catch.hpp)

ExternalProject_Add(spdlog_external
        PREFIX ${CMAKE_BINARY_DIR}/external/spdlog
        ${FETCH_EXTERNAL_SPDLOG}
        INSTALL_DIR ${CMAKE_BINARY_DIR}/external/spdlog/
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> <SOURCE_DIR>)

add_library(Catch INTERFACE)
add_dependencies(Catch catch2_external)
target_include_directories(Catch INTERFACE ${CMAKE_BINARY_DIR}/external/Catch/include)
target_compile_definitions(Catch INTERFACE $<$<CXX_COMPILER_ID:MSVC>:_SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING>)

add_library(spdlog INTERFACE)
add_dependencies(spdlog spdlog_external)
target_include_directories(spdlog INTERFACE ${CMAKE_BINARY_DIR}/external/spdlog/include)

add_subdirectory(mpilib)
add_subdirectory(mpi)
add_subdirectory(reachi)
add_subdirectory(lmac)