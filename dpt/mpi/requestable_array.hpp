/*******************************************************************************
 * dpt/mpi/requestable_array.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <mpi.h>
#include <vector>

#include "mpi/allreduce.hpp"
#include "mpi/environment.hpp"
#include "mpi/type_mapper.hpp"

namespace dpt {
namespace mpi {

template <typename DataType>
class requestable_array {

public:
  requestable_array(std::vector<DataType>& data, size_t total_size,
                    environment env = environment())
    : local_size_(data.size()), slice_size_(total_size / env.size()),
      data_(data.data()), env_(env) {
    
    MPI_Win_create(data_,
                   local_size_,
                   sizeof(DataType),
                   MPI_INFO_NULL,
                   env_.communicator(),
                   &win_);
  }

  requestable_array(size_t local_size, DataType* data, size_t total_size,
                    environment env = environment())
    : local_size_(local_size), slice_size_(total_size / env.size()),
      data_(data), env_(env){

    MPI_Win_create(data_,
                   local_size_,
                   sizeof(DataType),
                   MPI_INFO_NULL,
                   env_.communicator(),
                   &win_);
  }

  inline DataType& operator [](size_t index) {
    return data_[index];
  }

  inline DataType operator [](size_t index) const {
    return data_[index];
  }

  inline DataType& back() {
    return data_[local_size_ - 1];
  }

  inline DataType back() const {
    return data_[local_size_ - 1];
  }

  template <typename IndexType>
  std::vector<DataType> request(std::vector<IndexType>& request_positions) {
    std::vector<DataType> result(request_positions.size());
    
    bool completed = false;
    size_t cur_pos = 0;
    size_t iteration = 1;
    while (!completed) {
      size_t const cur_end = std::min(request_positions.size(),
                                      iteration * req_round_size);
      MPI_Win_fence(MPI_MODE_NOSTORE | MPI_MODE_NOPUT | MPI_MODE_NOPRECEDE, win_);

      for (; cur_pos < cur_end; ++cur_pos) {
        int32_t rank = std::min<int32_t>(request_positions[cur_pos] / slice_size_,
                                         env_.size() - 1);
        size_t pos = request_positions[cur_pos] - (rank * slice_size_);

        MPI_Get(result.data() + cur_pos,
                1,
                dtm_.get_mpi_type(),
                rank,
                pos,
                1,
                dtm_.get_mpi_type(),
                win_);
      }
      MPI_Win_fence(MPI_MODE_NOSTORE | MPI_MODE_NOSUCCEED, win_);

      bool tmp_completed = (request_positions.size() == cur_pos);
      completed = dpt::mpi::allreduce_and(tmp_completed);
      ++iteration;
    }
    return result;
  }

  template <typename IndexType, typename LocalIndex>
  std::vector<DataType> request(std::vector<IndexType>& request_positions,
                                std::vector<LocalIndex>& request_lengths) {
    size_t total_length = 0;
    for (const auto length : request_lengths) {
      total_length += length;
    }
    std::vector<DataType> result(total_length);
    
    bool completed = false;
    size_t cur_req = 0;
    size_t cur_pos = 0;
    size_t iteration = 1;
    while (!completed) {
      size_t const cur_end = std::min(request_positions.size(),
                                      iteration * req_round_size);
      MPI_Win_fence(MPI_MODE_NOSTORE | MPI_MODE_NOPUT | MPI_MODE_NOPRECEDE, win_);

      for (; cur_req < cur_end; ++cur_req) {
        int32_t rank = std::min<int32_t>(request_positions[cur_req] / slice_size_,
                                         env_.size() - 1);
        size_t pos = request_positions[cur_req] - (rank * slice_size_);

        MPI_Get(result.data() + cur_pos,
                request_lengths[cur_req],
                dtm_.get_mpi_type(),
                rank,
                pos,
                request_lengths[cur_req],
                dtm_.get_mpi_type(),
                win_);
      }
      MPI_Win_fence(MPI_MODE_NOSTORE | MPI_MODE_NOSUCCEED, win_);

      bool tmp_completed = (request_positions.size() == cur_req);
      completed = dpt::mpi::allreduce_and(tmp_completed);
      cur_pos += request_lengths[cur_req];
      ++iteration;
    }
    return result;
  }

private:
  size_t local_size_;
  size_t slice_size_;
  DataType* data_;
  environment env_;
  data_type_mapper<DataType> dtm_;

  static constexpr size_t req_round_size = 1024 * 128;

  MPI_Win win_;

}; // class requestable_array

} // namespace mpi
} // namespace dpt

/******************************************************************************/
