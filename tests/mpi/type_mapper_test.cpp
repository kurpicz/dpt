/*******************************************************************************
 * tests/mpi/type_mapper_test.cpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <gtest/gtest.h>
#include <stdint.h>
#include <tuple>

#include <dpt/mpi/type_mapper.hpp>

TEST(type_mapper, Type) {
  using namespace dpt::mpi;

  ASSERT_EQ(MPI_CHAR, type_mapper<int8_t>::type());
  ASSERT_EQ(MPI_BYTE, type_mapper<uint8_t>::type());
  ASSERT_EQ(MPI_SHORT, type_mapper<int16_t>::type());
  ASSERT_EQ(MPI_UNSIGNED_SHORT, type_mapper<uint16_t>::type());
  ASSERT_EQ(MPI_INT, type_mapper<int32_t>::type());
  ASSERT_EQ(MPI_UNSIGNED, type_mapper<uint32_t>::type());
  ASSERT_EQ(MPI_LONG_LONG_INT, type_mapper<int64_t>::type());
  ASSERT_EQ(MPI_UNSIGNED_LONG_LONG, type_mapper<uint64_t>::type());

  ASSERT_EQ(MPI_CHAR,(type_mapper<std::tuple<int8_t, int8_t>>::type()));
  ASSERT_EQ(MPI_BYTE,(type_mapper<std::tuple<uint8_t, uint8_t>>::type()));
  ASSERT_EQ(MPI_SHORT,(type_mapper<std::tuple<int16_t, int16_t>>::type()));
  ASSERT_EQ(MPI_UNSIGNED_SHORT,
    (type_mapper<std::tuple<uint16_t, uint16_t>>::type()));
  ASSERT_EQ(MPI_INT,(type_mapper<std::tuple<int32_t, int32_t>>::type()));
  ASSERT_EQ(MPI_UNSIGNED,(type_mapper<std::tuple<uint32_t, uint32_t>>::type()));
  ASSERT_EQ(MPI_LONG_LONG_INT,
    (type_mapper<std::tuple<int64_t, int64_t>>::type()));
  ASSERT_EQ(MPI_UNSIGNED_LONG_LONG,
    (type_mapper<std::tuple<uint64_t, uint64_t>>::type()));

  ASSERT_EQ(MPI_UNSIGNED_LONG_LONG,(type_mapper<
    std::tuple<uint64_t, uint64_t, uint64_t>>::type()));
  ASSERT_EQ(MPI_UNSIGNED_LONG_LONG,(type_mapper<
    std::tuple<uint64_t, uint64_t, uint64_t, uint64_t>>::type()));
  ASSERT_EQ(MPI_UNSIGNED_LONG_LONG,(type_mapper<
    std::tuple<uint64_t, uint64_t, uint64_t, uint64_t, uint64_t>>::type()));

  ASSERT_EQ(MPI_BYTE,(type_mapper<std::tuple<uint64_t, uint8_t>>::type()));
  ASSERT_EQ(MPI_BYTE,(type_mapper<
    std::tuple<uint64_t, int64_t, uint64_t>>::type()));
  ASSERT_EQ(MPI_BYTE,(type_mapper<
    std::tuple<uint64_t, uint64_t, uint64_t, int32_t>>::type()));
  ASSERT_EQ(MPI_BYTE,(type_mapper<
    std::tuple<int8_t, int16_t, int32_t, int64_t, uint64_t>>::type()));
}

TEST(type_mapper, Size) {
  using namespace dpt::mpi;

  ASSERT_EQ(static_cast<uint64_t>(1), type_mapper<int8_t>::factor());
  ASSERT_EQ(static_cast<uint64_t>(1), type_mapper<uint8_t>::factor());
  ASSERT_EQ(static_cast<uint64_t>(1), type_mapper<int16_t>::factor());
  ASSERT_EQ(static_cast<uint64_t>(1), type_mapper<uint16_t>::factor());
  ASSERT_EQ(static_cast<uint64_t>(1), type_mapper<int32_t>::factor());
  ASSERT_EQ(static_cast<uint64_t>(1), type_mapper<uint32_t>::factor());
  ASSERT_EQ(static_cast<uint64_t>(1), type_mapper<int64_t>::factor());
  ASSERT_EQ(static_cast<uint64_t>(1), type_mapper<uint64_t>::factor());

  ASSERT_EQ(static_cast<uint64_t>(2),
    (type_mapper<std::tuple<int8_t, int8_t>>::factor()));
  ASSERT_EQ(static_cast<uint64_t>(2),
    (type_mapper<std::tuple<uint8_t, uint8_t>>::factor()));
  ASSERT_EQ(static_cast<uint64_t>(2),
    (type_mapper<std::tuple<int16_t, int16_t>>::factor()));
  ASSERT_EQ(static_cast<uint64_t>(2),
    (type_mapper<std::tuple<uint16_t, uint16_t>>::factor()));
  ASSERT_EQ(static_cast<uint64_t>(2),
    (type_mapper<std::tuple<int32_t, int32_t>>::factor()));
  ASSERT_EQ(static_cast<uint64_t>(2),
    (type_mapper<std::tuple<uint32_t, uint32_t>>::factor()));
  ASSERT_EQ(static_cast<uint64_t>(2),
    (type_mapper<std::tuple<int64_t, int64_t>>::factor()));
  ASSERT_EQ(static_cast<uint64_t>(2),
    (type_mapper<std::tuple<uint64_t, uint64_t>>::factor()));

  ASSERT_EQ(static_cast<uint64_t>(3),(type_mapper<
    std::tuple<uint64_t, uint64_t, uint64_t>>::factor()));
  ASSERT_EQ(static_cast<uint64_t>(4),(type_mapper<
    std::tuple<uint64_t, uint64_t, uint64_t, uint64_t>>::factor()));
  ASSERT_EQ(static_cast<uint64_t>(5),(type_mapper<
    std::tuple<uint64_t, uint64_t, uint64_t, uint64_t, uint64_t>>::factor()));

  ASSERT_EQ(static_cast<uint64_t>(24),(type_mapper<
    std::tuple<uint64_t, uint64_t, int64_t>>::factor()));
  ASSERT_EQ(static_cast<uint64_t>(14),(type_mapper<
    std::tuple<int8_t, uint8_t, int32_t, uint64_t>>::factor()));
  ASSERT_EQ(static_cast<uint64_t>(30),(type_mapper<
    std::tuple<int64_t, uint64_t, uint16_t, uint32_t, uint64_t>>::factor()));
}

/******************************************************************************/
