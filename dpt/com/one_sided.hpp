/*******************************************************************************
 * dpt/com/one_sided.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <memory>

#include "mpi/requestable_array.hpp"
#include "query/query_list.hpp"
#include "util/partition.hpp"

namespace dpt {
namespace com {

/// \brief Implementation of the communication among processing elements using
///        one-sided communication.
///
/// \tparam Alphabet Type of the data that is distributed.
/// \tparam GlobalIndex Type of an index position on the global data.
/// \tparam LocalIndex Type of an index position on the local data.
template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
class one_sided_communication {

  using partition = dpt::util::partition<Alphabet, GlobalIndex, LocalIndex>;
  using q_list = dpt::query::query_list<Alphabet, GlobalIndex, LocalIndex>;

private:
  static constexpr size_t MAX_REQUESTS = 2147483647;

public:

  /// \param text_positions A vector of text positions.
  /// \param local_text The local (partition) of the data to distribute.
  /// \returns Globally distributed characters based on their (global) position.
  static std::vector<Alphabet> request_characters(
    std::vector<GlobalIndex>& text_positions,
    partition& local_text);

  /// \param text_positions A vector of text positions.
  /// \param substring_lengths A vector of length of the requested substrings.
  /// \param local_text The local (partition) of the data to distribute.
  /// \returns Globally distributed substrings based on their (global) position.
  static std::vector<Alphabet> request_substrings(
    std::vector<GlobalIndex>& text_positions,
    const std::vector<LocalIndex>& substring_lengths,
    partition& local_text_);
  
  /// \param text_positions A vector of text positions.
  /// \param substring_lengths A vector of length of the requested substrings.
  /// \param local_text The local (partition) of the data to distribute.
  /// \returns Globally distributed substrings based on their (global) position
  ///          but returns the first character (head) of each substring in a
  ///          separate vector.
  static std::pair<std::vector<Alphabet>, std::vector<Alphabet>>
    request_substrings_head(std::vector<GlobalIndex>& text_positions,
      const std::vector<LocalIndex>& substring_lengths,
      partition& local_text);

  /// \param queries A vector of text (all queries concatenated w/o separator).
  /// \param query_lengths A vector of length of the queries.
  /// \param local_text The local (partition) of the data to distribute.
  /// \returns All queries that have been send to this processing element.
  static q_list distribute_queries(
    const std::vector<Alphabet>& queries,
    const std::vector<LocalIndex>& query_lengths,
    const std::vector<int32_t> hist_lengths, const std::vector<int32_t> hist);

}; // class one_sided_communication

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
std::vector<Alphabet> one_sided_communication<Alphabet, GlobalIndex, LocalIndex>
  ::request_characters(std::vector<GlobalIndex>& text_positions,
     partition& local_text) {
  dpt::mpi::requestable_array requestable(local_text.global_size(),
                                          local_text.local_data()->data(),
                                          local_text.local_size());
  auto result = requestable.request(text_positions);
  return result;
} // request_characters

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
std::vector<Alphabet> one_sided_communication<Alphabet, GlobalIndex, LocalIndex>
  ::request_substrings(
    std::vector<GlobalIndex>& text_positions,
    const std::vector<LocalIndex>& substring_lengths,
    partition& local_text) {

  dpt::mpi::requestable_array requestable(local_text.global_size(),
                                          local_text.local_data()->data(),
                                          local_text.local_size());

  std::vector<Alphabet> result = requestable.request(text_positions,
                                                     substring_lengths);
  return result;
} // request_substrings

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
std::pair<std::vector<Alphabet>, std::vector<Alphabet>>
  one_sided_communication<Alphabet, GlobalIndex, LocalIndex>
  ::request_substrings_head(std::vector<GlobalIndex>& text_positions,
    const std::vector<LocalIndex>& substring_lengths,
    partition& local_text) {

  dpt::mpi::requestable_array requestable(local_text.global_size(),
                                          local_text.local_data()->data(),
                                          local_text.local_size());

  auto heads = requestable.request(text_positions);

  std::vector<GlobalIndex> tail_pos(text_positions.size());
  std::vector<LocalIndex> tail_length(substring_lengths.size());
  for (size_t i = 0; i < text_positions.size(); ++i) {
    tail_pos[i] = text_positions[i] + 1;
    tail_length[i] = substring_lengths[i] - 1;
  }

  auto tails = requestable.request(tail_pos, tail_length);

  return std::make_pair(heads, tails);

} // request_substrings_head

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
dpt::query::query_list<Alphabet, GlobalIndex, LocalIndex>
one_sided_communication<Alphabet, GlobalIndex, LocalIndex>
  ::distribute_queries(
    const std::vector<Alphabet>& queries,
    const std::vector<LocalIndex>& query_lengths,
    const std::vector<int32_t> hist_lengths, const std::vector<int32_t> hist) {

  std::cout << "Not yet implemented. Please use 'collective "
            << "communication' to distribute queries" << std::endl;

  return dpt::query::query_list<Alphabet, GlobalIndex, LocalIndex>();

} // distribute_queries

} // namespace com
} // namespace dpt

/******************************************************************************/
