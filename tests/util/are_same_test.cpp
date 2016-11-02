/*******************************************************************************
 * tests/util/are_same_test.cpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <cstdint>
#include <gtest/gtest.h>
#include <tuple>

#include <dpt/util/are_same.hpp>

TEST(are_same, Value) {
  using namespace dpt::util;

  ASSERT_TRUE(are_same<int8_t>());
  ASSERT_TRUE(are_same<uint8_t>());
  ASSERT_TRUE(are_same<int16_t>());
  ASSERT_TRUE(are_same<uint16_t>());
  ASSERT_TRUE(are_same<int32_t>());
  ASSERT_TRUE(are_same<uint32_t>());
  ASSERT_TRUE(are_same<int64_t>());
  ASSERT_TRUE(are_same<uint64_t>());

  ASSERT_TRUE((are_same<int8_t, int8_t>()));
  ASSERT_TRUE((are_same<uint8_t, uint8_t>()));
  ASSERT_TRUE((are_same<int16_t, int16_t>()));
  ASSERT_TRUE((are_same<uint16_t, uint16_t>()));
  ASSERT_TRUE((are_same<int32_t, int32_t>()));
  ASSERT_TRUE((are_same<uint32_t, uint32_t>()));
  ASSERT_TRUE((are_same<int64_t, int64_t>()));
  ASSERT_TRUE((are_same<uint64_t, uint64_t>()));

  ASSERT_TRUE((are_same<int8_t, int8_t, int8_t>()));
  ASSERT_TRUE((are_same<uint8_t, uint8_t, uint8_t>()));
  ASSERT_TRUE((are_same<int16_t, int16_t, int16_t>()));
  ASSERT_TRUE((are_same<uint16_t, uint16_t, uint16_t>()));
  ASSERT_TRUE((are_same<int32_t, int32_t, int32_t>()));
  ASSERT_TRUE((are_same<uint32_t, uint32_t, uint32_t>()));
  ASSERT_TRUE((are_same<int64_t, int64_t, int64_t>()));
  ASSERT_TRUE((are_same<uint64_t, uint64_t, uint64_t>()));

  ASSERT_FALSE((are_same<int8_t, uint8_t>()));
  ASSERT_FALSE((are_same<uint8_t, int8_t>()));
  ASSERT_FALSE((are_same<int16_t, int32_t>()));
  ASSERT_FALSE((are_same<uint16_t, uint64_t>()));
  ASSERT_FALSE((are_same<int32_t, uint32_t>()));
  ASSERT_FALSE((are_same<uint32_t, uint8_t>()));
  ASSERT_FALSE((are_same<int64_t, int32_t>()));
  ASSERT_FALSE((are_same<uint64_t, int8_t>()));

  ASSERT_FALSE((are_same<int16_t, int8_t, int8_t>()));
  ASSERT_FALSE((are_same<uint8_t, uint64_t, uint8_t>()));
  ASSERT_FALSE((are_same<int16_t, int8_t, int32_t>()));
  ASSERT_FALSE((are_same<uint16_t, uint16_t, uint64_t>()));
  ASSERT_FALSE((are_same<int16_t, uint16_t, int32_t>()));
  ASSERT_FALSE((are_same<uint32_t, uint8_t, uint32_t>()));
  ASSERT_FALSE((are_same<int64_t, int64_t, uint64_t>()));
  ASSERT_FALSE((are_same<int32_t, int64_t, uint32_t>()));
}

/******************************************************************************/
