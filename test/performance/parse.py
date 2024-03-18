#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2006-2024 Knut Reinert & Freie Universität Berlin
# SPDX-FileCopyrightText: 2016-2024 Knut Reinert & MPI für molekulare Genetik
# SPDX-License-Identifier: BSD-3-Clause

#
# Usage parse.py <input_file>
#
# Extracts relative slowdown from benchmark results.
# E.g. ./test/benchmark/bin_influence_benchmark --benchmark_repetitions=5 --benchmark_report_aggregates_only=true
#                                               --benchmark_out=results_minimiser.json --benchmark_out_format=json
import argparse
import json
import pandas

def create_data_frame(json_string, aggregate_name):
    assert aggregate_name == 'mean' or aggregate_name == 'median'
    with pandas.option_context('mode.chained_assignment', None):
        data = pandas.read_json(json_string, convert_dates=False)
        data = data[data.aggregate_name == aggregate_name]
        data = data.loc[:, ['run_name','cpu_time']]
        data.reset_index(drop=True, inplace=True)
        min_time = data['cpu_time'][0]
        data['Factor'] = data.apply(lambda row: round(row['cpu_time'] / min_time, 2), axis='columns')
        data['run_name'] = data['run_name'].apply(lambda x: x.split('/')[1])
        data.rename(columns={'run_name' : 'Bins', 'cpu_time' : 'Time [ns]'}, inplace=True)
        return data

def process_json(path, aggregate_name):
    assert aggregate_name == 'mean' or aggregate_name == 'median'
    with open(path, 'r') as json_file:
        benchmark_json_string = json.dumps(json.load(json_file)['benchmarks'])
    return create_data_frame(benchmark_json_string, aggregate_name)

parser = argparse.ArgumentParser(description='Extracts relative slowdown from benchmark results. E.g., from the file '
                                             'generated by calling `./test/benchmark/bin_influence_benchmark --benchm'
                                             'ark_repetitions=5 --benchmark_report_aggregates_only=true --benchmark_o'
                                             'ut=results_minimiser.json --benchmark_out_format=json`',
                                 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('input', type=str, help='JSON File from bin_influence_benchmark.')
arguments = parser.parse_args()

print(process_json(arguments.input, 'mean'))
print(process_json(arguments.input, 'median'))
