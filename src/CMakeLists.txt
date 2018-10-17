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

#hunter_add_package(spdlog)
#find_package(spdlog CONFIG REQUIRED)
hunter_add_package(Catch)
find_package(Catch2 CONFIG REQUIRED)

hunter_add_package(Boost COMPONENTS random system thread filesystem chrono atomic date_time regex)
find_package(Boost CONFIG REQUIRED random system thread filesystem chrono atomic date_time regex)

hunter_add_package(Beast)
find_package(Beast CONFIG REQUIRED)

hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)

#[[

set(FETCH_EXTERNAL_CPPRESTSDK
        URL https://github.com/Microsoft/cpprestsdk/archive/v2.10.1.tar.gz
        URL_HASH MD5=bed28c3685beee9c977ae1bf5ae7e9ef)

ExternalProject_Add(cpprestsdk_external
        PREFIX ${CMAKE_BINARY_DIR}/external/cpprestsdk/
        ${FETCH_EXTERNAL_CPPRESTSDK}
        INSTALL_DIR ${CMAKE_BINARY_DIR}/external/cpprestsdk/
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> <SOURCE_DIR>/Release)
]]#[[
ExternalProject_Add(cpprestsdk_external
        GIT_REPOSITORY https://github.com/Microsoft/cpprestsdk.git
        CONFIGURE_COMMAND cd ../cpprestsdk_external/Release/ && cmake -DCMAKE_CXX_FLAGS=-fPIC -DGFLAGS=ON
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DBUILD_SHARED_LIBS=ON
        -DBUILD_DOCUMENTATION=OFF -DCMAKE_VERBOSE_MAKEFILE=ON
        -DCMAKE_BUILD_TYPE:STRING=Release
        -DBUILD_EXAMPLES=OFF
        -DBUILD_TESTING=OFF
        -DEXPORT_NONPUBLIC_HEADERS=ON
        BUILD_COMMAND cd ../cpprestsdk_external/Release/ && make -j8
        INSTALL_COMMAND cd ../cpprestsdk_external/Release/ && make install -j8
        )



#find_package(cpprestsdk REQUIRED)
add_library(cpprestsdk INTERFACE)
add_dependencies(cpprestsdk cpprestsdk_external)
target_include_directories(cpprestsdk INTERFACE ${CMAKE_BINARY_DIR}/external/cpprestsdk/include)]]

#hunter_add_package(Catch2 CONFIG REQUIRED)

#[[
include(ProcessorCount)
ProcessorCount(NJOBS)
if(NJOBS EQUAL 0)
    set(NJOBS 1)
endif()]]
#[[

### Catch2 ###

set(FETCH_EXTERNAL_CATCH
        URL https://github.com/catchorg/Catch2/archive/v2.4.1.tar.gz
        URL_HASH MD5=b0564e8cec53f597a0c67181fc15a3f3)

ExternalProject_Add(catch2_external
        PREFIX ${CMAKE_BINARY_DIR}/external/Catch
        ${FETCH_EXTERNAL_CATCH}
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/external/Catch/src/catch2_external/single_include/catch2/catch.hpp ${CMAKE_BINARY_DIR}/external/Catch/include/catch.hpp)

add_library(Catch INTERFACE)
add_dependencies(Catch catch2_external)
target_include_directories(Catch INTERFACE ${CMAKE_BINARY_DIR}/external/Catch/include)
target_compile_definitions(Catch INTERFACE $<$<CXX_COMPILER_ID:MSVC>:_SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING>)

### spdlog ###

set(FETCH_EXTERNAL_SPDLOG
        URL https://github.com/gabime/spdlog/archive/v1.2.0.tar.gz
        URL_HASH MD5=f3828e57320560348e59e0f575b66f8a)

ExternalProject_Add(spdlog_external
        PREFIX ${CMAKE_BINARY_DIR}/external/spdlog
        ${FETCH_EXTERNAL_SPDLOG}
        INSTALL_DIR ${CMAKE_BINARY_DIR}/external/spdlog/
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> <SOURCE_DIR>)

add_library(spdlog INTERFACE)
add_dependencies(spdlog spdlog_external)
target_include_directories(spdlog INTERFACE ${CMAKE_BINARY_DIR}/external/spdlog/include)
]]
#[[

### Boost ###

set(BOOST_BOOTSTRAP_COMMAND ./bootstrap.sh)
set(BOOST_B2_COMMAND ./b2)
set(BOOST_BUILD_ARGS )
list(APPEND BOOST_BUILD_ARGS "threading=multi")
list(APPEND BOOST_BUILD_ARGS "link=shared")
list(APPEND BOOST_BUILD_ARGS "variant=release")

set(FETCH_EXTERNAL_BOOST
        URL https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.gz
        URL_HASH MD5=5d8b4503582fffa9eefdb9045359c239)

ExternalProject_Add(boost_external
        PREFIX ${CMAKE_BINARY_DIR}/external/boost/
        ${FETCH_EXTERNAL_BOOST}
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ${BOOST_BOOTSTRAP_COMMAND} --with-libraries=random,system,thread,filesystem,chrono,atomic,date_time,regex
        BUILD_COMMAND ""
        INSTALL_COMMAND ${BOOST_B2_COMMAND} -d2 -q -j${NJOBS} install --layout=tagged --debug-configuration ${BOOST_BUILD_ARGS}
        )

find_package(Boost REQUIRED)
add_library(boost INTERFACE)
add_dependencies(boost boost_external)
target_include_directories(boost INTERFACE ${CMAKE_BINARY_DIR}/external/boost/src/boost_external)
target_link_libraries(boost INTERFACE
        Boost::boost
        Boost::random
        Boost::system
        Boost::thread
        Boost::filesystem
        Boost::chrono
        Boost::atomic
        Boost::date_time
        Boost::regex)
#link_directories(${CMAKE_BINARY_DIR}/external/boost/src/boost_external/stage/lib)


### REST SDK ###
]]

add_subdirectory(mpilib)
add_subdirectory(mpi)
add_subdirectory(reachi)
add_subdirectory(lmac)