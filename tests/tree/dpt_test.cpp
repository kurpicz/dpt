/*******************************************************************************
 * tests/tree/dpt_test.cpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <stdlib.h>
#include <ctime>
#include <vector>

#include "com/local.hpp"
#include "com/collective.hpp"
#include "com/local.hpp"
#include "com/manager.hpp"
#include "mpi/io.hpp"
#include "query/query_list.hpp"
#include "tree/compact_trie_pointer.hpp"
#include "tree/distributed_patricia_trie.hpp"
#include "tree/patricia_trie_pointer.hpp"

using dp_trie =  dpt::tree::distributed_patricia_trie<char, size_t, size_t,
  dpt::tree::compact_trie_pointer, dpt::tree::patricia_trie_pointer>;
using q_list = dpt::query::query_list<char, size_t, size_t>;

class dpt_test : public ::testing::Test {
protected:
  virtual void SetUp() {
    dpt_ = dp_trie("test_data/the_three_brothers.txt",
      "test_data/the_three_brothers_size_t_sa",
      "test_data/the_three_brothers_size_t_lcp", 335);

    dpt_.construct<dpt::com::collective_communication,
      dpt::com::collective_communication>();

    std::ifstream stream("test_data/the_three_brothers.txt", std::ios::in);
    stream.seekg(0, std::ios::end);
    uint64_t size = stream.tellg();
    stream.seekg(0);
    std::vector<char> text(size);
    stream.read(reinterpret_cast<char*>(text.data()), size);
    stream.close();
    global_text_ = std::string(text.begin(), text.end());
  }

  virtual void TearDown() { }

public:
  q_list gen_random_existing_queries(const size_t nr_queries,
    const size_t max_length) {
    std::srand(std::time(0));
    std::vector<char> queries;
    std::vector<size_t> query_lengths;
    for (size_t i = 0; i < nr_queries; ++i) {
      size_t query_length = (std::rand() % max_length) + 1;
      size_t query_pos = std::rand() % 
        (global_text_.size() - (query_length + 1));
      std::copy_n(global_text_.begin() + query_pos, query_length,
        std::back_inserter(queries));
      query_lengths.emplace_back(query_length);
    }
    return q_list(std::move(queries), std::move(query_lengths));
  }

public:
  dp_trie dpt_;
  std::string global_text_;
}; // class dpt_test

TEST_F(dpt_test, existential_batched_existing) {
  q_list queries = gen_random_existing_queries(2000, 10);
  auto results = dpt_.existential_batched<dpt::com::collective_communication>(
    std::move(queries));
  for (const auto& result : results) {
    ASSERT_EQ(dpt::tree::search_state::MATCH, result);
  }
}

TEST_F(dpt_test, first_occurrence_batched_non_existing) {
  std::vector<char> queries_txt = { 'x', 'p', 'l', 'a', 'r', 'e', 'h', 'e', 'r',
    'e', ' ', 'a', 'x', 'a', 'r', 'g', 'x', 'a', 'r', 'g', 'f', 'l', 'o', 'w',
    'e', 'r', 's', ','};
  std::vector<size_t> query_lengths = { 6, 6, 4, 4, 8 };
  q_list queries(std::move(queries_txt), std::move(query_lengths));
  auto results = dpt_.existential_batched<dpt::com::collective_communication>(
    std::move(queries));
  for (const auto& result : results) {
    ASSERT_NE(dpt::tree::search_state::MATCH, result);
  }
}

// TEST_F(dpt_test, counting_batched_existing) {
//   q_list queries = gen_random_existing_queries(2000, 10);
//   auto results = dpt_.counting_batched<dpt::com::collective_communication>(
//     std::move(queries));
//   for (const auto result : results) {
//     ASSERT_GE(result, static_cast<size_t>(0));
//   }
// }

// TEST_F(dpt_test, enumeration_batched_existing) {
//   q_list queries = gen_random_existing_queries(2000, 10);
//   auto results = dpt_.enumeration_batched<dpt::com::collective_communication>(
//     std::move(queries));
//   for (const auto result : results.second) {
//     ASSERT_GE(result, static_cast<size_t>(0));
//   }
// }

/******************************************************************************/
