/*******************************************************************************
 * dpt/tree/search_result.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <iostream>

namespace dpt {
namespace tree {

enum class search_state {
  NO_MATCH,
  NOT_YET_FOUND,
  MATCH,
  LEFT_OF,
  RIGHT_OF
}; // enum class search_state 

std::ostream& operator << (std::ostream& os, const search_state& ss) {
  switch (ss) {
    case search_state::NO_MATCH: os << "no match"; break;
    case search_state::NOT_YET_FOUND: os << "not yet found"; break;
    case search_state::MATCH: os << "match"; break;
    case search_state::LEFT_OF: os << "left of"; break;
    case search_state::RIGHT_OF: os << "right of"; break;
  }
  return os;
}

template <typename LocalIndex>
struct search_result {
  search_state state;
  LocalIndex position;

  friend std::ostream& operator << (std::ostream& os, const search_result& sr) {
    return os << '(' << sr.state << " @ position: " << sr.position << ')';
  }
}; // struct search_result

template <typename LocalIndex>
struct search_result_pair {
  search_state state;
  LocalIndex left_position;
  LocalIndex right_position;

  friend std::ostream& operator << (std::ostream& os,
    const search_result_pair& srp) {
    return os << '(' << srp.state << " between positions: " << srp.left_position 
              << ", " << srp.right_position << std::endl;
  }
}; // struct search_result_pair

} // namespace tree
} // namespace dpt

/******************************************************************************/
