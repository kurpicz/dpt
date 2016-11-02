/*******************************************************************************
 * dpt/mpi/big_type.hpp
 *
 * Copyright (C) 2016 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 * Based on
 *     https://github.com/jeffhammond/BigMPI/blob/master/src/type_contiguous_x.c
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <limits>

#include <mpi.h>

#include "mpi/type_mapper.hpp"

namespace dpt {
namespace mpi {

template <typename DataType>
MPI_Datatype get_big_type(const size_t size) {
  MPI_Datatype result;

  size_t mpi_max_int = std::numeric_limits<std::int32_t>::max();
  size_t nr_blocks = size / mpi_max_int;
  size_t left_elements = size % mpi_max_int;

  MPI_Datatype block_type;
  MPI_Datatype blocks_type;
  MPI_Type_contiguous(mpi_max_int, type_mapper<DataType>::type(), &block_type);
  MPI_Type_contiguous(nr_blocks, block_type, &blocks_type);

  if (left_elements) {
    MPI_Datatype leftover_type;
    MPI_Type_contiguous(
      left_elements, type_mapper<DataType>::type(), &leftover_type);

    MPI_Aint lb, extent;
    MPI_Type_get_extent(type_mapper<DataType>::type(), &lb, &extent);
    MPI_Aint displ = nr_blocks * mpi_max_int * extent;
    MPI_Aint displs[2] = {0, displ};
    std::int32_t blocklen[2] = { 1, 1 };
    MPI_Datatype mpitypes[2] = { blocks_type, leftover_type };
    MPI_Type_create_struct(2, blocklen, displs, mpitypes, &result);
    MPI_Type_commit(&result);
    MPI_Type_free(&leftover_type);
    MPI_Type_free(&blocks_type);
  } else {
    result = blocks_type;
    MPI_Type_commit(&result);
  }
  return result;
}

} // namespace mpi
} // namespace dpt

/******************************************************************************/
