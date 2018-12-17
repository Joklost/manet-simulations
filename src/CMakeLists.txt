cmake_minimum_required(VERSION 3.12)

project(manetsims VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -pthread)

include(GNUInstallDirs)
include(ExternalProject)

message(${PROJECT_SOURCE_DIR})

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/../.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/libs/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

add_subdirectory(libs)

# # # # # # # # # # # # # # # Catch2 # # # # # # # # # # # # # # #
#set(FETCH_EXTERNAL_CATCH
#        URL https://github.com/catchorg/Catch2/archive/v2.4.1.tar.gz
#        URL_HASH MD5=b0564e8cec53f597a0c67181fc15a3f3)
#
#ExternalProject_Add(catch2_external
#        PREFIX ${CMAKE_BINARY_DIR}/external/Catch
#        ${FETCH_EXTERNAL_CATCH}
#        CONFIGURE_COMMAND ""
#        BUILD_COMMAND ""
#        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/external/Catch/src/catch2_external/single_include/catch2/catch.hpp ${CMAKE_BINARY_DIR}/external/Catch/include/catch.hpp)
#
#add_library(Catch INTERFACE)
#add_dependencies(Catch catch2_external)
#target_include_directories(Catch INTERFACE ${CMAKE_BINARY_DIR}/external/Catch/include)
#target_compile_definitions(Catch INTERFACE $<$<CXX_COMPILER_ID:MSVC>:_SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING>)

#find_package(Catch2 CONFIG REQUIRED)


# # # # # # # # # # # # # # # spdlog # # # # # # # # # # # # # # #

#set(FETCH_EXTERNAL_SPDLOG
#        URL https://github.com/gabime/spdlog/archive/v1.2.0.tar.gz
#        URL_HASH MD5=f3828e57320560348e59e0f575b66f8a)
#
#ExternalProject_Add(spdlog_external
#        PREFIX ${CMAKE_BINARY_DIR}/external/spdlog
#        ${FETCH_EXTERNAL_SPDLOG}
#        INSTALL_DIR ${CMAKE_BINARY_DIR}/external/spdlog/
#        CONFIGURE_COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> <SOURCE_DIR>)
#
#add_library(spdlog INTERFACE)
#add_dependencies(spdlog spdlog_external)
#target_include_directories(spdlog INTERFACE ${CMAKE_BINARY_DIR}/external/spdlog/include)

#find_package(spdlog CONFIG REQUIRED)


# # # # # # # # # # # # # # # # cpr # # # # # # # # # # # # # # # #
#
#SET(FETCH_EXTERNAL_CPR
#        URL https://github.com/whoshuu/cpr/archive/1.3.0.tar.gz
#        URL_HASH MD5=f9df0c649208b06dd314699b4eb43759)
#
#ExternalProject_Add(cpr_external
#        PREFIX ${CMAKE_BINARY_DIR}/external/cpr
#        ${FETCH_EXTERNAL_CPR}
#        INSTALL_DIR ${CMAKE_BINARY_DIR}/external/cpr/
#        #CONFIGURE_COMMAND ${CMAKE_COMMAND} -DBUILD_CPR_TESTS=OFF -DUSE_SYSTEM_CURL=ON -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> <SOURCE_DIR>
#        )
#
#add_library(cpr INTERFACE)
#add_dependencies(cpr cpr_external)
#target_include_directories(cpr INTERFACE ${CMAKE_BINARY_DIR}/external/cpr/include)

#find_package(cpr CONFIG REQUIRED)


# # # # # # # # # # # # # # # # json # # # # # # # # # # # # # # #
#
#SET(FETCH_EXTERNAL_JSON
#        URL https://github.com/nlohmann/json/archive/v3.4.0.tar.gz
#        URL_HASH MD5=ebe637e7f9b0abe4f57f4f56dd2e5e74)
#
#ExternalProject_Add(json_external
#        PREFIX ${CMAKE_BINARY_DIR}/external/json
#        ${FETCH_EXTERNAL_CPR}
#        INSTALL_DIR ${CMAKE_BINARY_DIR}/external/json/
#        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DBUILD_TESTING=0 -DJSON_MultipleHeaders=ON
#        CONFIGURE_COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> <SOURCE_DIR>)
#
#add_library(json INTERFACE)
#add_dependencies(json json_external)
#target_include_directories(json INTERFACE ${CMAKE_BINARY_DIR}/external/json/include)

#find_package(nlohmann_json CONFIG REQUIRED)
include_directories(${CPR_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS} ${CATCH_INCLUDE_DIRS})

set(MPI_EXECUTABLE_SUFFIX ".mpich")
find_package(MPI REQUIRED)

add_subdirectory(mpilib)
add_subdirectory(mpi)
add_subdirectory(reachi)
add_subdirectory(lmac)
add_subdirectory(controller)