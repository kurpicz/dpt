/*******************************************************************************
 * dpt/tree/pointer_edge.hpp
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
namespace tree {

template <typename Alphabet, typename LocalIndex>
struct trie_node {
  LocalIndex string_depth;
  Alphabet out_degree;
  LocalIndex edge_begin;

  trie_node() = default;

  trie_node(const LocalIndex string_dpth, const Alphabet out_dgr,
            const LocalIndex edge_beg)
    : string_depth(string_dpth), out_degree(out_dgr), edge_begin(edge_beg) { }

  friend std::ostream& operator << (std::ostream& os, const trie_node& te) {
    return os << "(string_depth: " << static_cast<size_t>(te.string_depth) 
              << ", out_degree: " << static_cast<size_t>(te.out_degree) 
              << ", edge_begin: " << static_cast<size_t>(te.edge_begin) << ")";
  }
} __attribute__ ((packed)); // struct trie_node

} // namespace tree
} // namespace dpt

/******************************************************************************/
