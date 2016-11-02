/*******************************************************************************
 * dpt/tree/compact_trie_pointer.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include "com/manager.hpp"
#include "query/query_view.hpp"
#include "tree/pointer_node.hpp"
#include "tree/search_result.hpp"
#include "util/partition.hpp"

namespace dpt {
namespace tree {

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
class compact_trie_pointer {

  using node = trie_node<Alphabet, LocalIndex>;
  using manager = dpt::com::manager<Alphabet, GlobalIndex, LocalIndex>;
  using partition = dpt::util::partition<GlobalIndex, GlobalIndex, LocalIndex>;
  using query_view = dpt::query::query_view<Alphabet, LocalIndex>;


public:
  compact_trie_pointer() { }

  inline node root() const {
    return node { 0, root_.node_begin };
  }

  inline Alphabet out_degree(const node& n) const {
    return nodes_[n.position].out_degree;
  }

  inline bool leaf(const node& n) const {
    return (out_degree(n) == 0);
  }

  template <template <typename, typename, typename> class Communication>
  void construct(const std::vector<GlobalIndex>& local_sa,
    const std::vector<GlobalIndex>& local_lcp,
    manager& manager, const GlobalIndex max_lcp) {

    auto sa_iterator = local_sa.begin();
    auto lcp_iterator = local_lcp.begin();
    const auto sa_end = local_sa.end();
    const auto lcp_end = local_lcp.end();

    GlobalIndex prev_sa = 0;
    GlobalIndex cur_sa = *sa_iterator;
    GlobalIndex prev_lcp = 0;
    GlobalIndex cur_lcp = *lcp_iterator;

    update_check_iterators(prev_sa, cur_sa, prev_lcp, cur_lcp,
      sa_iterator, sa_end, lcp_iterator, lcp_end);

    struct stack_element {
      LocalIndex lcp;
      Alphabet nr_children;
      GlobalIndex node_buffer_pos;
      GlobalIndex text_buffer_pos;

      stack_element () = default;
      stack_element (const LocalIndex _lcp, const Alphabet nr_chld,
                     const GlobalIndex node_pos, const GlobalIndex txt_pos)
        : lcp(_lcp), nr_children(nr_chld), node_buffer_pos(node_pos),
          text_buffer_pos(txt_pos) { }
    } __attribute__ ((packed));

    std::vector<node> node_buffer;
    std::vector<stack_element> node_stack;
    std::vector<GlobalIndex> text_pos_buffer;
    std::vector<GlobalIndex> text_length_buffer;
    std::vector<GlobalIndex> requests;
    std::vector<LocalIndex> lengths;
    
    node_buffer.emplace_back(node { 0, 0, 0 });
    node_stack.emplace_back(cur_lcp, 2, 0, 0);
    text_pos_buffer.emplace_back(prev_sa + cur_lcp);
    text_pos_buffer.emplace_back(cur_sa + cur_lcp);
    text_length_buffer.emplace_back(1);
    text_length_buffer.emplace_back(1);

    size_t cur_leaf_pos = 1;
    while(true) {
      if (update_check_iterators(prev_sa, cur_sa, prev_lcp, cur_lcp,
        sa_iterator, sa_end, lcp_iterator, lcp_end)) {
        break;
      }
      node_buffer.emplace_back(0, 0, cur_leaf_pos++);
      GlobalIndex last_node_lcp = 0;
      while (node_stack.size() > 0 && 
        (last_node_lcp = node_stack.back().lcp) > cur_lcp) {
        const auto tmp = node_stack.back();
        const LocalIndex old_req_size = requests.size();
        std::copy_n(node_buffer.begin() + tmp.node_buffer_pos,
          tmp.nr_children, std::back_inserter(nodes_));
        std::copy_n(text_pos_buffer.begin() + tmp.text_buffer_pos,
          tmp.nr_children, std::back_inserter(requests));
        std::copy_n(text_length_buffer.begin() + tmp.text_buffer_pos,
          tmp.nr_children, std::back_inserter(lengths));
        node_buffer.resize(node_buffer.size() - tmp.nr_children);
        node_buffer.emplace_back(tmp.lcp, tmp.nr_children, old_req_size);
        text_pos_buffer.resize(text_pos_buffer.size() - tmp.nr_children);
        text_length_buffer.resize(
          text_length_buffer.size() - tmp.nr_children);
        node_stack.pop_back();
      }
      // Now, there are three cases:
      // 1. We have emptied the stack, therefore we add a new root to the trie
      // 2. We have found a node with the same string depth and append a child
      // 3. We branch below a node, thus we consider the right children of
      //    that node as the (new and only) left children of the new node
      if (node_stack.size() == 0) {
        node_stack.emplace_back(cur_lcp, 2, 0, 0);
        text_pos_buffer.emplace_back(prev_sa + cur_lcp);
        text_pos_buffer.emplace_back(cur_sa + cur_lcp);
        text_length_buffer.emplace_back(last_node_lcp - cur_lcp);
        text_length_buffer.emplace_back(1);
      } else if (node_stack.back().lcp == cur_lcp) {
        ++(node_stack.back().nr_children);
        text_pos_buffer.emplace_back(cur_sa + cur_lcp);
        text_length_buffer.emplace_back(1);
      } else {
        node_stack.emplace_back(cur_lcp, 2, node_buffer.size() - 1, text_pos_buffer.size());
        text_pos_buffer.emplace_back(prev_sa + cur_lcp);
        text_pos_buffer.emplace_back(cur_sa + cur_lcp);
        GlobalIndex old_length = text_length_buffer.back();
        text_length_buffer.back() = cur_lcp - GlobalIndex(last_node_lcp);
        text_length_buffer.emplace_back(
          (old_length == GlobalIndex(1) ? GlobalIndex(1) : old_length + last_node_lcp - cur_lcp));
        text_length_buffer.emplace_back(1);
      }        
    }
    node_buffer.emplace_back(0, 0, cur_leaf_pos++);
    while (node_stack.size() > 0) {
      const auto tmp = node_stack.back();
      const LocalIndex old_req_size = requests.size();
      std::copy_n(node_buffer.begin() + tmp.node_buffer_pos,
        tmp.nr_children, std::back_inserter(nodes_));
      std::copy_n(text_pos_buffer.begin() + tmp.text_buffer_pos,
        tmp.nr_children, std::back_inserter(requests));
      std::copy_n(text_length_buffer.begin() + tmp.text_buffer_pos,
            tmp.nr_children, std::back_inserter(lengths));
      node_buffer.resize(node_buffer.size() - tmp.nr_children);
      root_ = node(tmp.lcp, tmp.nr_children, old_req_size);
      node_buffer.emplace_back(root_);
      text_pos_buffer.resize(text_pos_buffer.size() - tmp.nr_children);
      text_length_buffer.resize(text_length_buffer.size() - tmp.nr_children);
      node_stack.pop_back();
    }

    std::vector<GlobalIndex>().swap(text_pos_buffer);
    std::vector<node>().swap(node_buffer);
    labels_starting_positions_.emplace_back(0);
    for (auto& length : lengths) {
      length = std::min<LocalIndex>(max_lcp, length);
      labels_starting_positions_.emplace_back(
        length + labels_starting_positions_.back() - 1);
    }
    std::tie(first_characters_, labels_) = 
      manager.template
        request_substrings_head<Communication>(requests, lengths);
  }

  inline search_result<uint32_t> first_occurrence(const query_view& query) const {
    auto cur_node = root_;
    while (cur_node.out_degree > 0 && cur_node.string_depth < query.length) {
      Alphabet child_pos = 0;
      while (child_pos < cur_node.out_degree &&
        first_characters_[cur_node.edge_begin + child_pos] <
        query[cur_node.string_depth]) {
        ++child_pos;
      }
      if (child_pos == cur_node.out_degree) {
        return { search_state::RIGHT_OF, rightmost_leaf(
            nodes_[cur_node.edge_begin + child_pos - 1]).edge_begin };
      }
      else if (first_characters_[cur_node.edge_begin + child_pos] >
        query[cur_node.string_depth]) {
        return { search_state::LEFT_OF,
          leftmost_leaf(
            nodes_[cur_node.edge_begin + child_pos]).edge_begin };
      } else /*(first_chrs[cur_node.edge_begin + child_pos] == query[q_pos])*/ {
        LocalIndex q_pos = cur_node.string_depth + 1;
        LocalIndex e_pos = 
          labels_starting_positions_[cur_node.edge_begin + child_pos];
        while (q_pos < query.length && e_pos <
          labels_starting_positions_[cur_node.edge_begin + child_pos + 1] &&
          labels_[e_pos] == query[q_pos]) {
          ++e_pos;
          ++q_pos;
        }
        if (e_pos ==
          labels_starting_positions_[cur_node.edge_begin + child_pos + 1]) {
          cur_node = nodes_[cur_node.edge_begin + child_pos];
        } else if (q_pos == query.length) {
          return { search_state::MATCH,
            leftmost_leaf(nodes_[cur_node.edge_begin + child_pos]).edge_begin };
        } else if (e_pos >
          labels_starting_positions_[cur_node.edge_begin + child_pos + 1]) {
          return { search_state::LEFT_OF,
            leftmost_leaf(nodes_[cur_node.edge_begin + child_pos]).edge_begin };
        } else if (e_pos <
          labels_starting_positions_[cur_node.edge_begin + child_pos + 1]) {
          return { search_state::RIGHT_OF,
            rightmost_leaf(
              nodes_[cur_node.edge_begin + child_pos]).edge_begin };
        }
      }
    }
    return { search_state::MATCH, cur_node.edge_begin };
  }

  inline search_result_pair<uint32_t> first_and_last_occurrence(
    const query_view& query) const {
    auto cur_node = root_;
    while (cur_node.out_degree > 0 && cur_node.string_depth < query.length) {
      Alphabet child_pos = 0;
      while (child_pos < cur_node.out_degree &&
        first_characters_[cur_node.edge_begin + child_pos] <
        query[cur_node.string_depth]) {
        ++child_pos;
      }
      if (child_pos == cur_node.out_degree) {
        const auto rightmost = rightmost_leaf(nodes_[
          cur_node.edge_begin + child_pos - 1]).edge_begin;
        return { search_state::RIGHT_OF, rightmost, rightmost };
      }
      else if (first_characters_[cur_node.edge_begin + child_pos] >
        query[cur_node.string_depth]) {
        const auto leftmost = leftmost_leaf(nodes_[
          cur_node.edge_begin + child_pos]).edge_begin;
        return { search_state::LEFT_OF, leftmost, leftmost };
      } else /*(first_chrs[cur_node.edge_begin + child_pos] == query[q_pos])*/ {
        LocalIndex q_pos = cur_node.string_depth + 1;
        LocalIndex e_pos =
          labels_starting_positions_[cur_node.edge_begin + child_pos];
        while (q_pos < query.length && e_pos <
          labels_starting_positions_[cur_node.edge_begin + child_pos + 1] &&
          labels_[e_pos] == query[q_pos]) {
          ++e_pos;
          ++q_pos;
        }
        if (e_pos ==
          labels_starting_positions_[cur_node.edge_begin + child_pos + 1]) {
          cur_node = nodes_[cur_node.edge_begin + child_pos];
        } else if (q_pos == query.length) {
          return { search_state::MATCH,
            leftmost_leaf(nodes_[cur_node.edge_begin + child_pos]).edge_begin,
            rightmost_leaf(nodes_[
              cur_node.edge_begin + child_pos]).edge_begin };
        } else if (e_pos >
          labels_starting_positions_[cur_node.edge_begin + child_pos + 1]) {
          const auto leftmost = leftmost_leaf(nodes_[
          cur_node.edge_begin + child_pos]).edge_begin;
          return { search_state::LEFT_OF, leftmost, leftmost };
        } else if (e_pos <
          labels_starting_positions_[cur_node.edge_begin + child_pos + 1]) {
          const auto rightmost = rightmost_leaf(nodes_[
          cur_node.edge_begin + child_pos - 1]).edge_begin;
          return { search_state::RIGHT_OF, rightmost, rightmost };
        }
      }
    }
    if (cur_node.out_degree == 0) {
      return{ search_state::MATCH, cur_node.edge_begin, cur_node.edge_begin };
    } else {
      return{ search_state::MATCH, leftmost_leaf(cur_node).edge_begin,
        rightmost_leaf(cur_node).edge_begin };
    }
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

  inline int32_t compare_edge(LocalIndex label_begin, LocalIndex& query_begin,
    const LocalIndex label_length, const query_view& query) const {
    const LocalIndex comparison_length = std::min(
      label_length, query.length - query_begin);
    size_t cur_char = 0;
    while (cur_char + 1 < comparison_length && 
      labels_[label_begin] == query[query_begin]) {
      ++cur_char;
      ++label_begin;
      ++query_begin;
    }
    return 0;
  }

  inline node leftmost_leaf(node cur_edge) const {
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
  std::vector<Alphabet> first_characters_;
  std::vector<Alphabet> labels_;
  std::vector<LocalIndex> labels_starting_positions_;
  std::vector<node> nodes_;
  node root_;

}; // class compact_trie_pointer

} // namespace tree
} // namespace dpt

/******************************************************************************/
