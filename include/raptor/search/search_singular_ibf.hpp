// --------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2023, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2023, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/raptor/blob/main/LICENSE.md
// --------------------------------------------------------------------------------------------------

/*!\file
 * \brief Provides raptor::search_singular_ibf.
 * \author Enrico Seiler <enrico.seiler AT fu-berlin.de>
 */

#pragma once

#include <seqan3/search/views/minimiser_hash.hpp>

#include <raptor/adjust_seed.hpp>
#include <raptor/dna4_traits.hpp>
#include <raptor/search/do_parallel.hpp>
#include <raptor/search/load_index.hpp>
#include <raptor/search/sync_out.hpp>
#include <raptor/threshold/threshold.hpp>

namespace raptor
{

template <typename index_t>
void search_singular_ibf(search_arguments const & arguments, index_t && index)
{
    constexpr bool is_ibf = std::same_as<index_t, raptor_index<index_structure::ibf>>
                         || std::same_as<index_t, raptor_index<index_structure::ibf_compressed>>;

    auto cereal_worker = [&]()
    {
        load_index(index, arguments);
    };
    auto cereal_handle = std::async(std::launch::async, cereal_worker);

    seqan3::sequence_file_input<dna4_traits, seqan3::fields<seqan3::field::id, seqan3::field::seq>> fin{
        arguments.query_file};
    using record_type = typename decltype(fin)::record_type;
    std::vector<record_type> records{};

    sync_out synced_out{arguments};

    raptor::threshold::threshold const thresholder{arguments.make_threshold_parameters()};

    auto worker = [&](size_t const start, size_t const end)
    {
        timer<concurrent::no> local_compute_minimiser_timer{};
        timer<concurrent::no> local_query_ibf_timer{};
        timer<concurrent::no> local_generate_results_timer{};

        auto counter = [&index]()
        {
            if constexpr (is_ibf)
                return index.ibf().template counting_agent<uint16_t>();
            else
                return index.ibf().membership_agent();
        }();
        std::string result_string{};
        std::vector<uint64_t> minimiser;

        auto hash_adaptor = seqan3::views::minimiser_hash(arguments.shape,
                                                          seqan3::window_size{arguments.window_size},
                                                          seqan3::seed{adjust_seed(arguments.shape_weight)});

        for (auto && [id, seq] : seqan3::views::slice(records, start, end))
        {
            result_string.clear();
            result_string += id;
            result_string += '\t';

            auto minimiser_view = seq | hash_adaptor | std::views::common;
            local_compute_minimiser_timer.start();
            minimiser.assign(minimiser_view.begin(), minimiser_view.end());
            local_compute_minimiser_timer.stop();

            size_t const minimiser_count{minimiser.size()};
            size_t const threshold = thresholder.get(minimiser_count);

            if constexpr (is_ibf)
            {
                local_query_ibf_timer.start();
                auto & result = counter.bulk_count(minimiser);
                local_query_ibf_timer.stop();
                size_t current_bin{0};
                local_generate_results_timer.start();
                for (auto && count : result)
                {
                    if (count >= threshold)
                    {
                        result_string += std::to_string(current_bin);
                        result_string += ',';
                    }
                    ++current_bin;
                }
            }
            else
            {
                local_query_ibf_timer.start();
                auto & result = counter.bulk_contains(minimiser, threshold); // Results contains user bin IDs
                local_query_ibf_timer.stop();
                local_generate_results_timer.start();
                for (auto && count : result)
                {
                    result_string += std::to_string(count);
                    result_string += ',';
                }
            }

            if (auto & last_char = result_string.back(); last_char == ',')
                last_char = '\n';
            else
                result_string += '\n';

            synced_out.write(result_string);
            local_generate_results_timer.stop();
        }

        arguments.compute_minimiser_timer += local_compute_minimiser_timer;
        arguments.query_ibf_timer += local_query_ibf_timer;
        arguments.generate_results_timer += local_generate_results_timer;
    };

    auto write_header = [&]()
    {
        if constexpr (is_ibf)
            return synced_out.write_header(arguments, index.ibf().hash_function_count());
        else
            return synced_out.write_header(arguments, index.ibf().ibf_vector[0].hash_function_count());
    };

    for (auto && chunked_records : fin | seqan3::views::chunk((1ULL << 20) * 10))
    {
        records.clear();
        arguments.query_file_io_timer.start();
        std::ranges::move(chunked_records, std::back_inserter(records));
        arguments.query_file_io_timer.stop();

        cereal_handle.wait();
        [[maybe_unused]] static bool header_written = write_header(); // called exactly once

        do_parallel(worker, records.size(), arguments.threads);
    }
}

} // namespace raptor
