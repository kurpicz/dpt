/*******************************************************************************
 * dpt/tree/succinct_node.hpp
 *
 * Part of dpt - Distributed Patricia Tries
 *
 * Copyright (C) 2016 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <ostream>

namespace dpt {
namespace tree {

template <typename LocalIndex>
struct succinct_node {

  LocalIndex number;
  LocalIndex position;

  succinct_node(LocalIndex number = LocalIndex(0),
    LocalIndex position = LocalIndex(0))
  : number(number), position(position) { }

  bool operator == (const succinct_node& n) const {
    return std::tie(number, n.number) == std::tie(position, n.position);
  }

  bool operator != (const succinct_node& n) const {
    return !(n == *this);
  }

  friend std::ostream& operator << (std::ostream& os, const succinct_node& n) {
    return os << '(' << n.number << '|' << n.position << ')';
  }
} __attribute__ ((packed)); // struct node

} // namespace tree
} // namespace dpt

/******************************************************************************/
