/*******************************************************************************
 * dpt/tree/patricia_trie_pointer.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once
#ifndef DPT_TREE_PATRICIA_TRIE_POINTER_HEADER
#define DPT_TREE_PATRICIA_TRIE_POINTER_HEADER

#include "com/manager.hpp"
#include "query/query_list.hpp"
#include "tree/pointer_node.hpp"
#include "tree/search_result.hpp"
#include "util/partition.hpp"

#include "mpi/environment.hpp"

namespace dpt {
namespace tree {

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
class patricia_trie_pointer {

  using node = trie_node<Alphabet, LocalIndex>;
  using q_list = dpt::query::query_list<Alphabet, GlobalIndex, LocalIndex>;
  using q_view = dpt::query::query_view<Alphabet, LocalIndex>;

public:
  patricia_trie_pointer() { }

  template <template <typename, typename, typename> class Communication>
  void construct(
    const dpt::util::partition<GlobalIndex, GlobalIndex, LocalIndex>& local_sa,
    const dpt::util::partition<GlobalIndex, GlobalIndex, LocalIndex>& local_lcp,
    dpt::com::manager<Alphabet, GlobalIndex, LocalIndex>& manager,
    const GlobalIndex max_lcp) {

    auto sa_iterator = local_sa.data_begin();
    auto lcp_iterator = local_lcp.data_begin();
    const auto sa_end = local_sa.data_end();
    const auto lcp_end = local_lcp.data_end();

    GlobalIndex prev_sa = 0;
    GlobalIndex cur_sa = *sa_iterator;
    GlobalIndex prev_lcp = 0;
    GlobalIndex cur_lcp = *lcp_iterator;

    bool finished = update_check_iterators(prev_sa, cur_sa, prev_lcp, cur_lcp,
      sa_iterator, sa_end, lcp_iterator, lcp_end);

    global_sa_[0] = prev_sa;
    global_lcp_[0] = prev_lcp;
    global_lcp_[1] = cur_lcp;

    struct stack_element {
      GlobalIndex lcp;
      Alphabet nr_children;
      GlobalIndex node_buffer_pos;
      GlobalIndex text_buffer_pos;
    } __attribute__ ((packed));

    std::vector<node> node_buffer;
    std::vector<stack_element> node_stack;
    std::vector<GlobalIndex> text_pos_buffer;
    std::vector<GlobalIndex> requests;
    
    node_buffer.emplace_back(0, 0, 0);
    node_stack.emplace_back(stack_element { cur_lcp, 2, 0, 0 });
    text_pos_buffer.emplace_back(prev_sa + cur_lcp);
    text_pos_buffer.emplace_back(cur_sa + cur_lcp);

    LocalIndex cur_sa_pos = 1;
    LocalIndex prev_leaf_pos = 0;
    LocalIndex cur_leaf_pos = 1;
    while(!finished) {
      finished = update_check_iterators(prev_sa, cur_sa, prev_lcp, cur_lcp,
        sa_iterator, sa_end, lcp_iterator, lcp_end);
      ++cur_sa_pos;
      // Entry is considered (its LCP value is not longer than the threshold)
      if (cur_lcp < max_lcp) {
        global_lcp_[1] = std::min(global_lcp_[1], cur_lcp);
        prev_leaf_pos = cur_leaf_pos;
        cur_leaf_pos = cur_sa_pos;
        // Insert the (old) leaf.
        node_buffer.emplace_back(node { 0, 0, prev_leaf_pos });
        while (node_stack.size() > 0 && 
          node_stack.back().lcp > cur_lcp) {
          const auto tmp = node_stack.back();
          const LocalIndex old_req_size = requests.size();
          std::copy_n(node_buffer.begin() + tmp.node_buffer_pos ,
            tmp.nr_children, std::back_inserter(nodes_));
          std::copy_n(text_pos_buffer.begin() + tmp.text_buffer_pos,
            tmp.nr_children, std::back_inserter(requests));
          node_buffer.resize(node_buffer.size() - tmp.nr_children);
          node_buffer.emplace_back(tmp.lcp, tmp.nr_children, old_req_size);
          text_pos_buffer.resize(text_pos_buffer.size() - tmp.nr_children);
          node_stack.pop_back();
        }
        // Now, there are three cases:
        // 1. We have emptied the stack, therefore we add a new root to the trie
        // 2. We have found a node with the same string depth and append a child
        // 3. We branch below a node, thus we consider the right children of
        //    that node as the (new and only) left children of the new node
        if (node_stack.size() == 0) {
          node_stack.emplace_back(stack_element { cur_lcp, 2, 0, 0});
          text_pos_buffer.emplace_back(prev_sa + cur_lcp);
          text_pos_buffer.emplace_back(cur_sa + cur_lcp);
        } else if (node_stack.back().lcp == cur_lcp) {
          ++(node_stack.back().nr_children);
          text_pos_buffer.emplace_back(cur_sa + cur_lcp);
        } else {
          node_stack.emplace_back( stack_element { cur_lcp, 2,
            node_buffer.size() - 1, text_pos_buffer.size() });
          text_pos_buffer.emplace_back(prev_sa + cur_lcp);
          text_pos_buffer.emplace_back(cur_sa + cur_lcp);
        }        
      }
    }
    global_sa_[1] = cur_sa;
    node_buffer.emplace_back(node { 0, 0, cur_leaf_pos });
    while (node_stack.size() > 0) {
      const auto& tmp = node_stack.back();
      const LocalIndex old_req_size = requests.size();
      std::copy_n(node_buffer.begin() + tmp.node_buffer_pos,
        tmp.nr_children, std::back_inserter(nodes_));
      std::copy_n(text_pos_buffer.begin() + tmp.text_buffer_pos,
        tmp.nr_children, std::back_inserter(requests));
      node_buffer.resize(node_buffer.size() - tmp.nr_children);
      root_ = node(tmp.lcp, tmp.nr_children, old_req_size);
      node_buffer.emplace_back(root_);
      text_pos_buffer.resize(text_pos_buffer.size() - tmp.nr_children);
      node_stack.pop_back();
    }
    std::vector<GlobalIndex>().swap(text_pos_buffer);
    std::vector<node>().swap(node_buffer);
    labels_ = manager.template request_characters<Communication>(requests);
  }

  auto global_sa_and_lcp() const {
    return std::make_pair(global_sa_, global_lcp_);
  }

  template <template <typename, typename, typename> class Communication>
  std::vector<search_state> existential_batched(q_list&& rec_queries,
    dpt::com::manager<Alphabet, GlobalIndex, LocalIndex>& manager,
    const dpt::util::partition<GlobalIndex, GlobalIndex, LocalIndex>& local_sa)
    const {

    dpt::mpi::environment env;

    std::vector<GlobalIndex> req_positions;
    std::vector<LocalIndex> req_lengths;
    std::vector<search_state> states;
    for (const auto& query : rec_queries) {
      auto bs_res = blind_search_first_sa_position(query);
      states.emplace_back(bs_res.state);
      if (bs_res.state == search_state::NOT_YET_FOUND) {
        req_positions.emplace_back(local_sa[bs_res.position]);
        req_lengths.emplace_back(query.length);
      }
    }

    auto req_substrings =
      manager.template request_substrings<Communication>(
        req_positions, req_lengths);
    for (size_t i = 0, cur_substr_pos = 0; i < rec_queries.size(); ++i) {
      if (states[i] == search_state::NOT_YET_FOUND) {
        size_t pos = 0;
        while (pos < rec_queries[i].length && 
            req_substrings[cur_substr_pos + pos] == rec_queries[i][pos]) {
          ++pos;
        }
        cur_substr_pos += rec_queries[i].length;
        states[i] = (pos == rec_queries[i].length) ?
          search_state::MATCH : search_state::NO_MATCH;
      }
    }
    return states;
  }

  template <template <typename, typename, typename> class Communication>
  std::vector<LocalIndex> counting_batched(q_list&& rec_queries,
    dpt::com::manager<Alphabet, GlobalIndex, LocalIndex>& manager,
    const dpt::util::partition<GlobalIndex, GlobalIndex, LocalIndex>& local_sa)
    const {

    std::vector<GlobalIndex> req_positions;
    std::vector<LocalIndex> req_lengths;
    std::vector<search_state> states;
    std::vector<search_result<LocalIndex>> search_results;
    for (const auto& query : rec_queries) {
      search_results.emplace_back(blind_search_node_position(query));
      if (search_results.back().state == search_state::NOT_YET_FOUND) {
        req_positions.emplace_back(local_sa[leftmoste_leaf(nodes_[search_results.back().position]).edge_begin]);
        req_lengths.emplace_back(query.length);
      }
    }
    auto req_substrings = manager.template request_substrings<Communication>(
      req_positions, req_lengths);
    std::vector<LocalIndex> nr_occurrences(rec_queries.size(), 0);
    for (size_t i = 0, cur_substr_pos = 0; i < rec_queries.size(); ++i) {
      if (search_results[i].state == search_state::NOT_YET_FOUND) {
        size_t pos = 1;
        while (pos < rec_queries[i].length && 
          req_substrings[cur_substr_pos + pos] == rec_queries[i][pos]) {
          ++pos;
        }
        cur_substr_pos += rec_queries[i].length;
        if (pos == rec_queries[i].length) {
          nr_occurrences[i] =
            rightmost_leaf(nodes_[search_results[i].position]).edge_begin -
            leftmoste_leaf(nodes_[search_results[i].position]).edge_begin + 1;
        }
      }
    }
    return nr_occurrences;
  }

  template <template <typename, typename, typename> class Communication>
  std::pair<std::vector<GlobalIndex>, std::vector<LocalIndex>>
    enumeration_batched(q_list&& rec_queries,
      dpt::com::manager<Alphabet, GlobalIndex, LocalIndex>& manager,
      const dpt::util::partition<GlobalIndex, GlobalIndex,
      LocalIndex>& local_sa) const {

    std::vector<GlobalIndex> req_positions;
    std::vector<LocalIndex> req_lengths;
    std::vector<search_state> states;
    std::vector<search_result<LocalIndex>> search_results;
    for (const auto& query : rec_queries) {
      search_results.emplace_back(blind_search_node_position(query));
      if (search_results.back().state == search_state::NOT_YET_FOUND) {
        req_positions.emplace_back(
          local_sa[leftmoste_leaf(nodes_[search_results.back().position]).edge_begin]);
        req_lengths.emplace_back(query.length);
      }
    }
    auto req_substrings = manager.template request_substrings<Communication>(
      req_positions, req_lengths);
    std::vector<GlobalIndex> intervals;
    std::vector<LocalIndex> interval_sizes(rec_queries.size(), 0);
    for (size_t i = 0, cur_substr_pos = 0; i < rec_queries.size(); ++i) {
      if (search_results[i].state == search_state::NOT_YET_FOUND) {
        size_t pos = 0;
        while (pos < rec_queries[i].length && 
            req_substrings[cur_substr_pos + pos] == rec_queries[i][pos]) {
          ++pos;
        }
        cur_substr_pos += rec_queries[i].length;
        if (pos == rec_queries[i].length) {
          const auto leftmost =
            leftmoste_leaf(nodes_[search_results[i].position]).edge_begin;
          interval_sizes[i] =
          rightmost_leaf(nodes_[search_results[i].position]).edge_begin -
          leftmost + 1;
          std::copy_n(local_sa.const_local_data()->begin() +
            leftmoste_leaf(nodes_[search_results[i].position]).edge_begin,
            interval_sizes[i], std::back_inserter(intervals));
        }
      }
    }
    return std::make_pair(intervals, interval_sizes);
  }

  const node root() {
    return root_;
  }

  const node get_node(const size_t pos) {
    return nodes_[pos];
  }

  const Alphabet get_label(const size_t pos) {
    return labels_[pos];
  }

  size_t number_of_nodes() const {
    return nodes_.size();
  }

private:
  template <typename Iterator>
  inline bool update_check_iterators(GlobalIndex& prev_sa, GlobalIndex& cur_sa,
    GlobalIndex& prev_lcp, GlobalIndex& cur_lcp, Iterator& sa_iterator,
    const Iterator& sa_end, Iterator& lcp_iterator, const Iterator& lcp_end) {
    if (++sa_iterator != sa_end && ++lcp_iterator != lcp_end) {
      prev_sa = cur_sa;
      cur_sa  = *sa_iterator;
      prev_lcp = cur_lcp;
      cur_lcp  = *lcp_iterator;
      return false;
    }
    return true;
  }

  search_result<LocalIndex> blind_search_first_sa_position(
    const q_view& q) const {
    node cur_node = root_;
    while (cur_node.string_depth < q.length && cur_node.out_degree > 0) {
      Alphabet child_nr = 0;
      while (child_nr < cur_node.out_degree &&
        labels_[cur_node.edge_begin + child_nr] < q[cur_node.string_depth]) {
        ++child_nr;
      }
      if (child_nr == cur_node.out_degree ||
        labels_[cur_node.edge_begin + child_nr] !=
        q[cur_node.string_depth]) {
        return { search_state::NO_MATCH, 0 };
      } else {
        cur_node = nodes_[cur_node.edge_begin + child_nr];
      }
    }
    return { search_state::NOT_YET_FOUND,
      leftmoste_leaf(cur_node).edge_begin };
  }

  search_result<LocalIndex> blind_search_node_position(
    const q_view& q) const {
    node cur_node = root_;
    LocalIndex node_pos = root_.edge_begin;
    while (cur_node.string_depth < q.length && cur_node.out_degree > 0) {
      Alphabet child_nr = 0;
      while (child_nr < cur_node.out_degree &&
        labels_[cur_node.edge_begin + child_nr] < q[cur_node.string_depth]) {
        ++child_nr;
      }
      if (child_nr == cur_node.out_degree ||
        labels_[cur_node.edge_begin + child_nr] !=
        q[cur_node.string_depth]) {
        return { search_state::NO_MATCH, 0 };
      } else {
        node_pos = cur_node.edge_begin + child_nr;
        cur_node = nodes_[node_pos];
      }
    }
    return { search_state::NOT_YET_FOUND, node_pos };
  }

  inline node leftmoste_leaf(node cur_edge) const {
    while (cur_edge.out_degree > 0) {
      cur_edge = nodes_[cur_edge.edge_begin];
    }
    return cur_edge;
  }

  inline node rightmost_leaf(node cur_edge) const {
    while (cur_edge.out_degree > 0) {
      cur_edge = nodes_[cur_edge.edge_begin + cur_edge.out_degree - 1];
    }
    return cur_edge;
  }

private:
  std::vector<Alphabet> labels_;
  std::vector<node> nodes_;
  node root_;

  std::array<GlobalIndex, 2> global_sa_;
  std::array<GlobalIndex, 2> global_lcp_;
}; // class patricia_trie_pointer

} // namespace tree
} // namespace dpt

#endif // DPT_TREE_PATRICIA_TRIE_POINTER_HEADER

/******************************************************************************/
