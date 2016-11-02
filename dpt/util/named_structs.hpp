/*******************************************************************************
 * dpt/util/named_structs.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

/// \file
/// \brief File contains different structs used by multiple classes.
///
/// This file contains structs that we could also represent as tuple. The usage
/// of structs results in more readability of the code. Hence, we advice that
/// these structs should be used whenever appropriate.

#pragma once
#ifndef DPT_UTIL_NAMED_STRUCTS_HEADER
#define DPT_UTIL_NAMED_STRUCTS_HEADER

#include <ostream>

namespace dpt {
namespace util {

/// \brief Tuple consisting of the id of a processing element (pe) and a
///        position on that processing element.
///
/// \tparam IndexType Type that is used to represent the (local) position.
template <typename IndexType>
struct pe_position {
  int32_t pe;
  IndexType position;

  inline bool operator == (const pe_position& other) const {
    return std::tie(pe, position) == std::tie(other.pe, other.position);
  }

  inline bool operator == (const std::pair<int32_t, IndexType>& other) const {
    return std::make_pair(pe, position) == other;
  }

  friend std::ostream& operator << (std::ostream& os,
    const pe_position& pap) {
    return os << '(' << pap.pe << '|' << pap.position << ')';
  }
} __attribute__ ((packed)); // struct pe_position

/// \brief Tuple consisting of a position and a size. Both of type IndexType.
///
/// \tparam IndexType Type that is used to represent the position and size.
template <typename IndexType>
struct position_size {
  IndexType position;
  IndexType size;

  friend std::ostream& operator << (std::ostream& os,
    const position_size& psr) {
    return os << '(' << psr.position << '|' << psr.size << ')';
  }
} __attribute__ ((packed)); // struct position_size

} // namespace util
} // namespace dpt

#endif // DPT_UTIL_NAMED_STRUCTS_HEADER

/******************************************************************************/
