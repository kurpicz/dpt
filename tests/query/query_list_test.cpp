/*******************************************************************************
 * tests/query/query_list_test.cpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include <iterator>

#include <dpt/query/query_list.hpp>

using q_list = dpt::query::query_list<char, size_t, size_t>;

class query_list_test : public ::testing::Test {

protected:
  virtual void SetUp() {
    std::vector<char> text_for_queries;
    std::ifstream stream("test_data/twenty_lines.txt", std::ios::in);
    std::istreambuf_iterator<char> iter(stream);
    std::copy(iter, std::istreambuf_iterator<char>(),
      std::back_inserter(text_for_queries));
    queries_ = q_list(std::move(text_for_queries), '\n', 30);
  }

  virtual void TearDown() { }

public:
  q_list queries_;
  std::vector<std::vector<char>> manually_added_queries = {
    { 'F', 'i', 'r', 's', 't', ' ', 'l', 'i', 'n', 'e' },
    { 'S', 'e', 'c', 'o', 'n', 'd', ' ', 'l', 'i', 'n', 'e' },
    { 'T', 'h', 'i', 'r', 'd', ' ', 'l', 'i', 'n', 'e' },
    { 'F', 'o', 'u', 'r', 't', 'h', ' ', 'l', 'i', 'n', 'e' },
    { 'F', 'i', 'f', 't', 'h', ' ', 'l', 'i', 'n', 'e' },
    { 'S', 'i', 'x', 't', 'h', ' ', 'l', 'i', 'n', 'e' },
    { 'S', 'e', 'v', 'e', 'n', 't', 'h', ' ', 'l', 'i', 'n', 'e' },
    { 'E', 'i', 'g', 'h', 't', 'h', 's', ' ', 'l', 'i', 'n', 'e' },
    { 'N', 'i', 'n', 't', 'h', ' ', 'l', 'i', 'n', 'e' },
    { 'T', 'e', 'n', 't', 'h', ' ', 'l', 'i', 'n', 'e' },
    { 'E', 'l', 'e', 'v', 'e', 'n', 't', 'h', ' ', 'l', 'i', 'n', 'e' },
    { 'T', 'w', 'e', 'l', 'f', 't', 'h', ' ', 'l', 'i', 'n', 'e' },
    { 'T', 'h', 'i', 'r', 't', 'e', 'e', 'n', 't', 'h', ' ',
      'l', 'i', 'n', 'e' },
    { 'F', 'o', 'u', 'r', 't', 'e', 'e', 'n', 't', 'h', ' ',
      'l', 'i', 'n', 'e' },
    { 'F', 'i', 'f', 't', 'e', 'e', 'n', 't', 'h', ' ', 'l', 'i', 'n', 'e' },
    { 'S', 'i', 'x', 't', 'e', 'e', 'n', 't', 'h', ' ', 'l', 'i', 'n', 'e' },
    { 'S', 'e', 'v', 'e', 'n', 't', 'e', 'e', 'n', 't', 'h', ' ',
      'l', 'i', 'n', 'e' },
    { 'E', 'i', 'g', 'h', 't', 'e', 'e', 'n', 't', 'h', ' ',
      'l', 'i', 'n', 'e' },
    { 'N', 'i', 'n', 'e', 't', 'e', 'e', 'n', 't', 'h', ' ',
      'l', 'i', 'n', 'e' },
    { 'T', 'w', 'e', 'n', 't', 'i', 'e', 't', 'h', ' ', 'l', 'i', 'n', 'e' }
  };
}; // class query_list_test

TEST_F(query_list_test, size) {
  ASSERT_EQ(static_cast<size_t>(20), queries_.size());
}

TEST_F(query_list_test, subscript_operator) {
  for (size_t i = 0; i < queries_.size(); ++i) {
    auto query = queries_[i];
    ASSERT_EQ(manually_added_queries[i].size(), query.length);
    for (size_t j = 0; j < query.length; ++j) {
      ASSERT_EQ(manually_added_queries[i][j], query[j]);
    }
  }
}

TEST_F(query_list_test, iterators) {
  size_t query_nr = 0;
  for (const auto& query : queries_) {
    ASSERT_EQ(manually_added_queries[query_nr].size(), query.length);
    for (size_t j = 0; j < query.length; ++j) {
      ASSERT_EQ(manually_added_queries[query_nr][j], query[j]);
    }
    ++query_nr;
  }
}

TEST_F(query_list_test, other_constructor) {
  std::vector<char> queries;
  std::vector<size_t> lengths;
  for (const auto& query : queries_) {
    std::copy_n(query.query, query.length, std::back_inserter(queries));
    lengths.emplace_back(query.length);
  }
  q_list list(std::move(queries), std::move(lengths));

  for (size_t i = 0; i < list.size(); ++i) {
    auto query = list[i];
    ASSERT_EQ(manually_added_queries[i].size(), query.length);
    for (size_t j = 0; j < query.length; ++j) {
      ASSERT_EQ(manually_added_queries[i][j], query[j]);
    }
  }
  size_t query_nr = 0;
  for (const auto& query : list) {
    ASSERT_EQ(manually_added_queries[query_nr].size(), query.length);
    for (size_t j = 0; j < query.length; ++j) {
      ASSERT_EQ(manually_added_queries[query_nr][j], query[j]);
    }
    ++query_nr;
  }
}

/******************************************************************************/
