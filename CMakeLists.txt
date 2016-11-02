################################################################################
# CMakeLists.txt
#
# Part of dpt - Distributed Patricia Trie
#
# Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/misc/CMakeModules") 

# Project Information
project("dpt")
set(PROJECT_VENDOR "Florian Kurpicz")
set(PROJECT_CONTACT "florian.kurpicz@tu-dortmund.de")
set(PROJECT_DESCRIPTION "dpt: Distributed Patricia Tries")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++1z")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -g")

# Include MPI
find_package(MPI REQUIRED)
if (MPI_FOUND)
  include_directories(SYSTEM ${MPI_INCLUDE_PATH})
  set(ALL_LIBRARIES ${ALL_LIBRARIES} ${MPI_LIBRARIES})
  set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
endif()

# Include the SDSL
find_package(SDSL REQUIRED)
include_directories(SYSTEM ${SDSL_INCLUDE_DIRS})
set(ALL_LIBRARIES ${ALL_LIBRARIES} ${SDSL})

# Add TLX command line parser
add_library(tlx_command_line
  external/tlx/tlx/cmdline_parser.cpp
  external/tlx/tlx/string/parse_si_iec_units.cpp)

target_include_directories(tlx_command_line PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/tlx/>
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/tlx/>
)


add_subdirectory(dpt)
# Add benchmark
add_subdirectory(benchmark)

# Add tests
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(external/googletest)
add_subdirectory(tests)

################################################################################
