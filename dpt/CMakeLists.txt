################################################################################
# dpt/CMakeLists.txt
#
# Part of dpt - Distributed Patricia Trie
#
# Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

set(DPT_MPI_IMPLS
  mpi/environment.cpp)

add_library(dpt_mpi
  ${DPT_MPI_IMPLS})

target_link_libraries(dpt_mpi
  ${MPI_LIBRARIES})

target_include_directories(dpt_mpi PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# file(GLOB SRCS
#   RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
#   ${CMAKE_CURRENT_SOURCE_DIR}/bench/*.[ch]pp
#   ${CMAKE_CURRENT_SOURCE_DIR}/com/*.[ch]pp
#   ${CMAKE_CURRENT_SOURCE_DIR}/mpi/*.[ch]pp
#   ${CMAKE_CURRENT_SOURCE_DIR}/query/*.[ch]pp
#   ${CMAKE_CURRENT_SOURCE_DIR}/tree/*.[ch]pp
#   ${CMAKE_CURRENT_SOURCE_DIR}/util/*.[ch]pp
# )

# add_library(dpt ${SRCS})
# set_target_properties(dpt PROPERTIES LINKER_LANGUAGE CXX)

################################################################################
