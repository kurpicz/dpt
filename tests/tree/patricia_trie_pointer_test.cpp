/*******************************************************************************
 * tests/tree/patricia_trie_pointer_test.cpp
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

#include "com/one_sided.hpp"
#include "com/collective.hpp"
#include "com/local.hpp"
#include "com/manager.hpp"
#include "com/one_sided.hpp"
#include "mpi/environment.hpp"
#include "mpi/io.hpp"
#include "query/query_list.hpp"
#include "tree/patricia_trie_pointer.hpp"
#include "tree/search_result.hpp"

using char_partition = dpt::util::partition<char, size_t, size_t>;
using manager = dpt::com::manager<char, size_t, size_t>;
using pat_trie = dpt::tree::patricia_trie_pointer<char, size_t, size_t>;
using q_list = dpt::query::query_list<char, size_t, size_t>;
using size_t_partition = dpt::util::partition<size_t, size_t, size_t>;

class patricia_trie_pointer_test : public ::testing::Test {
protected:
  virtual void SetUp() {
    local_text_ = dpt::mpi::distribute_file<char, size_t, size_t>(
        "test_data/the_three_brothers.txt", 40);
    auto tmp_text = local_text_;
    manager_ = manager(std::move(tmp_text));
    part_sa_ = 
      dpt::mpi::distribute_file<size_t, size_t, size_t>(
        "test_data/the_three_brothers_size_t_sa", 0);
    part_lcp_ = 
      dpt::mpi::distribute_file<size_t, size_t, size_t>(
        "test_data/the_three_brothers_size_t_lcp", 0);

    std::ifstream stream("test_data/the_three_brothers.txt", std::ios::in);
    stream.seekg(0, std::ios::end);
    uint64_t size = stream.tellg();
    stream.seekg(0);
    std::vector<char> text(size);
    stream.read(reinterpret_cast<char*>(text.data()), size);
    global_text_ = std::string(text.begin(), text.end());

    if (env_.size() == 1) {
      pt_.template construct<dpt::com::collective_communication>(
        std::move(part_sa_), std::move(part_lcp_), manager_, 300);
    } else {
      pt_.template construct<dpt::com::collective_communication>(
        std::move(part_sa_), std::move(part_lcp_), manager_, 300);
    }
  }

  virtual void TearDown() { }

public:
  q_list gen_random_existing_queries(const size_t nr_queries,
    const size_t max_length) {
    std::srand(44227);
    std::vector<char> queries;
    std::vector<size_t> query_lengths;
    for (size_t i = 0; i < nr_queries; ++i) {
      size_t query_length = (std::rand() % max_length) + 1;
      const auto iter_sa = part_sa_.data_begin();
      const auto query_pos = *(iter_sa + (std::rand() % (part_sa_.local_size() - 1)));
      if (query_pos + query_length >= global_text_.size()) {
        query_length = global_text_.size() - query_pos;
      }
      std::copy_n(global_text_.begin() + query_pos, query_length,
        std::back_inserter(queries));
      query_lengths.emplace_back(query_length);
    }
    return q_list(std::move(queries), std::move(query_lengths));
  }

public:
  dpt::mpi::environment env_;
  pat_trie pt_;
  manager manager_;
  char_partition local_text_;
  size_t_partition part_sa_;
  size_t_partition part_lcp_;
  std::string global_text_;
}; // class patricia_trie_pointer_test

TEST_F(patricia_trie_pointer_test, existential_batched_existing) {
  q_list queries = gen_random_existing_queries(2000, 10);
  std::vector<dpt::tree::search_state> results;
  auto q_tmp = queries;
  if (env_.size() == 1) {
    results = pt_.existential_batched<dpt::com::local_communication>(
      std::move(queries), manager_, part_sa_);
  } else {
    results = pt_.existential_batched<dpt::com::collective_communication>(
      std::move(queries), manager_, part_sa_);
  }
  for (const auto result : results) {
    ASSERT_EQ(dpt::tree::search_state::MATCH, result);
  }
}

TEST_F(patricia_trie_pointer_test, existential_batched_non_existing) {
  std::vector<char> queries = { 'x', 'p', 'l', 'a', 'r', 'e', 'h', 'e', 'r',
    'e', ' ', 'a', 'x', 'a', 'r', 'g', 'x', 'a', 'r', 'g', 'f', 'l', 'o', 'w',
    'e', 'r', 's', ','};
  std::vector<size_t> query_lengths = { 6, 6, 4, 4, 8 };
  q_list query_list(std::move(queries), std::move(query_lengths));
  std::vector<dpt::tree::search_state> results;
  if (env_.size() == 1) {
    results = pt_.existential_batched<dpt::com::local_communication>(
      std::move(query_list), manager_, part_sa_);
  } else {
    results = pt_.existential_batched<dpt::com::collective_communication>(
      std::move(query_list), manager_, part_sa_);
  }
  for (const auto result : results) {
    ASSERT_EQ(dpt::tree::search_state::NO_MATCH, result);
  }
}

TEST_F(patricia_trie_pointer_test, counting_batched_existing) {
  q_list queries = gen_random_existing_queries(2000, 10);

  auto q_checker = queries;
  std::vector<size_t> results;
  if (env_.size() == 1) {
    results = pt_.counting_batched<dpt::com::local_communication>(
      std::move(queries), manager_, part_sa_);
  } else {
    results = pt_.counting_batched<dpt::com::collective_communication>(
      std::move(queries), manager_, part_sa_);
  }
  ASSERT_EQ(results.size(), q_checker.size());
  for (size_t i = 0; i < results.size(); ++i) {
    size_t occurrence_query = 0;
    size_t cur_text_pos = 0;
    while ((cur_text_pos = global_text_.find(q_checker[i].query, cur_text_pos,
      q_checker[i].length)) != std::string::npos) {
      ++occurrence_query;
      ++cur_text_pos;
    }
    EXPECT_EQ(occurrence_query, results[i]);
  }
}

TEST_F(patricia_trie_pointer_test, counting_batched_non_existing) {
  std::vector<char> queries_txt = { 'x', 'p', 'l', 'a', 'r', 'e', 'h', 'e', 'r',
    'e', ' ', 'a', 'x', 'a', 'r', 'g', 'x', 'a', 'r', 'g', 'f', 'l', 'o', 'w',
    'e', 'r', 's', ','};
  std::vector<size_t> query_lengths = { 6, 6, 4, 4, 8 };
  q_list queries(std::move(queries_txt), std::move(query_lengths));
  std::vector<size_t> results;
  if (env_.size() == 1) {
    results = pt_.counting_batched<dpt::com::local_communication>(
      std::move(queries), manager_, part_sa_);
  } else {
    results = pt_.counting_batched<dpt::com::collective_communication>(
      std::move(queries), manager_, part_sa_);
  }
  for (const auto result : results) {
    ASSERT_EQ(size_t(0), result);
  }
}

TEST_F(patricia_trie_pointer_test, enumeration_batched_existing) {
  q_list queries = gen_random_existing_queries(2000, 10);
  std::pair<std::vector<size_t>, std::vector<size_t>> results;
  auto q_checker = queries;
  if (env_.size() == 1) {
    results = pt_.enumeration_batched<dpt::com::local_communication>(
      std::move(queries), manager_, part_sa_);
  } else {
    results = pt_.enumeration_batched<dpt::com::collective_communication>(
      std::move(queries), manager_, part_sa_);
  }
  for (size_t i = 0; i < results.second.size(); ++i) {
    size_t occurrence_query = 0;
    size_t cur_text_pos = 0;
    while ((cur_text_pos = global_text_.find(q_checker[i].query, cur_text_pos,
      q_checker[i].length)) != std::string::npos) {
      ++occurrence_query;
      ++cur_text_pos;
    }
    ASSERT_EQ(occurrence_query, results.second[i]);
  }
}

TEST_F(patricia_trie_pointer_test, enumeration_batched_non_existing) {
  std::vector<char> queries_txt = { 'x', 'p', 'l', 'a', 'r', 'e', 'h', 'e', 'r',
    'e', ' ', 'a', 'x', 'a', 'r', 'g', 'x', 'a', 'r', 'g', 'f', 'l', 'o', 'w',
    'e', 'r', 's', ','};
  std::vector<size_t> query_lengths = { 6, 6, 4, 4, 8 };
  q_list queries(std::move(queries_txt), std::move(query_lengths));
  std::pair<std::vector<size_t>, std::vector<size_t>> results;
  if (env_.size() == 1) {
    results = pt_.enumeration_batched<dpt::com::local_communication>(
      std::move(queries), manager_, part_sa_);
  } else {
    results = pt_.enumeration_batched<dpt::com::collective_communication>(
      std::move(queries), manager_, part_sa_);
  }
  for (const auto sizes : results.second) {
    ASSERT_EQ(static_cast<size_t>(0), sizes);
  }
}

/******************************************************************************/
