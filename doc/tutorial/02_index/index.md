# Indexing with Raptor {#tutorial_index}

You will learn how to construct a Raptor index of large collections of nucleotide sequences.

\tutorial_head{Easy, 30 min, \ref tutorial_first_steps,
[<b>Interleaved Bloom Filter (IBF)</b>](https://docs.seqan.de/seqan/3-master-user/classseqan3_1_1interleaved__bloom__filter.html#details)\,
\ref raptor::hierarchical_interleaved_bloom_filter "Hierarchical Interleaved Bloom Filter (HIBF)"}

[TOC]

# Indexing large collections of nucleotide sequences

## A first index

Raptor consists of two methods, `build` and `search`. The former creates an index over a given dataset so that it can be
searched efficiently with `raptor search`.
In this tutorial, we will look at `raptor build` in more detail.

Raptor can be used as a pre-filter for applications where searching the complete dataset is not feasible. For example,
in read mapping you might want to compare your genome to the genome of 100'000 other people.
It can also be used for metagenomic classification, i.e., which microbes are in a single sample.

\todo link needle for rna-seq expression analysis, ganon/ganon2 for metagenomics

Regardless of the application, we start with a data set of different nucleotide sequences over which we want to build an
index.

These sequences are typically available as FASTA or FASTQ files, but can be in any format supported by \todo seqan3 sequence_file_input blah.
\todo example of this list. Maybe use some toy dataset?
We summarise these in a list of paths and can then create a first index using the default values of raptor:
```bash
raptor build --output raptor.index all_bin_paths.txt
```

\note
Raptor also has a help page, which can be accessed as usual by typing `raptor build -h` or `raptor build --help`. And
also an advanced help page: `raptor build -hh` or `raptor build --advanced-help`.

## General Idea
\image html ibf.svg width=90%

Before explaining parameters, we would like to briefly explain the general idea of the Raptor index.

If we want to check whether a query is contained in a sample, we can use a
[Bloom Filter (BF)](https://en.wikipedia.org/wiki/Bloom_filter) to create an index for the sample. Although this only
gives us a probability, it saves us a time-consuming complete mapping.
If our data set consists of many samples, there is a BF for each sample. By default, Raptor uses an
[Interleaved Bloom Filter (IBF)](https://docs.seqan.de/seqan/3-master-user/classseqan3_1_1interleaved__bloom__filter.html#details),
which is an efficient way to store these many Bloom Filters, called raptor index. Another possibility is to use the
Hierarchical Interleaved Bloom Filter (HIBF), more about this later in \ref hibf.

## Using parameters

\todo might be better in general idea?
To create the index, the individual samples of the data set are chopped up into k-mers and determine in their so-called
bin the specific bit setting of the Bloom Filter by passing them through the hash functions. This means that a k-mer
from sample `i` marks in bin `i` with `j` hash functions `j` bits with a `1`.
If a query is then searched, its k-mers are thrown into the hash functions and looked at in which bins it only points
to ones. This can also result in false positives. Thus, the result only indicates that the query is probably part of a
sample.

With `--kmer` you can specify the length of the k-mers, which should be long enough to avoid random hits.
By using multiple hash functions, you can further reduce the possibility of false positives (`--hash`).
\todo not necessarily https://hur.st/bloomfilter/

\todo might be better in general idea?
Each Bloom Filter has a bit vector length, which over all Bloom Filters gives the size of the Interleaved Bloom Filter,
which we specify with `--size`. We can therefore specify how much space the bins take up in total, whereby the following
also applies here: the larger the bins, the fewer false positives.

```bash
raptor build --kmer 19 --hash 3 --size 8m --output raptor.index all_bin_paths.txt
```

## Using minimisers

The k-mers can also be saved as [minimisers](https://docs.seqan.de/seqan/3-master-user/group__views.html#ga191fcd1360fc430441567f3ed0f371d1),
which saves space. To do this, first call `raptor build` with `--compute-minimiser` and then with the minimiser paths. A
minimiser works with windows, which means that you also have to define their size `--window`. A window is always larger
than a k-mer because it combines several k-mers.

```bash
raptor build --kmer 19 --window 23 --compute-minimiser --output precomputed_minimisers all_bin_paths.txt
raptor build --size 8m --output minimiser_raptor.index all_minimiser_paths.txt
```

To save additional space, a cutoff discards all minimisers that do not occur frequently enough. This can be disabled
with `--disable-cutoffs` to reduce the false positive rate.

\note
If you want to learn more about minimisers, take a look at the SeqAn3 tutorial for
[minimisers](https://docs.seqan.de/seqan/3-master-user/tutorial_minimiser.html).

### Advanced

When hashing a sequence, there may be positions that do not count towards the final hash value. A
[shape](https://docs.seqan.de/seqan/3-master-user/classseqan3_1_1shape.html) offers an easy way to define such patterns:
`--shape`.

## IBF vs HIBF {#hibf}

Raptor works with the Interleaved Bloom Filter by default. A new feature is the Hierarchical Interleaved Bloom Filter
(HIBF) (raptor::hierarchical_interleaved_bloom_filter), which can be used with `--hibf` can be used. This uses a more
space-saving method of storing the bins. It distinguishes between the user bins, which reflect the individual samples as
before, and the so-called technical bins, which throw some bins together. This is especially useful when there are
samples of very different sizes.
Since the HIBF calculates the size of the index itself, it is no longer possible to specify a size here. But we can
offer the option to name the desired false positive rate with `--fpr`.

```bash
raptor build --kmer 19 --hash 3 --hibf --fpr 0.1 --output raptor.index all_bin_paths.txt
```

\note
For a detailed explanation of the Hierarchical Interleaved Bloom Filter (HIBF), please refer to the `raptor::hierarchical_interleaved_bloom_filter` API.

## Others

### Compressed index

The raptor index can also be stored in a compressed form (`--compressed`). This is only worthwhile if you want a very
low false positive rate, because then you have many bits equal 0 that can be compressed well.

### Parallelization

Raptor supports parallelization. By specifying `--threads`, for example, the fastq-records are processed simultaneously.

### Partitioned indices

To reduce the overall memory consumption of the search, the index can be divided into multiple (a power of two) parts.
This can be done by passing `--parts n`, where n is the number of parts you want to create. This will create `n` files,
each representing one part of the index.
The `--size` parameter describes the overall size of the index. For example, `--size 8g --parts 4` will create four
2 GiB indices. This will reduce the memory consumption of `raptor build` and `raptor search` by approximately 6 GiB,
since there will only be one part in memory at any given time. `raptor search` will automatically detect the parts, and
does not need any special parameters.