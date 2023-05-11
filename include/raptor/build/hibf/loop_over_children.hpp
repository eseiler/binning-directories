// --------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2023, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2023, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/raptor/blob/main/LICENSE.md
// --------------------------------------------------------------------------------------------------

/*!\file
 * \brief Provides raptor::hibf::loop_over_children.
 * \author Enrico Seiler <enrico.seiler AT fu-berlin.de>
 */

#pragma once

#include <robin_hood.h>


#include <raptor/build/hibf/build_data.hpp>

namespace raptor::hibf
{

void loop_over_children(robin_hood::unordered_flat_set<size_t> & parent_kmers,
                        seqan3::interleaved_bloom_filter<> & ibf,
                        std::vector<int64_t> & ibf_positions,
                        lemon::ListDigraph::Node const & current_node,
                        build_data & data,
                        bool is_root);

} // namespace raptor::hibf
