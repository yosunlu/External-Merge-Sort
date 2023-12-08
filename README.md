# CS764

# Group members

- HUI-DAN CHANG 9084661785
- YUSHAN LU 9084761288
- YUNTZU CHEN 9083898305

# Techniques implemented and reasons

- Quicksort + Cache-size mini runs:
  - use quick sort for cache size (1MB) runs becacues quicksort is in-place
- Tournament trees + Offset-value coding:
  - enable the most efficient sort algorithms for large records
- Minimum count of row & column comparisons
  - enabled by Offset-value coding
- Device-optimized page sizes
  - carfully selected different page sizes for different stages to utilize bandwidths and minimize latency
- Spilling memory to SSD + Spilling from SSD to disk
  - used for external mergesort
- Graceful degradation:
  - for data size that exceeds only 25% of memory or SSD, lower I/Os by only spilling that extra data to the next level
- Beyond one merge step
  - use quicksort for in-place sorting; use external mergesort for merging from SSD -> DRAM -> HDD and HDD -> DRAM -> HDD
- Optimized merge patterns
- Verifying: sets of rows & values + sort order
  - use CRC-32 (Cyclic Redundancy Check 32), a checksum algorithm

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
