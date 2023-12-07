#include "TraceFile.h"
#include "SortState.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include "global.h"
#define MB 1000000LL                      // B
#define SSD_latency 0.1                   // ms
#define HDD_latency 10                    // ms
#define SSD_bandwidth (100 * MB * 1000LL) // B / ms
#define HDD_bandwidth 100000000000LL      // B / ms

std::string to_scientific_notation(long long num)
{
    std::ostringstream oss;
    oss << std::scientific << std::setprecision(1) << num;
    return oss.str();
}

TraceFile::TraceFile(const std::string &filename)
{
    file.open(filename, std::ios::out);
    if (!file.is_open())
    {
        // Handle the error, e.g., throw an exception or log an error
    }
    file << std::left; // Align text to the left
    file << "-----------------------------------------------------------------------------------\n";
    file << "| Phase            | Description                                                  |\n";
    file << "-----------------------------------------------------------------------------------\n";
    file << "| RUN_PHASE_1      | HDD  -> DRAM, Quicksort 1MB cache-sized runs                 |\n";
    file << "| RUN_PHASE_2      | DRAM -> SSD , Mergesort 1MB cache-sized runs to SSD          |\n";
    file << "| EXTERNAL_PHASE_1 | SSD  -> DRAM -> HDD, Merge sort 100MB DRAM-sized runs to HDD |\n";
    file << "| EXTERNAL_PHASE_2 | HDD  -> DRAM -> HDD, Merge sort 10GB SSD-sized runs to HDD   |\n";
    file << "-----------------------------------------------------------------------------------\n\n";
    file << "[TRACE START]";
    file << std::endl;
}

TraceFile::~TraceFile()
{
    if (file.is_open())
    {
        file.close();
    }
}

void TraceFile::trace(SortState state)
{

    long long HDD_page_size;
    long long SSD_page_size;
    double total_transfer_time;
    totalSize = 120000000000;

    if (!file.is_open())
    {
        // TODO: do something
    }
    else
    {
        file << "[" << sortStateToString(state) << "]: ";
        switch (state)
        {

        case RUN_PHASE_1:

        {
            HDD_page_size = 100 * MB;
            double total_latency_100MB_HDD = totalSize / HDD_page_size * HDD_latency;
            total_transfer_time = static_cast<double>(totalSize) / HDD_bandwidth;

            file << "total latency: " << total_latency_100MB_HDD << "ms;  ";
            file << "total transfer time: " << total_transfer_time << "ms" << std::endl;

            break;
        }

        case RUN_PHASE_2:
        {
            SSD_page_size = 100 * MB;
            double total_latency_100MB_SSD = totalSize / SSD_page_size * SSD_latency;
            total_transfer_time = static_cast<double>(totalSize) / SSD_bandwidth;

            file << "total latency: " << total_latency_100MB_SSD << "ms;  ";
            file << "total transfer time: " << total_transfer_time << "ms" << std::endl;
            break;
        }
        }
    }
}

std::string TraceFile::sortStateToString(SortState state)
{
    switch (state)
    {
    case RUN_PHASE_1:
        return "RUN_PHASE_1";
    case RUN_PHASE_2:
        return "RUN_PHASE_2";
    case EXTERNAL_PHASE_1:
        return "EXTERNAL_PHASE_1";
    case EXTERNAL_PHASE_2:
        return "EXTERNAL_PHASE_2";
    default:
        return "Unknown State";
    }
}
