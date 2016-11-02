/*******************************************************************************
 * tests/mpi/io_test.cpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <vector>

#include "dpt/mpi/io.hpp"
#include "mpi/environment.hpp"

TEST(io_test, distribute_char) {
  auto local_text = dpt::mpi::distribute_file<char, uint32_t, uint32_t>(
    "./test_data/the_three_brothers.txt", 10);

  std::ifstream stream("./test_data/the_three_brothers.txt",
    std::ios::in);

  stream.seekg(0, std::ios::end);
  size_t text_size = stream.tellg();

  stream.seekg(0);
  auto text = std::vector<char>(std::istreambuf_iterator<char>(stream),
    std::istreambuf_iterator<char>());
  stream.close();

  ASSERT_EQ(text_size / 4, (local_text.local_size()));

  int32_t rank = local_text.text_environment().rank();
  auto local_startpos = 
    rank > 0 ? ((rank * (local_text.local_size()))) : static_cast<size_t>(0);
  for (size_t i = 0; i < (local_text.local_size()); ++i) {
    ASSERT_EQ(text[local_startpos + i],
      local_text[i]);
  }
}

TEST(io_test, distribute_and_transform) {
  auto local_text =
    dpt::mpi::distribute_and_transform_file<char, uint64_t, uint32_t, uint32_t>(
    "./test_data/the_three_brothers.txt", 10);

  std::ifstream stream("./test_data/the_three_brothers.txt",
    std::ios::in);

  stream.seekg(0, std::ios::end);
  size_t text_size = stream.tellg();

  stream.seekg(0);
  auto text = std::vector<char>(std::istreambuf_iterator<char>(stream),
    std::istreambuf_iterator<char>());
  stream.close();

  ASSERT_EQ(text_size / 4, (local_text.local_size()));

  int32_t rank = local_text.text_environment().rank();
  auto local_startpos = 
    rank > 0 ? ((rank * (local_text.local_size()))) : static_cast<size_t>(0);
  for (size_t i = 0; i < (local_text.local_size()); ++i) {
    ASSERT_EQ(static_cast<uint64_t>(text[local_startpos + i]),
      local_text[i]);
  }
}

TEST(io_test, distribute_levelwise) {
  dpt::mpi::environment env;
  std::vector<std::vector<char>> test_data = {
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
  std::string file_name = "test_data/twenty_lines.txt";
  size_t lines_per_pe = 5;
  char separator = '\n';
  auto local_lines =
    dpt::mpi::distribute_linewise(file_name, lines_per_pe, 30, separator);

  size_t separator_count = 0;
  for (const auto c : local_lines) {
    if (c == separator) {
      ++separator_count;
    }
  }
  ASSERT_EQ(lines_per_pe, separator_count);
  for (size_t line = 0, pos = 0; line < lines_per_pe; ++line, ++pos) {
    size_t test_pos = 0;
    while (local_lines[pos] != separator) {
      ASSERT_EQ(test_data[(lines_per_pe * env.rank()) + line][test_pos++],
        local_lines[pos++]);
    }
  }

  local_lines =
    dpt::mpi::distribute_linewise(file_name, lines_per_pe, 4, separator);
  separator_count = 0;
  for (const auto c : local_lines) {
    if (c == separator) {
      ++separator_count;
    }
  }
  ASSERT_EQ(lines_per_pe, separator_count);
  for (size_t line = 0, pos = 0; line < lines_per_pe; ++line, ++pos) {
    size_t test_pos = 0;
    while (local_lines[pos] != separator) {
      ASSERT_EQ(test_data[(lines_per_pe * env.rank()) + line][test_pos++],
        local_lines[pos++]);
    }
    ASSERT_EQ(test_pos, static_cast<size_t>(4));
  }
}

/******************************************************************************/
