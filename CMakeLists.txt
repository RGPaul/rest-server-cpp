#
#  CMakeLists.txt
#
#  Created by Ralph-Gordon Paul on 24. Nov 2020
#  Copyright (c) 2020 Ralph-Gordon Paul. All rights reserved.
#

cmake_minimum_required (VERSION 3.18)
project (restserver)
set(CMAKE_CXX_STANDARD 17)

# For Visual Studio Code
include(CMakeToolsHelpers OPTIONAL)

# we prefer libraries over frameworks
set(CMAKE_FIND_FRAMEWORK LAST)

# add local cmake modules to module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# conan dependencies (try conan_paths first and then conanbuildinfo)
if (EXISTS ${CMAKE_BINARY_DIR}/conan_paths.cmake)
    include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
elseif (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
endif()

# on unix systems, the dependencies should be installed under /usr/local/lib /usr/local/include
if(UNIX)
    set(CMAKE_PREFIX_PATH "/usr/local/")
    set(CMAKE_INCLUDE_PATH "/usr/local/include/" CACHE FILEPATH "Path to headers (include) for third party libraries")
    set(CMAKE_LIBRARY_PATH "/usr/local/lib/" CACHE FILEPATH "Path to third party libraries")
    include_directories(BEFORE ${CMAKE_INCLUDE_PATH})
    link_directories(${CMAKE_LIBRARY_PATH})
endif()

# on windows systems, the dependencies should be installed under C:/lib and C:/include
if (WIN32)
    set(CMAKE_INCLUDE_PATH "C:/include" CACHE FILEPATH "Path to headers (include) for third party libraries")
    set(CMAKE_LIBRARY_PATH "C:/lib" CACHE FILEPATH "Path to third party libraries")
    include_directories(BEFORE ${CMAKE_INCLUDE_PATH})
    link_directories(${CMAKE_LIBRARY_PATH})
endif()

# setup clang compiler options
if((CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    message("Setting clang options")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")
endif()

# setup gcc compiler options
if((CMAKE_CXX_COMPILER_ID MATCHES "GCC"))
    message("Setting clang options")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -std=gnu++17")
endif()

# set default visibility to hidden
if(APPLE AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    message("Setting fvisibility options for clang")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
endif()

if (WIN32)
    # set msvc runtime property
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

### link external libraries ###

# Boost - https://www.boost.org
if (DEFINED CONAN_BOOST_ROOT)
    set(BOOST_ROOT ${CONAN_BOOST_ROOT})
endif()
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
if (WIN32)
    # disable autolinking in boost
    add_definitions( -DBOOST_ALL_NO_LIB )
endif()
find_package(Boost 1.73.0 REQUIRED COMPONENTS filesystem log program_options random system)

# OpenSSL - https://www.openssl.org/
if (DEFINED CONAN_OPENSSL_ROOT)
    set(OPENSSL_ROOT_DIR ${CONAN_OPENSSL_ROOT})
endif()
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED COMPONENTS Crypto SSL)
set(SSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR})
set(SSL_LIBRARIES ${OPENSSL_LIBRARIES})

# nlohmann json - https://github.com/nlohmann/json
if (DEFINED CONAN_NLOHMANN_JSON_ROOT)
    set(NLOHMANN_JSON_ROOT ${CONAN_NLOHMANN_JSON_ROOT})
endif()
find_package(NlohmannJson REQUIRED)

### create executable ###

add_executable(restserver
                src/main.cpp
                )

### link libraries ###

target_include_directories(restserver PRIVATE 
                            ${Boost_INCLUDE_DIRS}
                            ${NLOHMANN_JSON_INCLUDE_DIR}
                            ${SSL_INCLUDE_DIR}
                            )

target_link_libraries(restserver
                        ${Boost_LIBRARIES}
                        ${SSL_LIBRARIES}
                        )

# add posix thread library for Linux, BSD, Solaris, Minix, ...
if (UNIX AND NOT APPLE)
    target_link_libraries(restserver dl pthread)
elseif(WIN32)
    target_link_libraries(restserver crypt32)
endif()

### PCH ###

if (WIN32)
    target_precompile_headers(restserver PUBLIC "${CMAKE_SOURCE_DIR}/src/windows/Pch.hpp")
endif()

# installation 
install(TARGETS restserver
    RUNTIME DESTINATION bin)
