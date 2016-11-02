/*******************************************************************************
 * tests/com/collective_test.cpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <gtest/gtest.h>
#include <memory>
#include <stdint.h>
#include <vector>

#include "com/collective.hpp"
#include "mpi/environment.hpp"
#include "util/partition.hpp"

using partition = dpt::util::partition<char, uint32_t, uint32_t>;

class collective_test : public ::testing::Test {

protected:
  virtual void SetUp() {
    std::vector<char> content = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
      'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
      'y', 'z'};
    part_ = partition(
      static_cast<uint32_t>(content.size() * env_.size()),
      static_cast<uint32_t>(content.size()), std::move(content), env_);
  }

  virtual void TearDown() { }

public:
  dpt::mpi::environment env_;
  partition part_;
}; // class collective_test

TEST_F(collective_test, RequestCharacter) {
  std::vector<uint32_t> request_positions;
  for (int32_t i = 0; i < env_.size(); ++i) {
    request_positions.emplace_back(1);
  }
  auto result = dpt::com::collective_communication<char, uint32_t, uint32_t>::
    request_characters(request_positions, part_);
  for (const auto& rec_char : result) {
    ASSERT_EQ('b', rec_char);
  }
  request_positions.clear();
  result.clear();

  for (int32_t i = 0; i < env_.size(); ++i) {
    for (int32_t rank = 0; rank < env_.size(); ++rank) {
      request_positions.emplace_back(1 + (rank * part_.local_size()));
    }
  }
  result = dpt::com::collective_communication<char, uint32_t, uint32_t>::
    request_characters(request_positions, part_);
  for (const auto& rec_char : result) {
    ASSERT_EQ('b', rec_char);
  }
  request_positions.clear();
  result.clear();

  for (uint32_t text_pos = 0; text_pos < part_.global_size(); ++text_pos) {
    request_positions.emplace_back(text_pos);
  }
  result = dpt::com::collective_communication<char, uint32_t, uint32_t>::
    request_characters(request_positions, part_);

  for (uint32_t i = 0; i < result.size(); ++i) {
    ASSERT_EQ(static_cast<char>('a' + (i % part_.local_size())), result[i]);
  }
}

TEST_F(collective_test, RequestSubstring) {
  std::vector<uint32_t> request_positions;
  std::vector<uint32_t> request_lengths;
  for (int32_t i = 0; i < env_.size(); ++i) {
    request_positions.emplace_back(1);
    request_lengths.emplace_back(i + 1);
  }
  auto result = dpt::com::collective_communication<char, uint32_t, uint32_t>::
    request_substrings(request_positions, request_lengths, part_);
  for (size_t i = 0, pos = 0; i < request_positions.size(); ++i) {
    for (size_t j = 0; j < request_lengths[i]; ++j) {
      ASSERT_EQ(static_cast<char>('b' + j), result[pos++]);
    }
  }
  request_positions.clear();
  request_lengths.clear();
  result.clear();

  for (int32_t i = 0; i < env_.size(); ++i) {
    for (int32_t rank = 0; rank < env_.size(); ++rank) {
      request_positions.emplace_back(1 + (rank * part_.local_size()));
      request_lengths.emplace_back(i + 1);
    }
  }
  result = dpt::com::collective_communication<char, uint32_t, uint32_t>::
    request_substrings(request_positions, request_lengths, part_);
  for (size_t i = 0, pos = 0; i < request_positions.size(); ++i) {
    for (size_t j = 0; j < request_lengths[i]; ++j) {
      ASSERT_EQ(static_cast<char>('b' + j), result[pos++]);
    }
  }
}

TEST_F(collective_test, RequestSubstringWithEmpty) {
  std::vector<uint32_t> request_positions;
  std::vector<uint32_t> request_lengths;
  for (int32_t i = 0; i < env_.size(); ++i) {
    if (0 == i % 2) {
      request_positions.emplace_back(1);
      request_lengths.emplace_back(i + 1);
    }
  }
  auto result = dpt::com::collective_communication<char, uint32_t, uint32_t>::
    request_substrings(request_positions, request_lengths, part_);
  for (size_t i = 0, pos = 0; i < request_positions.size(); ++i) {
    for (size_t j = 0; j < request_lengths[i]; ++j) {
      ASSERT_EQ(static_cast<char>('b' + j), result[pos++]);
    }
  }
  request_positions.clear();
  request_lengths.clear();
  result.clear();

  for (int32_t i = 0; i < env_.size(); ++i) {
    for (int32_t rank = 0; rank < env_.size(); ++rank) {
      request_positions.emplace_back(1 + (rank * part_.local_size()));
      request_lengths.emplace_back(i + 1);
    }
  }
  result = dpt::com::collective_communication<char, uint32_t, uint32_t>::
    request_substrings(request_positions, request_lengths, part_);
  for (size_t i = 0, pos = 0; i < request_positions.size(); ++i) {
    for (size_t j = 0; j < request_lengths[i]; ++j) {
      ASSERT_EQ(static_cast<char>('b' + j), result[pos++]);
    }
  }
}

TEST_F(collective_test, RequestSubstrinHead) {
  std::vector<uint32_t> request_positions;
  std::vector<uint32_t> request_lengths;
  for (int32_t i = 0; i < env_.size(); ++i) {
    request_positions.emplace_back(1);
    request_lengths.emplace_back(i + 1);
  }
  std::vector<char> heads;
  std::vector<char> tails;
  std::tie(heads, tails) =
    dpt::com::collective_communication<char, uint32_t, uint32_t>::
    request_substrings_head(request_positions, request_lengths, part_);
  for (size_t i = 0, head_pos = 0, tail_pos = 0;
    i < request_positions.size(); ++i) {
    ASSERT_EQ(static_cast<char>('b'), heads[head_pos++]);
    for (size_t j = 1; j < request_lengths[i]; ++j) {
      ASSERT_EQ(static_cast<char>('b' + j), tails[tail_pos++]);
    }
  }
  request_positions.clear();
  request_lengths.clear();
  heads.clear();
  tails.clear();

  for (int32_t i = 0; i < env_.size(); ++i) {
    for (int32_t rank = 0; rank < env_.size(); ++rank) {
      request_positions.emplace_back(1 + (rank * part_.local_size()));
      request_lengths.emplace_back(i + 1);
    }
  }

  std::tie(heads, tails) =
    dpt::com::collective_communication<char, uint32_t, uint32_t>::
    request_substrings_head(request_positions, request_lengths, part_);
  for (size_t i = 0, head_pos = 0, tail_pos = 0;
    i < request_positions.size(); ++i) {
    ASSERT_EQ(static_cast<char>('b'), heads[head_pos++]);
    for (size_t j = 1; j < request_lengths[i]; ++j) {
      ASSERT_EQ(static_cast<char>('b' + j), tails[tail_pos++]);
    }
  }
}

/******************************************************************************/
