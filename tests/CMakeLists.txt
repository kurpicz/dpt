################################################################################
# tests/CMakeLists.txt
#
# Part of dpt - Distributed Patricia Trie
#
# Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

enable_testing()

# Copy the test data
file(GLOB test_data "test_data/*")
file(COPY ${test_data} DESTINATION "${CMAKE_BINARY_DIR}/tests/test_data")

include(testsuite.cmake)

run_test(com/local_test)
run_test(mpi/type_mapper_test)
run_test(query/query_list_test)
run_test(query/query_view_test)
run_test(util/are_same_test)
run_test(util/uint_types_test)
run_test(tree/patricia_trie_pointer_test)

run_distributed_test(com/collective_test 4)
run_distributed_test(mpi/environment_test 4)
run_distributed_test(mpi/io_test 4)
run_distributed_test(tree/compact_trie_pointer_test 1)
run_distributed_test(tree/compact_trie_pointer_test 4)
run_distributed_test(tree/dpt_test 4)
run_distributed_test(util/partition_test 4)

################################################################################
