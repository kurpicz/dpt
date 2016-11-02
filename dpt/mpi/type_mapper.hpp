/*******************************************************************************
 * dpt/mpi/type_mapper.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (c) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <mpi.h>
#include <stdint.h>
#include <type_traits>

#include "util/are_same.hpp"

namespace dpt {
namespace mpi {

template <typename cxx_type>
struct type_mapper {
  static_assert(std::is_trivial<cxx_type>::value,
    "Only trivial types can be mapped.");

  static constexpr MPI_Datatype type() { return MPI_BYTE; }
  static constexpr uint64_t factor() { return sizeof(cxx_type); }
};

#define DPT_INTEGRAL_DATATYPE_MAPPER(integral_type, mpi_type) \
template <>                                                   \
struct type_mapper<integral_type> {                           \
  static constexpr MPI_Datatype type() { return mpi_type; }   \
  static constexpr uint64_t factor() { return 1; }            \
};                                                            \

DPT_INTEGRAL_DATATYPE_MAPPER(int8_t, MPI_CHAR);
DPT_INTEGRAL_DATATYPE_MAPPER(uint8_t, MPI_BYTE);
DPT_INTEGRAL_DATATYPE_MAPPER(int16_t, MPI_SHORT);
DPT_INTEGRAL_DATATYPE_MAPPER(uint16_t, MPI_UNSIGNED_SHORT);
DPT_INTEGRAL_DATATYPE_MAPPER(int32_t, MPI_INT);
DPT_INTEGRAL_DATATYPE_MAPPER(uint32_t, MPI_UNSIGNED);
DPT_INTEGRAL_DATATYPE_MAPPER(int64_t, MPI_LONG_LONG_INT);
DPT_INTEGRAL_DATATYPE_MAPPER(uint64_t, MPI_UNSIGNED_LONG_LONG);

#undef DPT_INTEGRAL_DATATYPE_MAPPER

template <typename... cxx_types>
struct type_mapper<std::tuple<cxx_types ...>> {

  static constexpr MPI_Datatype type() { 
    return dpt::util::are_same<cxx_types...>() ?
      type_mapper<typename std::tuple_element<
        0, std::tuple<cxx_types...>>::type>::type() : MPI_BYTE;
  }

  static constexpr uint64_t factor() {
    return dpt::util::are_same<cxx_types...>() ?
      (type_mapper<cxx_types>::factor() + ...) : (sizeof(cxx_types) + ...);
  }
}; // class type_mapper

template <typename CXXType>
class data_type_mapper {

public:
  data_type_mapper() : custom_(type_mapper<CXXType>::factor() != 1) {
    if (custom_) {
      MPI_Type_contiguous(type_mapper<CXXType>::factor(),
                          MPI_BYTE,
                          &mpi_datatype_);
      MPI_Type_commit(&mpi_datatype_);
    } else { mpi_datatype_ = type_mapper<CXXType>::type(); }
  }

  ~data_type_mapper() {
    if (custom_) { MPI_Type_free(&mpi_datatype_); }
  }

  MPI_Datatype get_mpi_type() const { return mpi_datatype_; }

private:
  bool custom_;
  MPI_Datatype mpi_datatype_;
}; // class datatype

} // namespace mpi
} // namespace dpt

/******************************************************************************/
