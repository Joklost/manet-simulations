cmake_minimum_required(VERSION 3.7)

project(manetsims VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -pthread)

include(GNUInstallDirs)
include(ExternalProject)

message(${PROJECT_SOURCE_DIR})

find_package(Git QUIET)
if (GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/../.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if (GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if (NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif ()
    endif ()
endif ()

if (NOT EXISTS "${PROJECT_SOURCE_DIR}/libs/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif ()

include_directories(${JSON_INCLUDE_DIRS})

set(MPI_EXECUTABLE_SUFFIX ".mpich")
find_package(MPI REQUIRED)

# Add libraries
add_subdirectory(libs)

# Add executables
add_subdirectory(coordr)
add_subdirectory(scratch)
add_subdirectory(lmc)
#add_subdirectory(lmac)
add_subdirectory(alohanet)
