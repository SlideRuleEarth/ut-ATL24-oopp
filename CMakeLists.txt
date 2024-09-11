cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(oopp VERSION 1.0.0 LANGUAGES CXX)

find_package(OpenMP REQUIRED)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(CMAKE_CXX_FLAGS "-Wall -Werror ${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/oopp)

############################################################
# Unit tests
############################################################
macro(add_test name)
    add_executable(${name} ./tests/${name}.cpp)
    target_link_libraries(${name})
    target_precompile_headers(${name} PUBLIC oopp/precompiled.h)
endmacro()

add_test(test_dataframe)
add_test(test_oopp)
add_test(test_utils)

############################################################
# Applications
############################################################

macro(add_app name)
    add_executable(${name} ./apps/${name}.cpp)
    target_link_libraries(${name})
    target_precompile_headers(${name} PUBLIC oopp/precompiled.h)
endmacro()

add_app(classify)
add_app(score)
