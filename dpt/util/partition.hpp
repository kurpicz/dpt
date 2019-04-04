/*******************************************************************************
 * dpt/util/partition.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once
#ifndef DPT_UTIL_TEXT_HEADER
#define DPT_UTIL_TEXT_HEADER

#include <algorithm>
#include <assert.h>
#include <vector>

#include "mpi/environment.hpp"
#include "util/named_structs.hpp"

namespace dpt {
namespace util {

/// \brief Used to represent distrubuted data. 
///
/// We use partitions to represent distributed data. A partition provides access
/// to more information than just the data; it can be queried for the processing
/// element and local position of any distributed element (distributed in this
/// partition). A partition is always distributed within an \e environment.
///
/// \tparam Alphabet Type of the data that is distributed.
/// \tparam GlobalIndex Type of an index position on the global data.
/// \tparam LocalIndex Type of an index position on the local data.
template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
class partition {

  using pe_and_position = dpt::util::pe_position<LocalIndex>;

public:
  partition(dpt::mpi::environment env = dpt::mpi::environment()) : env_(env), global_size_(0),
    local_size_(0), min_local_size_(0) { }

  partition(const GlobalIndex global_size, const GlobalIndex local_size,
    std::vector<Alphabet>&& local_data, dpt::mpi::environment env = dpt::mpi::environment())
  : env_(env),
    global_size_(global_size),
    local_size_(local_size),
    min_local_size_(global_size_ / env_.size()),
    local_data_(std::move(local_data)) { }

  partition(const partition& other) : env_(other.env_),
    global_size_(other.global_size_), local_size_(other.local_size_),
    min_local_size_(other.min_local_size_),
    local_data_(other.local_data_) { }

  partition(partition&& other) : env_(std::move(other.env_)),
    global_size_(other.global_size_), local_size_(other.local_size_),
    min_local_size_(other.min_local_size_),
    local_data_(std::move(other.local_data_)) { }

  partition& operator = (partition&& other) {
    env_ = std::move(other.env_);
    global_size_ = other.global_size_;
    local_size_ = other.local_size_;
    min_local_size_ = other.min_local_size_;
    local_data_ = std::move(other.local_data_);
    return *this;
  }

  inline const Alphabet operator [] (LocalIndex index) const {
    assert(index < static_cast<LocalIndex>(local_data_.size()));
    return local_data_[index];
  }

  inline const dpt::mpi::environment text_environment() const {
    return env_;
  }

  inline size_t global_size() const {
    return global_size_;
  }

  inline size_t local_size() const {
    return local_size_;
  }

  inline std::vector<Alphabet>* local_data() {
    return &local_data_;
  }

  inline const std::vector<Alphabet>* const_local_data() const {
    return &local_data_;
  }

  inline auto data_begin() const {
    return local_data_.begin();
  }

  inline auto data_end() const {
    return local_data_.end();
  }

  inline int32_t pe(const GlobalIndex index) const {
    return std::min(
      static_cast<int32_t>(index / min_local_size_), env_.size() -  1);
  }

  inline pe_and_position pe_and_norm_position(
    const GlobalIndex index) const {
    const auto pe =
      std::min(static_cast<int32_t>(index / min_local_size_), env_.size() - 1);
    return pe_and_position { pe,
      static_cast<LocalIndex>(index - (pe * min_local_size_)) };
  }

private:
  dpt::mpi::environment env_;

  size_t global_size_;
  size_t local_size_;
  size_t min_local_size_; // the local size of PEs excl the last one
  std::vector<Alphabet> local_data_;

}; // class partition

} // namespace util
} // namespace dpt

#endif // DPT_UTIL_TEXT_HEADER

/******************************************************************************/
