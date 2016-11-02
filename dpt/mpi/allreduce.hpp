/*******************************************************************************
 * dpt/mpi/allreduce.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <type_traits>

#include "mpi/environment.hpp"
#include "mpi/type_mapper.hpp"

namespace dpt {
namespace mpi {

static inline bool allreduce_and(bool& send_data,
  environment env = environment()) {

  bool receive_data;
  MPI_Allreduce(
    &send_data,
    &receive_data,
    type_mapper<bool>::factor(),
    type_mapper<bool>::type(),
    MPI_LAND,
    env.communicator());
  return receive_data;
}

template <typename DataType>
static inline DataType allreduce_max(DataType& send_data,
  environment env = environment()) {
  static_assert(std::is_arithmetic<DataType>(),
    "Only arithmetic types are allowed for allreduce_max.");
  DataType receive_data;
  MPI_Allreduce(
    &send_data,
    &receive_data,
    type_mapper<DataType>::factor(),
    type_mapper<DataType>::type(),
    MPI_MAX,
    env.communicator());
  return receive_data;
}

} // namespace mpi
} // namespace dpt

/******************************************************************************/
