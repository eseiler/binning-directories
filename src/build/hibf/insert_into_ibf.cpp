// --------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2023, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2023, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/raptor/blob/main/LICENSE.md
// --------------------------------------------------------------------------------------------------

/*!\file
 * \brief Implements raptor::hibf::insert_into_ibf.
 * \author Enrico Seiler <enrico.seiler AT fu-berlin.de>
 */

#include <raptor/build/hibf/insert_into_ibf.hpp>
#include <raptor/contrib/std/chunk_view.hpp>

namespace raptor::hibf
{

// automatically does naive splitting if number_of_bins > 1
void insert_into_ibf(robin_hood::unordered_flat_set<uint64_t> const & kmers,
                     size_t const number_of_bins,
                     size_t const bin_index,
                     seqan3::interleaved_bloom_filter<> & ibf,
                     timer<concurrent::yes> & fill_ibf_timer)
{
    size_t const chunk_size = kmers.size() / number_of_bins + 1;
    size_t chunk_number{};

    timer<concurrent::no> local_fill_ibf_timer{};
    local_fill_ibf_timer.start();
    for (auto chunk : kmers | seqan::std::views::chunk(chunk_size))
    {
        assert(chunk_number < number_of_bins);
        seqan3::bin_index const bin_idx{bin_index + chunk_number};
        ++chunk_number;
        for (size_t const value : chunk)
            ibf.emplace(value, bin_idx);
    }
    local_fill_ibf_timer.stop();
    fill_ibf_timer += local_fill_ibf_timer;
}

void insert_into_ibf(build_data const & data,
                     chopper::layout::layout::user_bin const & record,
                     seqan3::interleaved_bloom_filter<> & ibf)
{
    auto const bin_index = seqan3::bin_index{static_cast<size_t>(record.storage_TB_id)};
    robin_hood::unordered_flat_set<uint64_t> values;

    timer<concurrent::no> local_user_bin_io_timer{};
    local_user_bin_io_timer.start();
    data.input_fn(record.idx, values);
    local_user_bin_io_timer.stop();
    data.arguments.user_bin_io_timer += local_user_bin_io_timer;

    timer<concurrent::no> local_fill_ibf_timer{};
    local_fill_ibf_timer.start();
    for (auto && value : values)
        ibf.emplace(value, bin_index);
    local_fill_ibf_timer.stop();
    data.arguments.fill_ibf_timer += local_fill_ibf_timer;
}

} // namespace raptor::hibf
