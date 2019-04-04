/*******************************************************************************
 * benchmark/dpt.cpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <string>

#include "tlx/cmdline_parser.hpp"

#include "com/collective.hpp"
#include "com/manager.hpp"
#include "mpi/io.hpp"
#include "mpi/environment.hpp"
#include "query/query_list.hpp"
#include "tree/distributed_patricia_trie.hpp"
#include "tree/compact_trie_pointer.hpp"
#include "tree/patricia_trie_pointer.hpp"
#include "util/uint_types.hpp"

using glidx_t = dpt::uint40;
//using glidx_t = uint64_t;

int32_t main(int32_t argc, char const* argv[]) {
  dpt::mpi::environment env;
  tlx::CmdlineParser cp;

  cp.set_description("dpt: Distributed Patricia Tries");
  cp.set_author("Florian Kurpicz <florian.kurpicz@tu-dortmund.de>");

  std::string sa_file;
  cp.add_param_string("sa_file", sa_file, "The suffix array.");
  std::string lcp_file;
  cp.add_param_string("lcp_file", lcp_file, "The LCP array.");
  std::string text_file;
  cp.add_param_string("text_file", text_file, "The input text.");
  std::string query_file;
  cp.add_string('q',"queries", query_file, "The queries.");
  uint32_t number_queries = 0;
  cp.add_unsigned('n', "number_of_queries_per_pe", "N", number_queries,
                  "Initially have batches of size N queries at each PE.");
  std::string query_type("ex");
  cp.add_string('t', "query_type", query_type, "The type of query:\n"
                "[ex]istential queries (default), [co]unting queries, or "
                "[en]umeration queries.");

  if (!cp.process(argc, argv)) {
    return -1;
  }

  auto local_text = dpt::mpi::distribute_file<uint8_t, glidx_t,
                                              uint32_t>(text_file.c_str(), 40);
  dpt::com::manager<uint8_t, glidx_t, uint32_t> com_manager(std::move(local_text));
  auto local_sa = dpt::mpi::distribute_file<glidx_t, glidx_t,
                                            uint32_t>(sa_file.c_str(), 0);
  auto local_lcp = dpt::mpi::distribute_file<glidx_t, glidx_t,
                                             uint32_t>(lcp_file.c_str(), 0);
  
  dpt::tree::distributed_patricia_trie<uint8_t, glidx_t, uint32_t,
                                       dpt::tree::compact_trie_pointer,
                                       dpt::tree::patricia_trie_pointer> dpt(text_file.c_str(),
                                                                             sa_file.c_str(),
                                                                             lcp_file.c_str(), 30);

  auto start_time = MPI_Wtime();
  dpt.construct<dpt::com::collective_communication, dpt::com::collective_communication>();
  auto end_time = MPI_Wtime();
  if (env.rank() == 0) {
    std::cout << "CONSTRUCTION TIME: " << end_time - start_time << std::endl;
  }

  if (query_file.size() > 0) {
    if (number_queries == 0 && env.rank() == 0) {
      std::cout << "-n, --number_of_queries_per_pe is required." << std::endl;
      std::exit(-1);
    }
    auto query_text = dpt::mpi::distribute_linewise<uint8_t,
                                                    glidx_t, uint32_t>(query_file, number_queries,
                                                    30, 0, env);
    dpt::query::query_list<uint8_t, glidx_t,
                           uint32_t> queries(std::move(query_text), 0, 30);
    start_time = MPI_Wtime();
    if (query_type.compare("co") == 0) {
      dpt.counting_batched<dpt::com::collective_communication>(std::move(queries));
    } else if (query_type.compare("en") == 0) {
      dpt.enumeration_batched<dpt::com::collective_communication>(std::move(queries));
    } else {
      dpt.existential_batched<dpt::com::collective_communication>(std::move(queries));
    }
    end_time = MPI_Wtime();
    if (env.rank() == 0) {
      std::cout << "QUERY TIME: " << end_time - start_time << std::endl;
    }
  }

  env.finalize();
  return 0;
}

/******************************************************************************/
