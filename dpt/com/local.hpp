/*******************************************************************************
 * dpt/com/local.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <dpt/query/query_list.hpp>
#include <dpt/util/partition.hpp>

namespace dpt {
namespace com {

/// \brief Implementation (wrapper) of the communication that can be used if
///        there is just one processing element (mainly for testing purpose).
///
/// \tparam Alphabet Type of the data that is distributed.
/// \tparam GlobalIndex Type of an index position on the global data.
/// \tparam LocalIndex Type of an index position on the local data.
template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
class local_communication {

  using partition = dpt::util::partition<Alphabet, GlobalIndex, LocalIndex>;
  using pos_size_request = dpt::util::position_size<LocalIndex>;
  using q_list = dpt::query::query_list<Alphabet, GlobalIndex, LocalIndex>;

public:
  /// \param text_positions A vector of text positions.
  /// \param local_text The local (partition) of the data to distribute.
  /// \returns Globally distributed characters based on their (global) position.
  static std::vector<Alphabet> request_characters(
    std::vector<GlobalIndex>& text_positions,
    const partition& local_text);

  /// \param text_positions A vector of text positions.
  /// \param substring_lengths A vector of length of the requested substrings.
  /// \param local_text The local (partition) of the data to distribute.
  /// \returns Globally distributed substrings based on their (global) position.
  static std::vector<Alphabet> request_substrings(
    std::vector<GlobalIndex>& text_positions,
    const std::vector<LocalIndex>& substring_lengths,
    const partition& local_text_);

  /// \param text_positions A vector of text positions.
  /// \param substring_lengths A vector of length of the requested substrings.
  /// \param local_text The local (partition) of the data to distribute.
  /// \returns Globally distributed substrings based on their (global) position
  ///          but returns the first character (head) of each substring in a
  ///          separate vector.
  static std::pair<std::vector<Alphabet>, std::vector<Alphabet>>
    request_substrings_head(std::vector<GlobalIndex>& text_positions,
      const std::vector<LocalIndex>& substring_lengths,
      const partition& local_text);

  /// \param queries A vector of text (all queries concatenated w/o separator).
  /// \param query_lengths A vector of length of the queries.
  /// \param local_text The local (partition) of the data to distribute.
  /// \returns All queries that have been send to this processing element.
  static q_list distribute_queries(
    const std::vector<Alphabet>& queries,
    const std::vector<LocalIndex>& query_lengths,
    const std::vector<int32_t> hist_lengths, const std::vector<int32_t> hist);

}; // class local_communication

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
std::vector<Alphabet> local_communication<Alphabet, GlobalIndex, LocalIndex>
  ::request_characters(std::vector<GlobalIndex>& text_positions,
    const partition& local_text) {

  std::vector<Alphabet> result;
  for (const auto& req : text_positions) {
    result.emplace_back(local_text[req]);
  }

  return result;
} // request_characters

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
std::vector<Alphabet> 
  local_communication<Alphabet, GlobalIndex, LocalIndex>
  ::request_substrings(
    std::vector<GlobalIndex>& text_positions,
    const std::vector<LocalIndex>& substring_lengths,
    const partition& local_text) {

  assert(text_positions.size() == substring_lengths.size());

  std::vector<Alphabet> result;
  for (size_t i = 0; i < text_positions.size(); ++i) {
    std::copy_n(local_text.data_begin() + text_positions[i],
      substring_lengths[i], std::back_inserter(result));
  }

  return result;
} // request_substrings

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
std::pair<std::vector<Alphabet>, std::vector<Alphabet>> 
  local_communication<Alphabet, GlobalIndex, LocalIndex>
  ::request_substrings_head(std::vector<GlobalIndex>& text_positions,
    const std::vector<LocalIndex>& substring_lengths,
    const partition& local_text) {

  assert(text_positions.size() == substring_lengths.size());

  std::vector<Alphabet> heads;
  std::vector<Alphabet> tails;

  std::vector<Alphabet> result;
  for (size_t i = 0; i < text_positions.size(); ++i) {
    heads.emplace_back(local_text[text_positions[i]]);
    std::copy_n(local_text.data_begin() + text_positions[i] + 1,
      substring_lengths[i] - 1, std::back_inserter(tails));
  }

  return std::make_pair(heads, tails);
} // request_substrings_head

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
dpt::query::query_list<Alphabet, GlobalIndex, LocalIndex> 
local_communication<Alphabet, GlobalIndex, LocalIndex>
  ::distribute_queries(
    const std::vector<Alphabet>& queries,
    const std::vector<LocalIndex>& query_lengths,
    const std::vector<int32_t> hist_lengths, const std::vector<int32_t> hist) {

    return dpt::query::query_list<Alphabet, GlobalIndex, LocalIndex>(
      std::move(queries), std::move(query_lengths));
} // distribute_queries

} // namespace com
} // namespace dpt

/******************************************************************************/
