/*******************************************************************************
 * tests/query/query_view_test.cpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <gtest/gtest.h>

#include <dpt/query/query_view.hpp>

class query_view_test : public ::testing::Test {

protected:
  virtual void SetUp() {
    content_ = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
      'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
  }

  virtual void TearDown() { }

public:
  std::vector<char> content_;
}; // class query_view_test

TEST_F(query_view_test, subscript_operator) {
  std::vector<dpt::query::query_view<char, size_t>> views = {
   dpt::query::query_view<char, size_t> { content_.data(), 5 },
   dpt::query::query_view<char, size_t> { content_.data(), 12 },
   dpt::query::query_view<char, size_t> { content_.data(), 3 }
  };

  for (const auto& view : views) {
    for (size_t i = 0; i < view.length; ++i) {
      ASSERT_EQ(content_[i], view[i]);
    }
  }
}

TEST_F(query_view_test, equal_operator) {
  std::vector<dpt::query::query_view<char, size_t>> views = {
   dpt::query::query_view<char, size_t> { content_.data(), 5 },
   dpt::query::query_view<char, size_t> { content_.data(), 12 },
   dpt::query::query_view<char, size_t> { content_.data(), 3 }
  };
  ASSERT_TRUE(views[0] == content_.data());
  ASSERT_TRUE(views[1] == content_.data());
  ASSERT_TRUE(views[2] == content_.data());
}

/******************************************************************************/
