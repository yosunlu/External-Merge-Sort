#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "gen.h"
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>

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

	// create records that will be saved in the input file
	genDataRecords(numOfRecord);
	Plan *const plan = new SortPlan(new ScanPlan(numOfRecord));
	// Plan * const plan = new FilterPlan ( new ScanPlan (7) );
	// new SortPlan ( new FilterPlan ( new ScanPlan (7) ) );

	Iterator *const it = plan->init();
	// SortPlan has private attribute _input, which is intialized to a ScanPlan.
	// In SortPlan's init(), a SortIterator(this) is constructed and returned.
	// In SortIterator's constructor, the object itself is passed in as argument.
	// The object itself's private attribute _input is used to initialized SortIterator's private attributes.
	// In the constructor, the object's _input (a ScanPlan) is used to call _input->init().
	// ScanPlan's init() will return and construct a ScanIterator(this), where input.txt will be read.

	// run (defined in iterator.cpp) will call SortIterator::next() in a while loop
	it->run();
	delete it;

	delete plan;

	return 0;
} // main
