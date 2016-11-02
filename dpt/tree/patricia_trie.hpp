/*******************************************************************************
 * dpt/tree/patricia_trie.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include "com/manager.hpp"
#include "query/query_list.hpp"
#include "util/partition.hpp"

namespace dpt {
namespace tree {

template <typename Alphabet,
          typename GlobalIndex,
          typename LocalIndex,
          template <typename, typename, typename> class PatriciaTrieStructure>
class patricia_trie {

  using manager = dpt::com::manager<Alphabet, GlobalIndex, LocalIndex>;
  using partition = dpt::util::partition<GlobalIndex, GlobalIndex, LocalIndex>;
  using q_list = dpt::query::query_list<Alphabet, GlobalIndex, LocalIndex>;

public:
  patricia_trie() { }

  template <template <typename, typename, typename> class Communication>
  void construct(partition&& local_sa, partition&& local_lcp,
    manager& manager, const GlobalIndex max_lcp) {
    local_sa_ = std::move(local_sa);
    trie_.template construct<Communication>(local_sa_, local_lcp, manager, max_lcp);
  }

  std::pair<std::array<GlobalIndex, 2>, std::array<GlobalIndex, 2>>
    global_sa_and_lcp() const {
    return trie_.global_sa_and_lcp();
  }

  template <template <typename, typename, typename> class Communication>
  inline auto existential_batched(q_list&& rec_queries, manager& manager)
    const {
    return trie_.template existential_batched<Communication>(
      std::move(rec_queries), manager, local_sa_);
  }

  template <template <typename, typename, typename> class Communication>
  inline auto counting_batched(q_list&& rec_queries, manager& manager) const {
    return trie_.template counting_batched<Communication>(
      std::move(rec_queries), manager, local_sa_);
  }

  template <template <typename, typename, typename> class Communication>
  inline auto enumeration_batched(q_list&& rec_queries, manager& manager) const {
    return trie_.template enumeration_batched<Communication>(
      std::move(rec_queries), manager, local_sa_);
  }

private:
  partition local_sa_;
  PatriciaTrieStructure<Alphabet, GlobalIndex, LocalIndex> trie_;

}; // class patricia_trie

} // namespace tree
} // namespace dpt

/******************************************************************************/
