# Intro
This is s course work from UW-Madison cs764 - Advanced Topics in Datrabase management. This project happend to be quite challenging, and it involed several techniques to fufill the requirements.

# Goal
- The goal is to implement an external merge sort. We were asked to emulate SSD, HDD, CPU, and DRAM. We were also asked to generate 120 GB of data, which consists of 120 millions of 1KB entries, in the form of random numbers and letters. The implemented merge sort has to sort these 120 millons correctly and time-efficient
- Constraints are the hardware:
  - 1 CPU
  - 1 MB cache
  - 100 MB DRAM
  - 10 GB SSD
  - HDD with unlimited storage   

# Techniques implemented / Reasonings / Corresponding source files

- Quicksort + Cache-size mini runs
  - use quick sort for cache size (1MB) runs since quicksort is in-place
  - Sort.cpp:78~113
- Tournament trees + Offset-value coding
  - the combination of tree-of-losers and OVC enables the most efficient sort algorithms for large records
  - PQ.cpp
- Minimum count of row & column comparisons
  - enabled by offset-value coding
- Device-optimized page sizes
  - carefully selected different page sizes for different stages to utilize bandwidths and minimize latency
  - Sort.cpp: 8KB vs 1MB in different phases
- Spilling memory to SSD + Spilling from SSD to disk
  - used for external mergesort
- Beyond one merge step
  - use quicksort for in-place sorting; use external mergesort for merging from SSD -> DRAM -> HDD and HDD -> DRAM -> HDD
- Verifying - sets of rows & values + sort order
  - use CRC-32 (Cyclic Redundancy Check 32), a checksum algorithm
  - Verify.cpp
- Graceful degradation:
  - for data size that exceeds only 25% of memory or SSD, lower I/Os by only spilling that extra size of data to the next level
  - For 125MB total data size, we first sort 100MB, then output 25MB of data to SSD. Next, we input and sort the exeeding 25MB. The fan-in for DRAM mergesort is 125; the first 100 buckets each has 750KB/250KB of data in DRAM/SSD, while the last 25 buckets each has 1MB in DRAM. With this technique, we lowered 50% of total I/O (500 MB -> 250MB)
  - Test.cpp:462\~530, 613\~694, Sort.cpp:117\~310, 390\~691

# Verificaiton
- `./verify.exe -i input/input.txt -o output/final_output.txt -s [record size]`
