cmake_minimum_required(VERSION 3.12)

include("cmake/HunterGate.cmake")
HunterGate(
        URL "https://github.com/ruslo/hunter/archive/v0.23.34.tar.gz"
        SHA1 "70287b1ffa810ee4e952052a9adff9b4856d0d54"
)

project(manetsims VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -pthread)

include(GNUInstallDirs)
include(ExternalProject)

hunter_add_package(Catch)
find_package(Catch2 CONFIG REQUIRED)

hunter_add_package(Boost COMPONENTS random system thread filesystem chrono atomic date_time regex)
find_package(Boost CONFIG REQUIRED random system thread filesystem chrono atomic date_time regex)

hunter_add_package(Beast)
find_package(Beast CONFIG REQUIRED)

hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)

hunter_add_package(nlohmann_json)
find_package(nlohmann_json CONFIG REQUIRED)

add_subdirectory(mpilib)
add_subdirectory(mpi)
add_subdirectory(reachi)
add_subdirectory(lmac)