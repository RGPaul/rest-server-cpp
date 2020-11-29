#  ---------------------------------------------------------------------------------------------------------------------
#  The MIT License (MIT)
#
#  Copyright (c) 2020 Ralph-Gordon Paul. All rights reserved.
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
#  documentation files (the "Software"), to deal in the Software without restriction, including without limitation the 
#  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
#  permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the 
#  Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
#  WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
#  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
#  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#  ---------------------------------------------------------------------------------------------------------------------

cmake_minimum_required (VERSION 3.18)
project (RestServer VERSION 0.1.0 LANGUAGES CXX)
set (CMAKE_CXX_STANDARD 17)

# For Visual Studio Code
include (CMakeToolsHelpers OPTIONAL)

# we prefer libraries over frameworks
set (CMAKE_FIND_FRAMEWORK LAST)

# add local cmake modules to module path
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

# conan dependencies (try conan_paths first and then conanbuildinfo)
if (EXISTS ${CMAKE_BINARY_DIR}/conan_paths.cmake)
    include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
elseif (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
endif()

if (WIN32)
    # set msvc runtime property
    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# setting default visibility
set (CMAKE_CXX_VISIBILITY_PRESET hidden)
set (CMAKE_VISIBILITY_INLINES_HIDDEN YES)

# ----------------------------------------------------------------------------------------------------------------------
# external libraries
# ----------------------------------------------------------------------------------------------------------------------

# Boost - https://www.boost.org
if (DEFINED CONAN_BOOST_ROOT)
    set (BOOST_ROOT ${CONAN_BOOST_ROOT})
endif()
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_RUNTIME ON)
if (WIN32)
    # disable autolinking in boost
    add_definitions ( -DBOOST_ALL_NO_LIB )
endif()
find_package (Boost 1.74.0 REQUIRED COMPONENTS filesystem log program_options random system)

# nlohmann json - https://github.com/nlohmann/json
if (DEFINED CONAN_NLOHMANN_JSON_ROOT)
    set (NLOHMANN_JSON_ROOT ${CONAN_NLOHMANN_JSON_ROOT})
endif()
find_package (NlohmannJson REQUIRED)

# ----------------------------------------------------------------------------------------------------------------------
# create library
# ----------------------------------------------------------------------------------------------------------------------

if (NOT APPLE)
    set (CMAKE_INSTALL_RPATH $ORIGIN)
endif()

set (restserver_public_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rgpaul/RestServer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rgpaul/Session.hpp
)

add_library (RestServer STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RestServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Session.cpp
)

# link dependencies
target_link_libraries (RestServer
    Boost::filesystem
    Boost::log
    Boost::random
    Boost::system
)

# add inlcude folders of dependencies
target_include_directories (RestServer
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE ${NLOHMANN_JSON_INCLUDE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# set version info
set_target_properties (RestServer PROPERTIES
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER "${restserver_public_headers}"
)

### PCH ###

if (WIN32)
    target_precompile_headers(RestServer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/windows/Pch.hpp")
endif()

# ----------------------------------------------------------------------------------------------------------------------
# create executable (sample)
# ----------------------------------------------------------------------------------------------------------------------

add_executable(sample
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_compile_definitions(sample PRIVATE "APP_VERSION=\"${PROJECT_VERSION}\"")

### link libraries ###

target_include_directories(sample PRIVATE 
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE ${NLOHMANN_JSON_INCLUDE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(sample
    Boost::program_options
    RestServer
)

# add posix thread library for Linux, BSD, Solaris, Minix, ...
if (UNIX AND NOT APPLE)
    target_link_libraries(sample dl pthread)
endif()

### PCH ###

if (WIN32)
    target_precompile_headers(sample PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/windows/Pch.hpp")
endif()

# ----------------------------------------------------------------------------------------------------------------------
# Installation
# ----------------------------------------------------------------------------------------------------------------------

# generating cmake package
include (CMakePackageConfigHelpers)
write_basic_package_version_file(
    RestServerConfigVersion.cmake
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion
)

include (GNUInstallDirs)
install (TARGETS RestServer EXPORT RestServerTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT RestServer_Runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT RestServer_Runtime
        NAMELINK_COMPONENT RestServer_Development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT RestServer_Development
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rgpaul
)

install (EXPORT RestServerTargets
    FILE RestServerTargets.cmake
    NAMESPACE RestServer::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RestServer
)

install ( FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/RestServerConfig.cmake"
    "${CMAKE_BINARY_DIR}/RestServerConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RestServer
)
