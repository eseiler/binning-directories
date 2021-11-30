// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2021, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2021, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/raptor/blob/master/LICENSE.md
// -----------------------------------------------------------------------------------------------------

#include "../cli_test.hpp"

struct search_hibf : public raptor_base,
                     public testing::WithParamInterface<std::tuple<size_t, size_t, size_t>> {};

TEST_P(search_hibf, with_error)
{
    auto const [number_of_repeated_bins, window_size, number_of_errors] = GetParam();

    if (window_size == 23 && number_of_errors == 0)
        GTEST_SKIP() << "Needs dynamic threshold correction";

    cli_test_result const result = execute_app("raptor", "search",
                                                         "--output search.out",
                                                         "--error ", std::to_string(number_of_errors),
                                                         "--hibf",
                                                         "--index ", ibf_path(number_of_repeated_bins,
                                                                              window_size,
                                                                              is_compressed::no,
                                                                              is_hibf::yes),
                                                         "--query ", data("query.fq"));
    EXPECT_EQ(result.out, std::string{});
    EXPECT_EQ(result.err, std::string{});
    RAPTOR_ASSERT_ZERO_EXIT(result);

    compare_search(number_of_repeated_bins, number_of_errors, "search.out");
}

TEST_P(search_hibf, with_threshold)
{
    auto const [number_of_repeated_bins, window_size, number_of_errors] = GetParam();

    cli_test_result const result = execute_app("raptor", "search",
                                                         "--output search.out",
                                                         "--threshold 0.50",
                                                         "--hibf",
                                                         "--index ", ibf_path(number_of_repeated_bins,
                                                                              window_size,
                                                                              is_compressed::no,
                                                                              is_hibf::yes),
                                                         "--query ", data("query.fq"));
    EXPECT_EQ(result.out, std::string{});
    EXPECT_EQ(result.err, std::string{});
    RAPTOR_ASSERT_ZERO_EXIT(result);

    compare_search(number_of_repeated_bins, 1 /* Always finds everything */, "search.out");
}

TEST_P(search_hibf, no_hits)
{
    auto const [number_of_repeated_bins, window_size, number_of_errors] = GetParam();

    if (window_size == 23 && number_of_errors == 0)
        GTEST_SKIP() << "Needs dynamic threshold correction";

    cli_test_result const result = execute_app("raptor", "search",
                                                         "--output search.out",
                                                         "--error ", std::to_string(number_of_errors),
                                                         "--hibf",
                                                         "--index ", ibf_path(number_of_repeated_bins,
                                                                              window_size,
                                                                              is_compressed::no,
                                                                              is_hibf::yes),
                                                         "--query ", data("query_empty.fq"));
    EXPECT_EQ(result.out, std::string{});
    EXPECT_EQ(result.err, std::string{});
    RAPTOR_ASSERT_ZERO_EXIT(result);

    compare_search(number_of_repeated_bins, number_of_errors, "search.out", is_empty::yes);
}

INSTANTIATE_TEST_SUITE_P(
    search_hibf_suite,
    search_hibf,
    testing::Combine(testing::Values(0, 16, 32), testing::Values(19), testing::Values(0, 1)),
    [] (testing::TestParamInfo<search_hibf::ParamType> const & info)
    {
        std::string name = std::to_string(std::max<int>(1, std::get<0>(info.param) * 4)) + "_bins_" +
                           std::to_string(std::get<1>(info.param)) + "_window_" +
                           std::to_string(std::get<2>(info.param)) + "_error";
        return name;
    });

TEST_F(search_hibf, three_levels)
{
    cli_test_result const result = execute_app("raptor", "search",
                                                         "--output search.out",
                                                         "--error 0",
                                                         "--hibf",
                                                         "--index ", data("three_levels.hibf"),
                                                         "--query ", data("query.fq"));
    EXPECT_EQ(result.out, std::string{});
    EXPECT_EQ(result.err, std::string{});
    RAPTOR_ASSERT_ZERO_EXIT(result);

    compare_search(32, 0, "search.out");
}
