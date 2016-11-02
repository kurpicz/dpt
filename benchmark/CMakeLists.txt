################################################################################
# benchmark/CMakeLists.txt
#
# Part of dpt - Distributed Patricia Trie
#
# Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

add_executable(dpt dpt.cpp)

target_link_libraries(dpt 
  tlx_command_line
  dpt_mpi)

target_include_directories(dpt PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dpt/>
  $<INSTALL_INTERFACE:${PROJECT_SOURCE_DIR}/dpt/>
)

target_compile_options(dpt
  PRIVATE
  ${CMAKE_CXX_COMPILE_FLAGS}
  $<$<CONFIG:Debug>:${DPT_DEBUG_FLAGS}>
  $<$<CONFIG:Release>:${DPT_RELEASE_FLAGS}>)

add_executable(array_transform array_transform.cpp)

target_link_libraries(array_transform 
  tlx_command_line
  dpt_mpi)

target_include_directories(array_transform PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dpt/>
  $<INSTALL_INTERFACE:${PROJECT_SOURCE_DIR}/dpt/>
)

target_compile_options(array_transform
  PRIVATE
  ${CMAKE_CXX_COMPILE_FLAGS}
  $<$<CONFIG:Debug>:${DPT_DEBUG_FLAGS}>
  $<$<CONFIG:Release>:${DPT_RELEASE_FLAGS}>)

################################################################################
