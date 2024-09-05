cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(ATL24_openoceanspp VERSION 1.0.0 LANGUAGES CXX)

find_package(OpenMP REQUIRED)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(CMAKE_CXX_FLAGS "-Wall -Werror ${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(INSTALLDIR /usr/local CACHE STRING "Installation directory for library and executables")

# Plugin #
add_library(openoceanspp SHARED ${CMAKE_CURRENT_LIST_DIR}/openoceanspp.cpp)
target_sources(openoceanspp PRIVATE ${CMAKE_CURRENT_LIST_DIR}/OpenOceansPPClassifier.cpp)
target_include_directories (openoceanspp PUBLIC ..)
target_include_directories (openoceanspp PUBLIC ../apps)
target_include_directories (openoceanspp PUBLIC ${INSTALLDIR}/include/sliderule)
target_include_directories (openoceanspp PUBLIC ${INSTALLDIR}/include/sliderule/bathy)
target_include_directories (openoceanspp PUBLIC ${INSTALLDIR}/include/sliderule/icesat2)
file(STRINGS ${INSTALLDIR}/etc/sliderule/version.txt LIBVER)
target_compile_definitions (openoceanspp PUBLIC BINID="${LIBVER}")
target_precompile_headers(openoceanspp PUBLIC ../oopp/precompiled.h)
set_target_properties (openoceanspp PROPERTIES OUTPUT_NAME openoceanspp)
set_target_properties (openoceanspp PROPERTIES PREFIX "")


# Build Information #
execute_process (COMMAND git --work-tree ${PROJECT_SOURCE_DIR}/.. --git-dir ${PROJECT_SOURCE_DIR}/../.git describe --abbrev --dirty --always --tags --long OUTPUT_VARIABLE BUILDINFO)
string(REGEX REPLACE "\n$" "" BUILDINFO "${BUILDINFO}")
target_compile_definitions (openoceanspp PUBLIC BUILDINFO="${BUILDINFO}")

# Plugin Installation #
install (TARGETS openoceanspp LIBRARY DESTINATION ${INSTALLDIR}/lib/sliderule)
