#include "TraceFile.h"
#include "SortState.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include "global.h"
#define MB 1000000LL                      // B
#define KB 1000LL                         // B
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
    file << "----------------------------------------------------------------------------------------\n";
    file << "| Phase              | Description                                                     |\n";
    file << "----------------------------------------------------------------------------------------\n";
    file << "| RUN_PHASE_1        | HDD  -> DRAM, READ from HDD and QUICKSORT 1MB cache-sized runs  |\n";
    file << "| RUN_PHASE_2        | DRAM -> SSD,  MERGESORT 1MB cache-sized runs and WRTIE to SSD   |\n";
    file << "| EXTERNAL_PHASE_1_1 | SSD  -> DRAM, READ from SSD and MERGESORT 100MB DRAM-sized runs |\n";
    file << "| EXTERNAL_PHASE_1_2 | DRAM -> HDD,  WRITE 10GB SSD-sized runs to HDD                  |\n";
    file << "| EXTERNAL_PHASE_2_1 | HDD  -> DRAM, READ from HDD and MERGESORT 10GB SSD-sized runs   |\n";
    file << "| EXTERNAL_PHASE_2_2 | DRAM -> HDD,  WRITE final output to HDD                         |\n";
    file << "----------------------------------------------------------------------------------------\n";
    file << "- total latency = # of pages * (latency + transfer time)\n";
    file << "- transfer time = page size / bandwidth\n";
    file << "- SSD latency: 0.1ms; HDD latency: 10ms\n";
    file << "- SSD bandwidth: 100MB/ms; HDD bandwidth: 100MB/ms\n\n";
    file << "[TRACE START] ...";
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
    long double numOfPage;
    double singlePageTransferTime;

    if (!file.is_open())
    {
        // TODO: do something
    }
    else
    {

        switch (state)
        {
        case RUN_PHASE_1:

        {

            HDD_page_size = 100 * MB;
            numOfPage = static_cast<long double>(totalSize) / HDD_page_size;
            singlePageTransferTime = static_cast<double>(HDD_page_size) / HDD_bandwidth;
            double total_latency_100MB_HDD = numOfPage * (HDD_latency + singlePageTransferTime);

            file << "[" << sortStateToString(state) << "]: ";
            file << "HDD page size: " << HDD_page_size / 1000000 << "MB;  ";
            file << "latency: " << numOfPage * HDD_latency << "ms;  ";
            file << "transfer time: " << numOfPage * singlePageTransferTime << "ms;  ";
            file << "total " << total_latency_100MB_HDD << "ms\n";

            break;
        }

        case RUN_PHASE_2:
        {
            SSD_page_size = 100 * MB;
            numOfPage = static_cast<long double>(totalSize) / SSD_page_size;
            singlePageTransferTime = static_cast<double>(SSD_page_size) / SSD_bandwidth;
            double total_latency_100MB_SSD = numOfPage * (SSD_latency + singlePageTransferTime);

            file << "[" << sortStateToString(state) << "]: ";
            file << "SSD page size: " << SSD_page_size / 1000000 << "MB;  ";
            file << "latency: " << numOfPage * SSD_latency << "ms;  ";
            file << "transfer time: " << numOfPage * singlePageTransferTime << "ms;  ";
            file << "total: " << total_latency_100MB_SSD << "ms\n";

            break;
        }

        case EXTERNAL_PHASE_1_1:
        {
            SSD_page_size = 8 * KB;
            numOfPage = static_cast<double>(totalSize) / SSD_page_size;
            singlePageTransferTime = static_cast<double>(SSD_page_size) / SSD_bandwidth;
            double total_latency_8MB_SSD = numOfPage * (SSD_latency + singlePageTransferTime);

            file << "[" << sortStateToString(state) << "]: ";
            file << "SSD page size: " << SSD_page_size / 1000 << "KB;  ";
            file << "latency: " << SSD_latency * numOfPage << "ms;  ";
            file << "transfer time: " << numOfPage * singlePageTransferTime << "ms;  ";
            file << "total: " << total_latency_8MB_SSD << "ms\n";

            // SSD_page_size = 8 * KB;
            // numOfPage = totalSize / SSD_page_size;
            // singlePageTransferTime = SSD_page_size / SSD_bandwidth;
            // double total_latency_100MB_SSD = numOfPage * (SSD_latency + singlePageTransferTime);

            // file << "total latency: " << total_latency_100MB_SSD << "ms\n";
            break;
        }

        case EXTERNAL_PHASE_1_2:
        {
            HDD_page_size = 1 * MB;
            numOfPage = static_cast<double>(totalSize) / HDD_page_size;
            singlePageTransferTime = static_cast<double>(HDD_page_size) / HDD_bandwidth;
            double total_latency_1MB_HDD = numOfPage * (HDD_latency + singlePageTransferTime);

            file << "[" << sortStateToString(state) << "]: ";
            file << "HDD page size: " << HDD_page_size / 1000000 << "MB;  ";
            file << "latency: " << numOfPage * HDD_latency << "ms;  ";
            file << "transfer time: " << numOfPage * singlePageTransferTime << "ms;  ";
            file << "total: " << total_latency_1MB_HDD << "ms\n";

            // HDD_page_size = 1 * MB;
            // numOfPage = totalSize / HDD_page_size;
            // singlePageTransferTime = HDD_page_size / HDD_bandwidth;
            // double total_latency_1MB_HDD = numOfPage * (HDD_latency + singlePageTransferTime);

            // file << "total latency: " << total_latency_1MB_HDD << "ms\n";
            break;
        }

        case EXTERNAL_PHASE_2_1:
        {
            HDD_page_size = 1 * MB;
            numOfPage = static_cast<double>(totalSize) / HDD_page_size;
            singlePageTransferTime = static_cast<double>(HDD_page_size) / HDD_bandwidth;
            double total_latency_1MB_HDD = numOfPage * (HDD_latency + singlePageTransferTime);

            file << "[" << sortStateToString(state) << "]: ";
            file << "HDD page size: " << HDD_page_size / 1000000 << "MB;  ";
            file << "latency: " << numOfPage * HDD_latency << "ms;  ";
            file << "transfer time: " << numOfPage * singlePageTransferTime << "ms;  ";
            file << "total: " << total_latency_1MB_HDD << "ms\n";

            break;
        }

        case EXTERNAL_PHASE_2_2:
        {
            HDD_page_size = 1 * MB;
            numOfPage = static_cast<double>(totalSize) / HDD_page_size;
            singlePageTransferTime = static_cast<double>(HDD_page_size) / HDD_bandwidth;
            double total_latency_1MB_HDD = numOfPage * (HDD_latency + singlePageTransferTime);

            file << "[" << sortStateToString(state) << "]: ";
            file << "HDD page size: " << HDD_page_size / 1000000 << "MB;  ";
            file << "latency: " << numOfPage * HDD_latency << "ms;  ";
            file << "transfer time: " << numOfPage * singlePageTransferTime << "ms;  ";
            file << "total: " << total_latency_1MB_HDD << "ms\n";

            break;
        }
        case END:
        {
            file << "[" << sortStateToString(state) << "]";
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
    case EXTERNAL_PHASE_1_1:
        return "EXTERNAL_PHASE_1_1";
    case EXTERNAL_PHASE_1_2:
        return "EXTERNAL_PHASE_1_2";
    case EXTERNAL_PHASE_2_1:
        return "EXTERNAL_PHASE_2_1";
    case EXTERNAL_PHASE_2_2:
        return "EXTERNAL_PHASE_2_2";
    case END:
        return "TRACE END";
    default:
        return "Unknown State";
    }
}

// [RUN_PHASE_1]: total latency: 12000ms;  total transfer time: 1.2ms