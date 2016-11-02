/*******************************************************************************
 * dpt/mpi/all_to_all.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <mpi.h>
#include <numeric>
#include <vector>

#include "mpi/allreduce.hpp"
#include "mpi/big_type.hpp"
#include "mpi/environment.hpp"
#include "mpi/type_mapper.hpp"

namespace dpt {
namespace mpi {

template <typename DataType>
inline std::vector<DataType> alltoall(std::vector<DataType>& send_data,
  environment env = environment()) {
  std::vector<DataType> receive_data(send_data.size(), 0);
  data_type_mapper<DataType> dtm;
  MPI_Alltoall(send_data.data(),
               send_data.size() / env.size(),
               dtm.get_mpi_type(),
               receive_data.data(),
               send_data.size() / env.size(),
               dtm.get_mpi_type(),
               env.communicator());
  return receive_data;
}

template <typename DataType>
inline std::pair<std::vector<size_t>, std::vector<DataType>> alltoallv_small(
  std::vector<DataType>& send_data, std::vector<size_t>& send_counts,
  environment env = environment()) {

  std::vector<int32_t> real_send_counts(send_counts.size());
  for (size_t i = 0; i < send_counts.size(); ++i) {
    real_send_counts[i] = static_cast<int32_t>(send_counts[i]);
  }
  std::vector<int32_t> receive_counts = alltoall(real_send_counts, env);
  std::vector<size_t> real_receive_counts = alltoall(send_counts, env);

  std::vector<int32_t> send_displacements(real_send_counts.size(), 0);
  std::vector<int32_t> receive_displacements(real_send_counts.size(), 0);
  for (size_t i = 1; i < real_send_counts.size(); ++i) {
    send_displacements[i] = send_displacements[i - 1] + real_send_counts[i - 1];
    receive_displacements[i] = receive_displacements[i - 1] +
      receive_counts[i - 1];
  }
  std::vector<DataType> receive_data(
    receive_counts.back() + receive_displacements.back());

  data_type_mapper<DataType> dtm;
  MPI_Alltoallv(send_data.data(),
                real_send_counts.data(),
                send_displacements.data(),
                dtm.get_mpi_type(),
                receive_data.data(),
                receive_counts.data(),
                receive_displacements.data(),
                dtm.get_mpi_type(),
                env.communicator());
  
  return std::make_pair(real_receive_counts, receive_data);
}

template <typename DataType>
inline std::vector<DataType> alltoallv(std::vector<DataType>& send_data,
    std::vector<size_t>& send_counts, environment env = environment()) {

  size_t local_send_count = std::accumulate(
    send_counts.begin(), send_counts.end(), 0);

  std::vector<size_t> receive_counts = alltoall(send_counts, env);
  size_t local_receive_count = std::accumulate(
    receive_counts.begin(), receive_counts.end(), 0);

  size_t local_max = std::max(local_send_count, local_receive_count);
  size_t global_max = allreduce_max(local_max, env);

  if (global_max < env.mpi_max_int()) {
    std::vector<DataType> result;
    std::tie(std::ignore, result) = alltoallv_small(send_data, send_counts, env);
    return result;
  } else {
    std::vector<size_t> send_displacements(0, env.size());
    for (size_t i = 1; i < send_counts.size(); ++i) {
      send_displacements[i] = send_displacements[i - 1] + send_counts[i - 1];
    }
    std::vector<size_t> receive_displacements(0, env.size());
    for (size_t i = 1; i < send_counts.size(); ++i) {
      receive_displacements[i] =
        receive_displacements[i - 1] + receive_counts[i - 1];
    }

    std::vector<MPI_Request> mpi_request(2 * env.size());
    std::vector<DataType> receive_data(receive_displacements.back() +
      receive_counts.back());
    for (int32_t i = 0; i < env.size(); ++i) {
      // start with self send/recv
      auto source = (env.rank() + (env.size() - i)) % env.size();
      auto receive_type = get_big_type<DataType>(receive_counts[source]);
      MPI_Irecv(receive_data.data() + receive_displacements[source],
                1,
                receive_type,
                source,
                44227,
                env.communicator(),
                &mpi_request[source]);
    }
    // dispatch sends
    for (int32_t i = 0; i < env.size(); ++i) {
      auto target = (env.rank() + i) % env.size();
      auto send_type = get_big_type<DataType>(send_counts[target]);
      MPI_Isend(send_data.data() + send_displacements[target],
                1,
                send_type,
                target,
                44227,
                env.communicator(),
                &mpi_request[env.size() + target]);
    }
    MPI_Waitall(2 * env.size(), mpi_request.data(), MPI_STATUSES_IGNORE);
    return receive_data;
  }
}

template <typename DataType>
inline std::pair<std::vector<size_t>, std::vector<DataType>> alltoallv_counts(
  std::vector<DataType>& send_data, std::vector<size_t>& send_counts,
  environment env = environment()) {

  size_t local_send_count = std::accumulate(
    send_counts.begin(), send_counts.end(), 0);

  std::vector<size_t> receive_counts = alltoall(send_counts, env);
  size_t local_receive_count = std::accumulate(
    receive_counts.begin(), receive_counts.end(), 0);

  size_t local_max = std::max(local_send_count, local_receive_count);
  size_t global_max = allreduce_max(local_max, env);

  if (global_max < env.mpi_max_int()) {
    return alltoallv_small(send_data, send_counts, env);
  } else {
    std::vector<size_t> send_displacements(0, env.size());
    for (size_t i = 1; i < send_counts.size(); ++i) {
      send_displacements[i] = send_displacements[i - 1] + send_counts[i - 1];
    }
    std::vector<size_t> receive_displacements(0, env.size());
    for (size_t i = 1; i < send_counts.size(); ++i) {
      receive_displacements[i] =
        receive_displacements[i - 1] + receive_counts[i - 1];
    }

    std::vector<MPI_Request> mpi_request(2 * env.size());
    std::vector<DataType> receive_data(receive_displacements.back() +
      receive_counts.back());
    for (int32_t i = 0; i < env.size(); ++i) {
      // start with self send/recv
      auto source = (env.rank() + (env.size() - i)) % env.size();
      auto receive_type = get_big_type<DataType>(receive_counts[source]);
      MPI_Irecv(receive_data.data() + receive_displacements[source],
                1,
                receive_type,
                source,
                44227,
                env.communicator(),
                &mpi_request[source]);
    }
    // dispatch sends
    for (int32_t i = 0; i < env.size(); ++i) {
      auto target = (env.rank() + i) % env.size();
      auto send_type = get_big_type<DataType>(send_counts[target]);
      MPI_Isend(send_data.data() + send_displacements[target],
                1,
                send_type,
                target,
                44227,
                env.communicator(),
                &mpi_request[env.size() + target]);
    }
    MPI_Waitall(2 * env.size(), mpi_request.data(), MPI_STATUSES_IGNORE);
    return std::make_pair(receive_counts, receive_data);
  }
}

} // namespace mpi
} // namespace dpt

/******************************************************************************/
