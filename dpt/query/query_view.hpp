/*******************************************************************************
 * dpt/query/query.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <ostream>

namespace dpt {
namespace query {

/// \brief Element returned by the query list (similar to the (new)
///        string view).
///
/// \tparam Alphabet Type of the data that is distributed.
/// \tparam LocalIndex Type of an index position on the local data.
template <typename Alphabet, typename LocalIndex>
struct query_view {

  const Alphabet* query; /// Pointer to the beginning of the query.
  const LocalIndex length; /// Length of the query.

  /// \param pos Position on the query.
  /// \returns Character at the given position \e pos.
  const Alphabet operator [] (const size_t pos) const {
    return *(query + pos);
  }

  /// \param other_string Pointer to data the query should be compared with.
  /// \returns \e true if the query and the data starting at the given pointer
  ///          are the same and \e false otherwise.
  bool operator == (const Alphabet* other_string) const {
    for (LocalIndex pos = 0; pos < length; ++pos) {
      if (*(query + pos) != *(other_string + pos)) {
        return false;
      }
    }
    return true;
  }

  friend std::ostream& operator << (std::ostream& os, const query_view& q) {
    os << '"';
    for (LocalIndex pos = 0; pos < q.length; ++pos) {
      os << q[pos];
    }
    os << "\" (length: " << q.length << ')';
    return os;
  }
} __attribute__ ((packed)); // struct query

} // namespace query
} // namespace dpt

/******************************************************************************/
