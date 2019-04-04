/*******************************************************************************
 * dpt/tree/compact_trie.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include "query/query_view.hpp"
#include "tree/search_result.hpp"

namespace dpt {
namespace tree {

template <typename Alphabet,
          typename GlobalIndex,
          typename LocalIndex,
          template <typename, typename, typename> class CompactTrieStructure>
class compact_trie {

  using manager = dpt::com::manager<Alphabet, GlobalIndex, LocalIndex>;
  using query_view = dpt::query::query_view<Alphabet, LocalIndex>;

public:
  compact_trie() { }

  template <template <typename, typename, typename> class Communication>
  void construct(const std::vector<GlobalIndex>& global_sa,
    const std::vector<GlobalIndex>& global_lcp, manager& manager,
    const GlobalIndex max_query_length) {
    trie_.template construct<Communication>(
      global_sa, global_lcp, manager, max_query_length);
  }

  inline search_result<uint32_t> first_occurrence(const query_view& query) const {
    const auto result = trie_.first_occurrence(query);
    // TODO: this can probably be expressed more consicely
    //       I wrote this out to more explicit of what is happening

    // if the query falls in between to processors: NO_MATCH
    if ((result.state == search_state::LEFT_OF && 
      (result.position % 2 == 0)) || // if left of one processor
      (result.state == search_state::RIGHT_OF &&
      (result.position % 2 == 1))) { // if right of on processor 
      return { search_state::NO_MATCH, 0};

    // if the query falls on a processor but between edges
    } else if ((result.state == search_state::LEFT_OF && 
      (result.position % 2 == 1)) || // if left of one processor
      (result.state == search_state::RIGHT_OF &&
      (result.position % 2 == 0))) { // if right of on processor 
      return { search_state::MATCH, result.position };
    }
    // else we have found a match already
    assert(search_state::MATCH);
    if ((result.state == search_state::LEFT_OF && !(result.position & 1UL)) ||
      (result.state == search_state::RIGHT_OF && (result.position & 1UL))) {
      return { search_state::NO_MATCH, 0 };
    }
    return result;
  }

  inline search_result_pair<uint32_t>first_and_last_occurrence(
    const query_view& query) const {
    const auto result = trie_.first_and_last_occurrence(query);
    // TODO: this can probably be expressed more consicely
    //       I wrote this out to more explicit of what is happening

    // if the query falls in between to processors: NO_MATCH
    if ((result.state == search_state::LEFT_OF && 
      (result.left_position % 2 == 0)) || // if left of one processor
      (result.state == search_state::RIGHT_OF &&
      (result.right_position % 2 == 1))) { // if right of on processor 
      assert(result.left_position == result.right_position);
      return { search_state::NO_MATCH, 0, 0 };

    // if the query falls on a processor but between edges
    } else if ((result.state == search_state::LEFT_OF && 
      (result.left_position % 2 == 1)) || // if left of one processor
      (result.state == search_state::RIGHT_OF &&
      (result.right_position % 2 == 0))) { // if right of on processor 
      assert(result.left_position == result.right_position);
      return { search_state::MATCH, result.left_position, result.right_position };
    }
    // else we have found a match already
    assert(search_state::MATCH);
    return result;
  }

private:
  CompactTrieStructure<Alphabet, GlobalIndex, LocalIndex> trie_;

}; // class compact_trie

} // namespace tree
} // namespace dpt

/******************************************************************************/
