/*******************************************************************************
 * dpt/mpi/environment.cpp
 *
 * Copyright (C) 2016 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "mpi/environment.hpp"
#include "util/macros.hpp"

namespace dpt::mpi {

environment::environment() : environment(MPI_COMM_WORLD) { }

environment::environment(MPI_Comm communicator)
 : communicator_(communicator) {
  if (DSSS_UNLIKELY(!environment::initialized())) {
    MPI_Init(nullptr, nullptr);
  }
  MPI_Comm_rank(communicator_, &world_rank_);
  MPI_Comm_size(communicator_, &world_size_);
}

environment::environment(const environment& other) = default;
environment::environment(environment&& other) = default;
environment::~environment() { }

environment& environment::operator = (environment&& other) {
  communicator_ = other.communicator_;
  world_rank_ = other.world_rank_;
  world_size_ = other.world_size_;
  return *this;
}

void environment::finalize() {
    MPI_Finalize();
  }

std::int32_t environment::rank() const {
  return world_rank_;
}

std::int32_t environment::size() const {
  return world_size_;
}

MPI_Comm environment::communicator() const {
  return communicator_;
}

void environment::barrier() const {
  MPI_Barrier(communicator_);
}

} // namespace dpt::mpi

/******************************************************************************/
