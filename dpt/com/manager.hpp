/*******************************************************************************
 * dpt/com/manager.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <mpi.h>

#include "mpi/type_mapper.hpp"
#include "mpi/environment.hpp"
#include "util/partition.hpp"

namespace dpt {
namespace com {

/// \brief Used for communication among the processing elements.
///
/// Each processing element contains a different slice of the text. When
/// constructing the distributed patricia trie (and when answering queries),
/// some sumbstrings from different processing elements may be required. We
/// use the \e manager to bundle all this communication in one class that offers
/// different types of MPI communication (collective and one-sided).
///
/// \tparam Alphabet Type of the data that is distributed.
/// \tparam GlobalIndex Type of an index position on the global data.
/// \tparam LocalIndex Type of an index position on the local data.
template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
class manager {

  using partition = dpt::util::partition<Alphabet, GlobalIndex, LocalIndex>;

public:
  manager() { }

  manager(partition&& local_text)
  : local_text_(std::move(local_text)) { }

  /// \tparam Communication Type of (MPI) communication used. 
  /// \param text_positions A vector of text positions.
  /// \returns Globally distributed characters based on their (global) position.
  template <template <typename, typename, typename> class Communication>
  std::vector<Alphabet> request_characters(
    std::vector<GlobalIndex>& text_positions) {
    return Communication<Alphabet, GlobalIndex, LocalIndex>
      ::request_characters(text_positions, local_text_);
  }

  /// \tparam Communication Type of (MPI) communication used. 
  /// \param text_positions A vector of text positions.
  /// \param substring_lengths A vector of length of the requested substrings.
  /// \returns Globally distributed substrings based on their (global) position.
  template <template <typename, typename, typename> class Communication>
  std::vector<Alphabet> request_substrings(
    std::vector<GlobalIndex>& text_positions,
    const std::vector<LocalIndex>& substring_lengths) {
    return Communication<Alphabet, GlobalIndex, LocalIndex>
      ::request_substrings(text_positions, substring_lengths, local_text_);
  }

  /// \tparam Communication Type of (MPI) communication used. 
  /// \param text_positions A vector of text positions.
  /// \param substring_lengths A vector of length of the requested substrings.
  /// \returns Globally distributed substrings based on their (global) position
  ///          but returns the first character (head) of each substring in a
  ///          separate vector.
  template <template <typename, typename, typename> class Communication>
  std::pair<std::vector<Alphabet>, std::vector<Alphabet>>
    request_substrings_head(std::vector<GlobalIndex>& text_positions,
      const std::vector<LocalIndex>& substring_lengths) {
        return Communication<Alphabet, GlobalIndex, LocalIndex>
          ::request_substrings_head(
            text_positions, substring_lengths, local_text_);
  }

  /// \tparam Communication Type of (MPI) communication used. 
  /// \param queries A vector of text (all queries concatenated w/o separator).
  /// \param query_lengths A vector of length of the queries.
  /// \returns All queries that have been send to this processing element.
  template <template <typename, typename, typename> class Communication>
  auto distribute_queries(
    std::vector<Alphabet>& queries, std::vector<LocalIndex>& query_lengths,
    std::vector<size_t> hist_lengths, std::vector<size_t> hist) {
    return Communication<Alphabet, GlobalIndex, LocalIndex>
      ::distribute_queries(queries, query_lengths, hist_lengths, hist);
  }

  /// \param sa_lcp Local part of the "global" SA and LCP-Array
  /// \returns Global SA and LCP-array, i.e., 
  auto distribute_global_sa_and_lcp(
    std::pair<std::array<GlobalIndex, 2>, std::array<GlobalIndex, 2>>& sa_lcp)
    const {
    dpt::mpi::environment env;
    std::vector<GlobalIndex> sa(env.size() << 1);
    std::vector<GlobalIndex> lcp(env.size() << 1);

    MPI_Allgather(
      sa_lcp.first.data(),
      dpt::mpi::type_mapper<GlobalIndex>::factor() << 1,
      dpt::mpi::type_mapper<GlobalIndex>::type(),
      sa.data(),
      dpt::mpi::type_mapper<GlobalIndex>::factor() << 1,
      dpt::mpi::type_mapper<GlobalIndex>::type(),
      env.communicator());
    MPI_Allgather(
      sa_lcp.second.data(),
      dpt::mpi::type_mapper<GlobalIndex>::factor() << 1,
      dpt::mpi::type_mapper<GlobalIndex>::type(),
      lcp.data(),
      dpt::mpi::type_mapper<GlobalIndex>::factor() << 1,
      dpt::mpi::type_mapper<GlobalIndex>::type(),
      env.communicator());
    return std::make_pair(sa, lcp);
  }

private:
  partition local_text_;

}; // class manager

} // namespace com
} // namespace dpt

/******************************************************************************/
