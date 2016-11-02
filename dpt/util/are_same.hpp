/*******************************************************************************
 * dpt/util/are_same.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (c) 2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

/// \file
/// \brief File contains functions to check if all given types are the same.
///
/// This is used (among others) for communication (MPI).

#pragma once
#ifndef DPT_UTIL_ARE_SAME_HEADER
#define DPT_UTIL_ARE_SAME_HEADER

#include <type_traits>

namespace dpt {
namespace util {

/// \brief Tail call for comparison of types. (Or if there is just one.)
///
/// \tparam single_type Type that is compared with itself.
/// \return Always true (as there is just one type).
template <typename single_type>
constexpr bool are_same() {
  return true;
}

/// \brief Compares all types of the variadic function template.
///
/// This function is used to determine if all types are the same. First, the
/// first and second type are compared, then the result is added to the
/// comparison of the second type with the third, and so on. 
///
/// \tparam first_type First type to be compared.
/// \tparam second_type Second type to be compared.
/// \tparam other_types All other types.
/// \return \e true if all types are the same \e false otherwise.
template <typename first_type, typename second_type, typename... other_types>
constexpr bool are_same() {
  return (std::is_same<first_type, second_type>::value &&
    are_same<second_type, other_types...>());
}

} // namespace util
} // namespace dpt

#endif // DPT_UTIL_ARE_SAME_HEADER

/******************************************************************************/
