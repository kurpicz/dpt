/*******************************************************************************
 * dpt/query/query_list.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <algorithm>

#include "query/query_iterator.hpp"
#include "query/query_view.hpp"

namespace dpt {
namespace query {

/// \brief Container for queries.
///
/// \tparam Alphabet Type of the data that is distributed.
/// \tparam GlobalIndex Type of an index position on the global data.
/// \tparam LocalIndex Type of an index position on the local data.
template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
class query_list {

public:
  /// \brief Empty constructor.
  query_list() { }

  /// \brief Constructor taking a list of queries separated by a special
  ///        character, i.e., the separator.
  ///
  /// \param queries Vector containing the queries separated by a separator.
  /// \param separator Character that separates the queries.
  /// \param max_query_length Maximum length a query can have.
  query_list(std::vector<Alphabet>&& queries, const Alphabet separator,
    const LocalIndex max_query_length) {
    start_positions_.emplace_back(0);
    for (size_t i = 0; i < queries.size(); ++i) {
      size_t length = 0;
      while (queries[i] != separator && i < queries.size() 
        && length++ < max_query_length) {
        ++i;
      }
      std::copy_n(queries.begin() + start_positions_.back() +
        start_positions_.size() - 1, length, std::back_inserter(queries_));
      start_positions_.emplace_back(queries_.size());
    }
  }

  /// \brief Constructor taking a list of queries and their lengths.
  ///
  /// \param queries Vector containing all concatenated queries.
  /// \param lengths Vector containing the length of each query.
  query_list(std::vector<Alphabet>&& queries,
    std::vector<LocalIndex>&& query_lengths) : queries_(std::move(queries)) {
    start_positions_.reserve(query_lengths.size());
    start_positions_.emplace_back(0);
    for (const auto length : query_lengths) {
      start_positions_.emplace_back(start_positions_.back() + length);
    }
  }

  /// \return The number of queries in the query list.
  inline size_t size() const {
    return start_positions_.size() - 1;
  }

  /// \param index The index of the requested query.
  /// \return The requested query.
  const query_view<Alphabet, LocalIndex> operator [] (size_t index) const {
    return query_view<Alphabet, LocalIndex> {
      queries_.data() + start_positions_[index],
      start_positions_[index + 1] - start_positions_[index] };
  }

  /// \return Constant iterator pointing to the first query in the list.
  const_query_iterator<Alphabet, GlobalIndex, LocalIndex> begin() const {
    return const_query_iterator<Alphabet, GlobalIndex, LocalIndex>(*this, 0);
  }

  /// \return Constant iterator pointing behind the last query in the list.
  const_query_iterator<Alphabet, GlobalIndex, LocalIndex> end() const {
    return const_query_iterator<Alphabet, GlobalIndex, LocalIndex>(
      *this, size());
  }

private:
  std::vector<Alphabet> queries_;
  std::vector<LocalIndex> start_positions_;

}; // class query_list

} // namespace query
} // namespace dpt

/******************************************************************************/
