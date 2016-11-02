/*******************************************************************************
 * dpt/mpi/io.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <fstream>
#include <iterator>
#include <memory>
#include <mpi.h>
#include <tuple>

#include "mpi/type_mapper.hpp"
#include "mpi/environment.hpp"
#include "util/partition.hpp"

namespace dpt {
namespace mpi {

using namespace dpt::util;

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
partition<Alphabet, GlobalIndex, LocalIndex>
  distribute_file(std::string file_name, const GlobalIndex padding,
  environment env = environment()) {

  MPI_File file;
  MPI_File_open(
    env.communicator(),
    const_cast<char*>(file_name.c_str()),
    MPI_MODE_RDONLY,
    MPI_INFO_NULL,
    &file);

  MPI_Offset global_size;
  MPI_File_get_size(
    file,
    &global_size);
  global_size /= sizeof(Alphabet);

  GlobalIndex local_size = size_t(global_size / env.size());
  MPI_File_seek(
    file,
    local_size * env.rank() * sizeof(Alphabet),
    MPI_SEEK_SET);

  if (env.rank() + 1 == env.size()) {
    local_size += size_t(global_size % env.size());
  }

  std::vector<Alphabet> local_data(local_size + padding);
  MPI_File_read(
    file,
    local_data.data(),
    (local_size + padding) * sizeof(Alphabet),
    MPI_BYTE,
    MPI_STATUS_IGNORE);


  return partition<Alphabet, GlobalIndex, LocalIndex>(
    GlobalIndex((size_t)global_size), local_size,
    std::move(local_data), env);
}

template <typename ReadAlphabet,
          typename WriteAlphabet,
          typename GlobalIndex,
          typename LocalIndex>
partition<WriteAlphabet, GlobalIndex, LocalIndex>
  distribute_and_transform_file(std::string file_name,
  const GlobalIndex padding, environment env = environment()) {

  MPI_File file;
  MPI_File_open(
    env.communicator(),
    const_cast<char*>(file_name.c_str()),
    MPI_MODE_RDONLY,
    MPI_INFO_NULL,
    &file);

  MPI_Offset global_size;
  MPI_File_get_size(
    file,
    &global_size);

  GlobalIndex local_size = (global_size / env.size());
  MPI_File_seek(
    file,
    local_size * env.rank() * sizeof(ReadAlphabet),
    MPI_SEEK_SET);

  if (env.rank() + 1 == env.size()) {
    local_size += (global_size % env.size());
  }

  std::vector<ReadAlphabet> local_data_tmp(local_size + padding);
  MPI_File_read(
    file,
    local_data_tmp.data(),
    (local_size + padding) * sizeof(ReadAlphabet),
    MPI_BYTE,
    MPI_STATUS_IGNORE);
  std::vector<WriteAlphabet> local_data;
  local_data.reserve(local_data_tmp.size());
  for (const auto& tmp : local_data_tmp) {
    local_data.emplace_back(static_cast<WriteAlphabet>(tmp));
  }

  return partition<WriteAlphabet, GlobalIndex, LocalIndex>(
    static_cast<GlobalIndex>(global_size), local_size,
    std::move(local_data), env);
}

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
std::vector<Alphabet> distribute_linewise(const std::string& file_name,
  const GlobalIndex lines_per_pe, const LocalIndex max_line_length,
  const Alphabet separator, environment env = environment()) {

  std::vector<Alphabet> lines;
  if (env.rank() == 0) {
    std::ifstream stream(file_name.c_str(), std::ios::in | std::ios::binary);
    if (!stream.good()) {
      std::exit(EXIT_FAILURE);
    }

    size_t j = 0;
    std::string tmp;
    while (std::getline(stream, tmp) && ++j < lines_per_pe) { }

    for (int32_t i = 1; i < env.size(); ++i) {
      j = 0;
      lines.clear();
      std::string line;
      while (j < lines_per_pe && std::getline(stream, line)) {
        std::copy_n(line.begin(),
          std::min(
            static_cast<LocalIndex>(std::distance(line.begin(), line.end())),
            max_line_length),
          std::back_inserter(lines));
        lines.emplace_back(separator);
        ++j;
      }

      MPI_Send(
        lines.data(),
        lines.size() * type_mapper<Alphabet>::factor(),
        type_mapper<Alphabet>::type(),
        i,
        0,
        env.communicator());
    }

    stream.clear();
    stream.seekg(0, std::ios::beg); 
    if (!stream.good()) {
      std::exit(EXIT_FAILURE);
    }

    j = 0;
    lines.clear();
    std::string line;
    while (std::getline(stream, line) && j < lines_per_pe) {
      std::copy_n(line.begin(),
        std::min(
          static_cast<LocalIndex>(std::distance(line.begin(), line.end())),
          max_line_length),
        std::back_inserter(lines));
      lines.emplace_back(separator);
      ++j;
    }
  } else {
    MPI_Status status;
    int32_t message_size;
    MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
    MPI_Get_count(
      &status,
      type_mapper<Alphabet>::type(),
      &message_size);

    lines.resize(message_size);

    MPI_Recv(
      lines.data(),
      message_size,
      type_mapper<Alphabet>::type(),
      0,
      0,
      env.communicator(),
      &status);
  }
  return lines;
}

template <typename DataType>
static void write_data(std::vector<DataType>& local_data,
  const std::string& file_name, environment env = environment()) {

  MPI_File mpi_file;

  MPI_File_open(env.communicator(),
                const_cast<char*>(file_name.c_str()),
                MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL,
                &mpi_file);

  MPI_File_write_ordered(mpi_file,
                         reinterpret_cast<unsigned char*>(local_data.data()),
                         local_data.size() * sizeof(DataType),
                         MPI_BYTE,
                         MPI_STATUS_IGNORE);
  MPI_File_close(&mpi_file);
}

} // namespace mpi
} // namespace dpt

/******************************************************************************/
