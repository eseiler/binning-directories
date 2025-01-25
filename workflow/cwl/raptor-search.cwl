#!/usr/bin/env cwl-runner
# SPDX-FileCopyrightText: 2006-2024 Knut Reinert & Freie Universität Berlin
# SPDX-FileCopyrightText: 2016-2024 Knut Reinert & MPI für molekulare Genetik
# SPDX-License-Identifier: CC0-1.0
cwlVersion: v1.2
class: CommandLineTool

label: search reads

baseCommand: [ raptor, search ]

inputs:
  index:
    type: File
    inputBinding:
       prefix: --index
  queries:
    type: File
    inputBinding:
       prefix: --query
    format:
      - edam:format_1929  # FASTA
      - edam:format_1930  # FASTQ
  error_tolerance:
    type: int?
    label: The number of errors to tolerate
    doc: |
      Default: 0. Value must be a positive integer or 0.
    inputBinding:
      prefix: --error
  output_name:
    type: string
    label: name of the search results to produce
    default: raptor.search.output
    inputBinding:
      prefix: --output
  p_max:
    type: double?
    label: Used in the dynamic thresholding
    doc: |
      The higher p_max, the lower the threshold. Default: 0.15. Value must be in
      the range [0,1]
    inputBinding:
      prefix: --p_max

hints:
  SoftwareRequirement:
    packages:
      raptor:
        specs: [ https://bio.tools/raptor ]
  cwltool:ParameterRestrictions:
    restrictions:
      error_tolerance:
        - class: intInterval
          low: 0
          # high default value is positive infinity
      p_max:
        - class: interval
          low: 0
          high: 1

requirements:
  EnvVarRequirement:
    envDef:
      SHARG_NO_VERSION_CHECK: "1"

arguments:
  - prefix: --threads
    valueFrom: $(runtime.cores)

outputs:
  results:
    type: File
    label: Search results
    doc: |
      Format: query id + tab + comma separated list of bin numbers that contain matches
    outputBinding:
      glob: $(inputs.output_name)

$namespaces:
  edam: http://edamontology.org/
  cwltool: "http://commonwl.org/cwltool#"
