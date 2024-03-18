#!/bin/bash

# SPDX-FileCopyrightText: 2006-2024 Knut Reinert & Freie Universität Berlin
# SPDX-FileCopyrightText: 2016-2024 Knut Reinert & MPI für molekulare Genetik
# SPDX-License-Identifier: BSD-3-Clause

source /project/archive-index-data/smehringer/benchmark.variables

OTH_DIR="${WORKDIR}/SeqOthello_bench"

# Build von file generated by ./gen

/project/archive-index-data/software/.paper/SeqOthello/build/bin/Build --flist=${OTH_DIR}/example/GrpList --folder=${OTH_DIR}/example/grp/ --out-folder=${OTH_DIR}/SeqOthello_index/ > ${OTH_DIR}/SeqOthello_index/Build.log

