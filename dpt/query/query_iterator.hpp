/*******************************************************************************
 * dpt/query/query_iterator.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <iterator>

#include "query/query_view.hpp"

namespace dpt {
namespace query {

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
class query_list;

/// \brief Constant iterator for the query list.
///
/// \tparam Alphabet Type of the data that is distributed.
/// \tparam GlobalIndex Type of an index position on the global data.
/// \tparam LocalIndex Type of an index position on the local data.
template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
class const_query_iterator {

public:
  using self_type = const_query_iterator;
  using value_type = query_view<Alphabet, LocalIndex>;
  using reference = query_view<Alphabet, LocalIndex>&;
  using iterator_category = std::forward_iterator_tag;
  using difference_type = size_t;

public:
  const_query_iterator(
    const query_list<Alphabet, GlobalIndex, LocalIndex>& q_list,
    size_t cur_pos) : q_list_(q_list), cur_pos_(cur_pos) { }

  const_query_iterator() : q_list_(nullptr), cur_pos_(0) { }

  self_type& operator ++ () {
    ++cur_pos_;
    return *this;
  }

  self_type& operator ++ (int /*add*/) {
    self_type i = *this;
    ++cur_pos_;
    return i;
  }

  const value_type operator * () const {
    return q_list_[cur_pos_];
  }

  bool operator == (const self_type& other) const {
    return cur_pos_ == other.cur_pos_;
  }
  
  bool operator != (const self_type& other) const {
    return !(other == *this);
  }

private:
  const query_list<Alphabet, GlobalIndex, LocalIndex>& q_list_;
  size_t cur_pos_;

}; // class const_query_iterator

} // namespace query
} // namespace dpt

/******************************************************************************/
