/*******************************************************************************
 * benchmark/array_transform.cpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "tlx/cmdline_parser.hpp"

#include "mpi/environment.hpp"
#include "mpi/io.hpp"
#include "util/uint_types.hpp"

int32_t main(int32_t argc, char const* argv[]) {
  dpt::mpi::environment env;
  tlx::CmdlineParser cp;

  cp.set_description("Transform 64 bit arrays to 40 bit arrays");
  cp.set_author("Florian Kurpicz <florian.kurpicz@tu-dortmund.de>");

  std::string file;
  cp.add_param_string("file", file, "The file containing the array.");

  if (!cp.process(argc, argv)) {
    return -1;
  }

  auto local_slice = dpt::mpi::distribute_file<uint64_t, uint64_t,
                                            uint64_t>(file.c_str(), 0);
  std::vector<dpt::uint40> result(local_slice.local_size());
  for (size_t i = 0; i < local_slice.local_size(); ++i) {
    result[i] = static_cast<dpt::uint40>(local_slice[i]);
  }

  std::string new_name = file + "_40bit";
  dpt::mpi::write_data(result, new_name.c_str(), env);

  env.finalize();
  return 0;
}

/******************************************************************************/
