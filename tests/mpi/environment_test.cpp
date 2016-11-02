/*******************************************************************************
 * tests/util/environment_test.cpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <gtest/gtest.h>

#include "mpi/environment.hpp"

using environment = dpt::mpi::environment;

class environment_test : public ::testing::Test {
 protected:
  virtual void SetUp() { }
  virtual void TearDown() { }

public:
  environment env_;
}; // class environment_test

TEST_F(environment_test, Constructor) {
  ASSERT_TRUE(environment::initialized());

  ASSERT_EQ(static_cast<int32_t>(4), env_.size());
  ASSERT_LE(static_cast<int32_t>(0), env_.rank());
  ASSERT_GE(static_cast<int32_t>(3), env_.rank());
  ASSERT_EQ(MPI_COMM_WORLD, env_.communicator());
}

/******************************************************************************/
