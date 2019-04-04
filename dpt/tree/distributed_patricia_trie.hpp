/*******************************************************************************
 * dpt/tree/distributed_patricia_trie.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include "mpi/environment.hpp"
#include "com/manager.hpp"
#include "mpi/io.hpp"
#include "query/query_list.hpp"
#include "tree/compact_trie.hpp"
#include "tree/patricia_trie.hpp"
#include "tree/search_result.hpp"

namespace dpt {
namespace tree {

template <typename Alphabet,
          typename GlobalIndex,
          typename LocalIndex,
          template <typename, typename, typename> class CompactTrieStructure,
          template <typename, typename, typename> class PatriciaTrieStructure>
class distributed_patricia_trie {

  using manager = dpt::com::manager<Alphabet, GlobalIndex, LocalIndex>;
  using com_trie =
    compact_trie<Alphabet, GlobalIndex, LocalIndex, CompactTrieStructure>;
  using pat_trie =
    patricia_trie<Alphabet, GlobalIndex, LocalIndex, PatriciaTrieStructure>;
  using q_list = dpt::query::query_list<Alphabet, GlobalIndex, LocalIndex>;

public:
  distributed_patricia_trie() { }

  distributed_patricia_trie(const std::string& text_path,
    const std::string& sa_path, const std::string& lcp_path,
    const GlobalIndex max_query_length) : manager_(
      dpt::mpi::template distribute_file<Alphabet, GlobalIndex, LocalIndex>(
      text_path, max_query_length + GlobalIndex(10))), text_path_(text_path),
      sa_path_(sa_path), lcp_path_(lcp_path),
      max_query_length_(max_query_length) { }

  template <template <typename, typename, typename> class GlobalCommunication,
            template <typename, typename, typename> class LocalCommunication>
  void construct() {
    construct_local_trie<LocalCommunication>(sa_path_, lcp_path_,
      max_query_length_);
    std::vector<GlobalIndex> global_sa;
    std::vector<GlobalIndex> global_lcp;
    auto local_global = local_trie_.global_sa_and_lcp();
    std::tie(global_sa, global_lcp) =
      manager_.distribute_global_sa_and_lcp(local_global);
    construct_global_trie<GlobalCommunication>(
      global_sa, global_lcp, max_query_length_);
  }

  template <template <typename, typename, typename> class Communication>
  auto existential_batched(q_list&& queries) {
    std::vector<size_t> hist(env_.size(), 0);
    std::vector<size_t> hist_length(env_.size(), 0);
    std::vector<int32_t> target_pes(queries.size());
    for (size_t i = 0; i < queries.size(); ++i) {
      const auto result = global_trie_.first_occurrence(queries[i]);
      if (result.state == search_state::MATCH) {
        target_pes[i] = (result.position >> 1);
        ++hist[target_pes[i]];
        hist_length[target_pes[i]] += queries[i].length;
      } else {
        target_pes[i] = -1;
      }
    }
    std::vector<size_t> displ(env_.size(), 0);
    std::vector<size_t> displ_length(env_.size(), 0);
    for (int32_t i = 1; i < env_.size(); ++i) {
      displ[i] = displ[i - 1] + hist[i - 1];
      displ_length[i] = displ_length[i - 1] + hist_length[i - 1];
    }
    std::vector<Alphabet> queries_to_distribute(
      displ_length.back() + hist_length.back());
    std::vector<LocalIndex> lengths_to_distribute(queries.size());
    for (size_t i = 0; i < queries.size(); ++i) {
      if (target_pes[i] >= 0) {
        std::copy_n(queries[i].query, queries[i].length, 
          queries_to_distribute.begin() + displ_length[target_pes[i]]);
        displ_length[target_pes[i]] += queries[i].length;
        lengths_to_distribute[displ[target_pes[i]]++] = queries[i].length;
      }
    }
    q_list rec_queries = manager_.template distribute_queries<Communication>(
      queries_to_distribute, lengths_to_distribute, hist_length, hist);
    return local_trie_.template existential_batched<Communication>(
      std::move(rec_queries), manager_);
  }

  template <template <typename, typename, typename> class Communication>
  auto counting_batched(q_list&& queries) {
    std::vector<size_t> hist(env_.size(), 0);
    std::vector<size_t> hist_length(env_.size(), 0);
    std::vector<std::pair<int32_t, int32_t>> target_pes(queries.size());
    for (size_t i = 0; i < queries.size(); ++i) {
      const auto result = global_trie_.first_and_last_occurrence(queries[i]);
      if (result.state == search_state::MATCH) {
        target_pes[i] = std::make_pair(result.left_position >> 1,
          result.right_position >> 1);
        ++hist[target_pes[i].first];
        hist_length[target_pes[i].first] += queries[i].length;
        if (target_pes[i].first != target_pes[i].second) {
          ++hist[target_pes[i].second];
          hist_length[target_pes[i].second] += queries[i].length;
        }
      } else {
        target_pes[i] = std::make_pair(-1, -1);
      }
    }
    std::vector<size_t> displ(env_.size(), 0);
    std::vector<size_t> displ_length(env_.size(), 0);
    for (int32_t i = 1; i < env_.size(); ++i) {
      displ[i] = displ[i - 1] + hist[i - 1];
      displ_length[i] = displ_length[i - 1] + hist_length[i - 1];
    }
    std::vector<Alphabet> queries_to_distribute(
      displ_length.back() + hist_length.back());
    std::vector<LocalIndex> lengths_to_distribute(displ.back() + hist.back());
    for (size_t i = 0; i < queries.size(); ++i) {
      if (target_pes[i].first >= 0) {
        std::copy_n(queries[i].query, queries[i].length, 
          queries_to_distribute.begin() + displ_length[target_pes[i].first]);
        displ_length[target_pes[i].first] += queries[i].length;
        lengths_to_distribute[displ[target_pes[i].first]++] = queries[i].length;
        if (target_pes[i].first != target_pes[i].second) {
          std::copy_n(queries[i].query, queries[i].length, 
            queries_to_distribute.begin() + displ_length[target_pes[i].second]);
          displ_length[target_pes[i].second] += queries[i].length;
          lengths_to_distribute[displ[target_pes[i].second]++] = queries[i].length;
        }
      }
    }
    q_list rec_queries = manager_.template distribute_queries<Communication>(
      queries_to_distribute, lengths_to_distribute, hist_length, hist);
    return local_trie_.template counting_batched<Communication>(
      std::move(rec_queries), manager_);
  }

  template <template <typename, typename, typename> class Communication>
  auto enumeration_batched(q_list&& queries) {
    std::vector<size_t> hist(env_.size(), 0);
    std::vector<size_t> hist_length(env_.size(), 0);
    std::vector<std::pair<int32_t, int32_t>> target_pes(queries.size());
    for (size_t i = 0; i < queries.size(); ++i) {
      const auto result = global_trie_.first_and_last_occurrence(queries[i]);
      if (result.state == search_state::MATCH) {
        target_pes[i] = std::make_pair(result.left_position >> 1,
          result.right_position >> 1);
        ++hist[target_pes[i].first];
        hist_length[target_pes[i].first] += queries[i].length;
        if (target_pes[i].first != target_pes[i].second) {
          ++hist[target_pes[i].second];
          hist_length[target_pes[i].second] += queries[i].length;
        }
      } else {
        target_pes[i] = std::make_pair(-1, -1);
      }
    }
    std::vector<size_t> displ(env_.size(), 0);
    std::vector<size_t> displ_length(env_.size(), 0);
    for (int32_t i = 1; i < env_.size(); ++i) {
      displ[i] = displ[i - 1] + hist[i - 1];
      displ_length[i] = displ_length[i - 1] + hist_length[i - 1];
    }
    std::vector<Alphabet> queries_to_distribute(
      displ_length.back() + hist_length.back());
    std::vector<LocalIndex> lengths_to_distribute(queries.size());
    for (size_t i = 0; i < queries.size(); ++i) {
      if (target_pes[i].first >= 0) {
        std::copy_n(queries[i].query, queries[i].length, 
          queries_to_distribute.begin() + displ_length[target_pes[i].first]);
        displ_length[target_pes[i].first] += queries[i].length;
        lengths_to_distribute[displ[target_pes[i].first]++] = queries[i].length;
        if (target_pes[i].first != target_pes[i].second) {
          std::copy_n(queries[i].query, queries[i].length, 
            queries_to_distribute.begin() + displ_length[target_pes[i].second]);
          displ_length[target_pes[i].second] += queries[i].length;
          lengths_to_distribute[displ[target_pes[i].second]++] = queries[i].length;
        }
      }
    }
    q_list rec_queries = manager_.template distribute_queries<Communication>(
      queries_to_distribute, lengths_to_distribute, hist_length, hist);
    return local_trie_.template enumeration_batched<Communication>(
      std::move(rec_queries), manager_);
  }

private:
  template <template <typename, typename, typename> class Communication>
  inline void construct_local_trie(const std::string& sa_path,
    const std::string& lcp_path, const GlobalIndex max_query_length) {
    auto local_sa =
      dpt::mpi::distribute_file<GlobalIndex, GlobalIndex, LocalIndex>(
        sa_path, 0);
    auto local_lcp =
      dpt::mpi::distribute_file<GlobalIndex, GlobalIndex, LocalIndex>(
        lcp_path, 0);
    local_trie_.template construct<Communication>(
      std::move(local_sa), std::move(local_lcp), manager_, max_query_length);
  }

  template <template <typename, typename, typename> class Communication>
  inline void construct_global_trie(const std::vector<GlobalIndex>& global_sa,
    const std::vector<GlobalIndex>& global_lcp,
    const GlobalIndex max_query_length) {
    global_trie_.template construct<Communication>(
      global_sa, global_lcp, manager_, max_query_length);
  }

private:
  dpt::mpi::environment env_;
  manager manager_;
  com_trie global_trie_;
  pat_trie local_trie_;

  std::string text_path_;
  std::string sa_path_;
  std::string lcp_path_;
  GlobalIndex max_query_length_;
}; // class distributed_patricia_trie

} // namespace tree
} // namespace dpt

/******************************************************************************/
