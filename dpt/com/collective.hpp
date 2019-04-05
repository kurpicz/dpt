/*******************************************************************************
 * dpt/com/collective.hpp
 *
 * Part of dpt - Distributed Patricia Trie
 *
 * Copyright (C) 2016-2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <algorithm>
#include <memory>

#include "query/query_list.hpp"
#include "util/named_structs.hpp"
#include "util/partition.hpp"
#include "mpi/all_to_all.hpp"

namespace dpt {
namespace com {

/// \brief Implementation of the communication among processing elements using
///        collective communication.
///
/// \tparam Alphabet Type of the data that is distributed.
/// \tparam GlobalIndex Type of an index position on the global data.
/// \tparam LocalIndex Type of an index position on the local data.
template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
class collective_communication {

  using partition = dpt::util::partition<Alphabet, GlobalIndex, LocalIndex>;
  using pos_size_request = dpt::util::position_size<LocalIndex>;
  using query_list = dpt::query::query_list<Alphabet, GlobalIndex, LocalIndex>;

public:

  /// \param text_positions A vector of text positions.
  /// \param local_text The local (partition) of the data to distribute.
  /// \returns Globally distributed characters based on their (global) position.
  static std::vector<Alphabet> request_characters(
    std::vector<GlobalIndex>& text_positions,
    const partition& local_text);

  /// \param text_positions A vector of text positions.
  /// \param substring_lengths A vector of length of the requested substrings.
  /// \param local_text The local (partition) of the data to distribute.
  /// \returns Globally distributed substrings based on their (global) position.
  static std::vector<Alphabet> request_substrings(
    std::vector<GlobalIndex>& text_positions,
    const std::vector<LocalIndex>& substring_lengths,
    const partition& local_text_);

  /// \param text_positions A vector of text positions.
  /// \param substring_lengths A vector of length of the requested substrings.
  /// \param local_text The local (partition) of the data to distribute.
  /// \returns Globally distributed substrings based on their (global) position
  ///          but returns the first character (head) of each substring in a
  ///          separate vector.
  static std::pair<std::vector<Alphabet>, std::vector<Alphabet>>
    request_substrings_head(std::vector<GlobalIndex>& text_positions,
      const std::vector<LocalIndex>& substring_lengths,
      const partition& local_text);

  /// \param queries A vector of text (all queries concatenated w/o separator).
  /// \param query_lengths A vector of length of the queries.
  /// \param local_text The local (partition) of the data to distribute.
  /// \returns All queries that have been send to this processing element.
  static query_list distribute_queries(
    std::vector<Alphabet>& queries, std::vector<LocalIndex>& query_lengths,
    std::vector<size_t>& hist_lengths, std::vector<size_t>& hist);

}; // class collective_communication

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
std::vector<Alphabet> collective_communication<Alphabet, GlobalIndex, LocalIndex>
  ::request_characters(std::vector<GlobalIndex>& text_positions,
    const partition& local_text) {

  std::vector<size_t> hist(local_text.text_environment().size(), 0);
  for (const auto& pos : text_positions) {
    ++hist[local_text.pe(pos)];
  }
  std::vector<size_t> counts(hist);
  std::vector<int32_t> start_pos(hist.size(), 0);
  for (size_t i = 1; i < start_pos.size(); ++i) {
    start_pos[i] = start_pos[i - 1] + hist[i - 1];
  }
  std::vector<size_t> normalized_req_pos(text_positions.size(), 0);
  for (auto& pos : text_positions) {
    const auto pe_and_pos = local_text.pe_and_norm_position(pos);
    pos = pe_and_pos.pe;
    normalized_req_pos[start_pos[pos]++] = pe_and_pos.position;
  }
  std::vector<size_t>().swap(hist);

  std::vector<size_t> rec_req_counts;
  std::vector<size_t> rec_req_positions;
  std::tie(rec_req_counts, rec_req_positions) =
    dpt::mpi::alltoallv_counts(normalized_req_pos, counts);

  std::vector<Alphabet> response;
  response.reserve(rec_req_positions.size());
  for (const auto& request : rec_req_positions) {
    response.emplace_back(local_text[request]);
  }
  std::vector<size_t>().swap(rec_req_positions);
  std::vector<Alphabet> rec_characters =
    dpt::mpi::alltoallv(response, rec_req_counts);

  start_pos[0] = 0;
  for (size_t i = 1; i < start_pos.size(); ++i) {
    start_pos[i] = start_pos[i - 1] + counts[i - 1];
  }
  std::vector<Alphabet> result;
  result.reserve(rec_characters.size());
  for (const auto& pos : text_positions) {
    result.emplace_back(rec_characters[start_pos[pos]++]);
  }

  return result;
} // request_characters

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
std::vector<Alphabet> 
  collective_communication<Alphabet, GlobalIndex, LocalIndex>
  ::request_substrings(
    std::vector<GlobalIndex>& text_positions,
    const std::vector<LocalIndex>& substring_lengths,
    const partition& local_text) {

  // Compute the number of requests send to each PE and the size of the
  // substrings that we will receive from each PE.
  std::vector<size_t> hist(local_text.text_environment().size(), 0);
  std::vector<int32_t> hist_receiving(local_text.text_environment().size(), 0);
  for (size_t i = 0; i < text_positions.size(); ++i) {
    const int32_t pe = local_text.pe(text_positions[i]);
    ++hist[pe];
    hist_receiving[pe] += substring_lengths[i];
  }
  // Compute the displacements (offsets) of the request (that we send) of type
  // pos_size_request and the responses (that we receive) of type
  // std::vector<Alphabet>.
  std::vector<size_t> counts(hist);
  std::vector<int32_t> start_pos(hist.size(), 0);
  std::vector<int32_t> start_pos_receiving(hist_receiving.size(), 0);
  for (size_t i = 1; i < start_pos.size(); ++i) {
    start_pos[i] = start_pos[i - 1] + hist[i - 1];
    start_pos_receiving[i] = start_pos_receiving[i - 1] + hist_receiving[i - 1];
  }
  // Prepare the requests and store them in pos_size_request
  std::vector<pos_size_request> pos_size_requests(text_positions.size());
  for (size_t i = 0; i < text_positions.size(); ++i) {
    const auto pe_and_pos = local_text.pe_and_norm_position(text_positions[i]);
    text_positions[i] = pe_and_pos.pe;
    pos_size_requests[start_pos[pe_and_pos.pe]++] =
      pos_size_request { pe_and_pos.position, substring_lengths[i] };
  }
  std::vector<size_t>().swap(hist);

  // Communicate the requests
  std::vector<size_t> rec_req_counts;
  std::vector<pos_size_request> rec_req_positions;
  std::tie(rec_req_counts, rec_req_positions) =
    dpt::mpi::alltoallv_counts(pos_size_requests, counts);
  // Prepare the responses for the received requests (i.e., allocate memory and
  // compute the displacements).
  size_t response_size = 0;
  auto res_size_accu = [](size_t init, pos_size_request const& req) {
    return init + req.size;
  };
  std::accumulate(rec_req_positions.begin(), rec_req_positions.end(), 0,
                  res_size_accu);
  std::vector<Alphabet> response;
  response.reserve(response_size);
  std::vector<size_t> response_sizes(local_text.text_environment().size(), 0);
  size_t request_size = 0;
  size_t target_pe = 0;
  size_t request_number = 0;
  while (target_pe < rec_req_counts.size() && rec_req_counts[target_pe] == 0) {
    ++target_pe;
  }
  for (const auto& request : rec_req_positions) {
    std::copy_n(local_text.data_begin() + request.position, request.size,
      std::back_inserter(response));
    request_size += request.size;
    ++request_number;
    if (request_number >= rec_req_counts[target_pe]) {
      response_sizes[target_pe++] += request_size;
      request_number = 0;
      request_size = 0;
    }
  }

  std::vector<pos_size_request>().swap(rec_req_positions);
  std::vector<Alphabet> rec_characters =
    dpt::mpi::alltoallv(response, response_sizes);

  // Compute the results with the initially computed offsets.
  std::vector<Alphabet> result;
  result.reserve(rec_characters.size());

  for (size_t req = 0; req < text_positions.size(); ++req) {
    std::copy_n(
      rec_characters.begin() + start_pos_receiving[text_positions[req]],
      substring_lengths[req],
      std::back_inserter(result));
    start_pos_receiving[text_positions[req]] += substring_lengths[req];
  }
  return result;
} // request_substrings

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
std::pair<std::vector<Alphabet>, std::vector<Alphabet>> 
  collective_communication<Alphabet, GlobalIndex, LocalIndex>
  ::request_substrings_head(std::vector<GlobalIndex>& text_positions,
    const std::vector<LocalIndex>& substring_lengths,
    const partition& local_text) {

  // Compute the number of requests send to each PE and the size of the
  // substrings that we will receive from each PE.
  std::vector<size_t> hist(local_text.text_environment().size(), 0);
  std::vector<int32_t> hist_receiving(local_text.text_environment().size(), 0);
  for (size_t i = 0; i < text_positions.size(); ++i) {
    const int32_t pe = local_text.pe(text_positions[i]);
    ++hist[pe];
    hist_receiving[pe] += substring_lengths[i];
  }
  // Compute the displacements (offsets) of the request (that we send) of type
  // pos_size_request and the responses (that we receive) of type
  // std::vector<Alphabet>.
  std::vector<size_t> counts(hist);
  std::vector<int32_t> start_pos(hist.size(), 0);
  std::vector<int32_t> start_pos_receiving(hist_receiving.size(), 0);
  for (size_t i = 1; i < start_pos.size(); ++i) {
    start_pos[i] = start_pos[i - 1] + hist[i - 1];
    start_pos_receiving[i] =
      start_pos_receiving[i - 1] + hist_receiving[i - 1];
  }
  // Prepare the requests and store them in pos_size_request
  std::vector<pos_size_request> pos_size_requests(text_positions.size());
  for (size_t i = 0; i < text_positions.size(); ++i) {
    const auto pe_and_pos = local_text.pe_and_norm_position(text_positions[i]);
    text_positions[i] = pe_and_pos.pe;
    pos_size_requests[start_pos[pe_and_pos.pe]++] =
      pos_size_request { pe_and_pos.position, substring_lengths[i] };
  }
  std::vector<size_t>().swap(hist);
  // Communicate the requests
  std::vector<size_t> rec_req_counts;
  std::vector<pos_size_request> rec_req_positions;
  std::tie(rec_req_counts, rec_req_positions) =
    dpt::mpi::alltoallv_counts(pos_size_requests, counts);
  // Prepare the responses for the received requests (i.e., allocate memory and
  // compute the displacements).
  std::vector<Alphabet> response;
  std::vector<size_t> response_sizes(local_text.text_environment().size(), 0);
  size_t request_size = 0;
  size_t target_pe = 0;
  size_t request_number = 0;
  for (const auto& request : rec_req_positions) {
    std::copy_n(local_text.data_begin() + request.position, request.size,
      std::back_inserter(response));
    request_size += request.size;
    ++request_number;
    if (request_number >= rec_req_counts[target_pe]) {
      response_sizes[target_pe++] += request_size;
      request_number = 0;
      request_size = 0;
    }
  }

  std::vector<pos_size_request>().swap(rec_req_positions);
  std::vector<Alphabet> rec_characters =
    dpt::mpi::alltoallv(response, response_sizes);
  // Compute the results with the initially computed offsets.
  std::vector<Alphabet> result;
  std::vector<Alphabet> heads(text_positions.size(), 0);
  std::vector<Alphabet> tails;
  tails.reserve(rec_characters.size() - text_positions.size());

  for (size_t req = 0; req < text_positions.size(); ++req) {
    heads[req] = rec_characters[start_pos_receiving[text_positions[req]]++];
    if (substring_lengths[req] > 1) {
      std::copy_n(
        rec_characters.begin() + start_pos_receiving[text_positions[req]],
        substring_lengths[req] - 1,
        std::back_inserter(tails));
      start_pos_receiving[text_positions[req]] += substring_lengths[req] - 1;
    }
  }

  return std::make_pair(heads, tails);
} // request_substrings_head

template <typename Alphabet, typename GlobalIndex, typename LocalIndex>
dpt::query::query_list<Alphabet, GlobalIndex, LocalIndex>
  collective_communication<Alphabet, GlobalIndex, LocalIndex>
  ::distribute_queries(
    std::vector<Alphabet>& queries, std::vector<LocalIndex>& query_lengths,
    std::vector<size_t>& hist_lengths, std::vector<size_t>& hist) {

    std::vector<Alphabet> received_queries = dpt::mpi::alltoallv(
      queries, hist_lengths);
    std::vector<LocalIndex> recieved_lengths = dpt::mpi::alltoallv(
      query_lengths, hist);

    return dpt::query::query_list<Alphabet, GlobalIndex, LocalIndex>(
      std::move(received_queries), std::move(recieved_lengths));

} // distribute_queries

} // namespace com
} // namespace dpt

/******************************************************************************/
