/*******************************************************************************
 * tests/util/partition_test.cpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <gtest/gtest.h>
#include <memory>
#include <stdint.h>
#include <vector>

#include "util/partition.hpp"

using partition = dpt::util::partition<char, uint32_t, uint32_t>; 

class partition_test : public ::testing::Test {

protected:
  virtual void SetUp() {
    std::vector<char> content = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
      'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
      'y', 'z'};
    part = std::make_unique<partition>(
      content.size() * 2, content.size(), std::move(content));
  }

  virtual void TearDown() { }

public:
  std::unique_ptr<partition> part;
}; // class partition_test

TEST_F(partition_test, BracketOperator) {
  for (size_t i = 0; i < 26; ++i) {
    ASSERT_EQ(static_cast<char>(97 + i), (*part)[i]);
  }
}

TEST_F(partition_test, LocalData) {
  auto local_data = part->local_data();
  char test = 'a';
  for (const auto& ld : (*local_data)) {
    ASSERT_EQ(test++, ld);
  }
}

TEST_F(partition_test, ProcessingElemenetAndPosition) {
  for (uint32_t i = 0; i < 26; ++i) {
    ASSERT_EQ(part->pe_and_norm_position(i),
      std::make_pair(static_cast<int32_t>(0), i));
    ASSERT_EQ(part->pe_and_norm_position(i + 26),
      std::make_pair(static_cast<int32_t>(1), i));
  }
}

/******************************************************************************/
