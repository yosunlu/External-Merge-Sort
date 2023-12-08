# CS764

# Group members

- HUI-DAN CHANG 9084661785
- YUSHAN LU 9084761288
- YUNTZU CHEN 9083898305

# Techniques implemented and reasons

- Quicksort + Cache-size mini runs:
  - use quick sort for cache size (1MB) runs becacues quicksort is in-place
- Tournament trees + Offset-value coding:
  - Leverages tree-of-losers tress; the combination of this and OVC enables the most efficient sort algorithms for large records
- Minimum count of row & column comparisons
  - enabled by offset-value coding
- Device-optimized page sizes
  - carfully selected different page sizes for different stages to utilize bandwidths and minimize latency
- Spilling memory to SSD + Spilling from SSD to disk
  - used for external mergesort
- Beyond one merge step
  - use quicksort for in-place sorting; use external mergesort for merging from SSD -> DRAM -> HDD and HDD -> DRAM -> HDD
- Verifying - sets of rows & values + sort order
  - use CRC-32 (Cyclic Redundancy Check 32), a checksum algorithm
- Graceful degradation:
  - for data size that exceeds only 25% of memory or SSD, lower I/Os by only spilling that extra size of data to the next level
  - For 125MB total data size, we first sort 100MB, then output 25MB of data to SSD. Next, we input and sort the exeeding 25MB. The fan-in for DRAM mergesort is 125; the first 100 buckets each has 750KB/250KB of data in DRAM/SSD, while the last 25 buckets each has 1MB in DRAM. With this technique, we lowered 50% of total I/O (500 MB -> 250MB)

# Project's state

- complete and can be tested with default specification

# Each group member's contributions

- HUI-DAN CHANG: tree-of-losers, OVC, external merge sort, variable size record, graceful degradation
- YUSHAN LU: external merge sort, variable size record, tracfile, README
- YUNTZU CHEN:

README file that includes:

    Each group member's names and student IDs

    The techniques (see Grading) implemented by your submission and the corresponding source files and lines

    A brief writeup of (1) the reasons you chose to implement the specific subset of techniques (2) the project's state (complete or have what kinds of bugs) (3) how to run your programs if they are different from the default specifications above. This is not expected to be a lengthy document, just to assist our understandings of your work.

    Each group member's contributions to the project
