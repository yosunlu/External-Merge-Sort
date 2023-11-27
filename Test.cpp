#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "gen.h"
#include "SortState.h"
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include "DataRecord.h"
#include <fstream>
#include "Dram.h"
#include <vector>

long numOfRecord = 0;
long record_size = 0;

int main(int argc, char *argv[])
{
	TRACE(true);
	int opt;
	std::string output_file; // the trace file

	// Using getopt to parse command line arguments
	while ((opt = getopt(argc, argv, "c:s:o:")) != -1)
	{
		switch (opt)
		{
		case 'c':
			// number following -c will be the number of total records
			numOfRecord = std::atol(optarg);
			break;
		case 's':
			record_size = std::atoi(optarg);
			break;
		case 'o':
			output_file = optarg;
			break;
		default: /* '?' */
			std::cerr << "Usage: " << argv[0] << " -c records -s size -o output_file" << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	// Check if all required arguments are provided
	if (numOfRecord == 0 || record_size == 0 || output_file.empty())
	{
		std::cerr << "All options -c, -s, and -o must be provided" << std::endl;
		exit(EXIT_FAILURE);
	}

	// create input directory
	const char *directoryName = "input";
	struct stat info;

	// Check if the directory exists
	if (stat(directoryName, &info) != 0)
	{
		// Directory does not exist, try to create it
		int dirCreationResult = mkdir(directoryName, 0777);
		if (dirCreationResult == 0)
		{
			std::cout << "Directory created successfully." << std::endl;
		}
		else
		{
			std::cerr << "Error creating directory." << std::endl;
		}
	}
	else if (info.st_mode & S_IFDIR)
	{
		// Directory exists
		std::cout << "Directory already exists." << std::endl;
	}
	else
	{
		// Path exists but is not a directory
		std::cerr << "Path exists but is not a directory." << std::endl;
	}

	// create SSD directory
	directoryName = "SSD-10GB";

	// Check if the directory exists
	if (stat(directoryName, &info) != 0)
	{
		// Directory does not exist, try to create it
		int dirCreationResult = mkdir(directoryName, 0777);
		if (dirCreationResult == 0)
		{
			std::cout << "SSD created successfully." << std::endl;
		}
		else
		{
			std::cerr << "Error creating SSD." << std::endl;
		}
	}
	else if (info.st_mode & S_IFDIR)
	{
		// Directory exists
		std::cout << "SSD already exists." << std::endl;
	}
	else
	{
		// Path exists but is not a directory
		std::cerr << "Path exists but is not a directory." << std::endl;
	}

	// create records that will be saved in the input file

	// assuming 10GB data; 1KB (per record) * 1000 (quicksort these  1000 record, 1MB) * 100 (total 100MB) * 100 = 10G
	genDataRecords(numOfRecord); // 10GB data = 1000 * 100 * 100 records = 10,000,000 records

	// int numBatch = numOfRecord / 1000;

	std::ifstream inputFile("input/input.txt", std::ios::binary);

	if (!inputFile.is_open())
		std::cerr << "Error opening input file." << std::endl;

	for (int i = 0; i < 10; i++) // 100 * 100MB dataRecords
	{
		// each iteration will push_back one pointer to 1000 records to dataRecords; when the for loop ends, there will be one dataRecord which is 100MB
		for (int j = 0; j < 100; ++j)
		{
			Plan *const plan = new SortPlan(new ScanPlan(1000), RUN_PHASE_1, &inputFile, 0); // quick sort 1MB of data and repeat 100 times
			Iterator *const it = plan->init();
			it->run();

			delete it;
			delete plan;
		}

		Plan *const plan = new SortPlan(new ScanPlan(100000), RUN_PHASE_2, nullptr, i);
		Iterator *const it = plan->init();
		it->run();

		for (int i = 0; i < 100; ++i)
		{
			dataRecords[i].clear();
		}

		delete it;
		delete plan;
	}

	inputFile.close();

	// Plan * const plan = new FilterPlan ( new ScanPlan (7) );
	// new SortPlan ( new FilterPlan ( new ScanPlan (7) ) );

	// SortPlan has private attribute _input, which is intialized to a ScanPlan.
	// In SortPlan's init(), a SortIterator(this) is constructed and returned.
	// In SortIterator's constructor, the object itself is passed in as argument.
	// The object itself's private attribute _input is used to initialized SortIterator's private attributes.
	// In the constructor, the object's _input (a ScanPlan) is used to call _input->init().
	// ScanPlan's init() will return and construct a ScanIterator(this), where input.txt will be read.

	// run (defined in iterator.cpp) will call SortIterator::next() in a while loop

	return 0;
} // main
