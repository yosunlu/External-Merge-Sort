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
#include <sstream>
#include <cstdio>

long numOfRecord = 0;
long record_size = 0;

// Function to close all files in the vector
void closeInputFiles(std::vector<std::ifstream *> &inputFiles)
{
	for (size_t i = 0; i < inputFiles.size(); ++i)
	{
		if (inputFiles[i]->is_open())
		{
			inputFiles[i]->close();
		}
	}

	// Clear the vector after closing files
	inputFiles.clear();
}

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
			std::cout << "Input directory created successfully." << std::endl;
		}
		else
		{
			std::cerr << "Error creating directory." << std::endl;
		}
	}
	else if (info.st_mode & S_IFDIR)
	{
		// Directory exists
		std::cout << "Input Directory already exists." << std::endl;
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

	// create HDD directory
	directoryName = "HDD";

	// Check if the directory exists
	if (stat(directoryName, &info) != 0)
	{
		// Directory does not exist, try to create it
		int dirCreationResult = mkdir(directoryName, 0777);
		if (dirCreationResult == 0)
		{
			std::cout << "HDD created successfully." << std::endl;
		}
		else
		{
			std::cerr << "Error creating HDD." << std::endl;
		}
	}
	else if (info.st_mode & S_IFDIR)
	{
		// Directory exists
		std::cout << "HDD already exists." << std::endl;
	}
	else
	{
		// Path exists but is not a directory
		std::cerr << "Path exists but is not a directory." << std::endl;
	}

	// create records that will be saved in the input file

	// assuming 10GB data; 1KB (per record) * 1000 (quicksort these  1000 record, 1MB) * 100 (total 100MB) * 100 = 10G
	// int numBatch = numOfRecord / 1000;

	genDataRecords(numOfRecord); // 10GB data = 1000 * 100 * 100 records = 10,000,000 records
								 // genDataRecords will store a single 10GB unsorted file in input/input.txt

	std::vector<std::ifstream *> inputFiles;

	// create a pointer to that 10GB * numOf10GBs unsorted file
	std::ifstream inputFile("input/input.txt", std::ios::binary);
	if (!inputFile.is_open())
		std::cerr << "Error opening input file." << std::endl;

	inputFiles.push_back(&inputFile);

	int numOf10GBs = numOfRecord / 10000000; // how many 10GB file to be generated

	// each iteration will create a 10GB sorted file to HDD
	for (int s = 0; s < numOf10GBs; ++s)
	{
		// each iteration will create a sorted 100MB file to SSD
		for (int i = 0; i < 100; i++) // 100 * 100MB dataRecords
		{
			// each iteration will push_back one pointer to 1000 records to dataRecords;
			// when the for loop ends, there will be one dataRecord which is 100MB
			for (int j = 0; j < 100; ++j)
			{
				Plan *const plan = new SortPlan(new ScanPlan(1000), RUN_PHASE_1, inputFiles, 0, 0); // quick sort 1MB of data and repeat 100 times
				Iterator *const it = plan->init();
				it->run();

				delete it;
				delete plan;
			}

			Plan *const plan = new SortPlan(new ScanPlan(100000), RUN_PHASE_2, inputFiles, i, 0); // 100000 record is 100MB
			Iterator *const it = plan->init();
			it->run();

			for (int i = 0; i < 100; ++i)
			{
				delete[] dataRecords[i];
			}

			dataRecords.clear();

			delete it;
			delete plan;
		}

		// external sort phase 1
		// merge 100 * 100MB on SSD to a 10GB on HDD
		// inputFiles[1] ~ inputFiles[100]

		// make the 100 outputs from last for loop the new inputs
		for (int i = 0; i < 100; i++)
		{
			std::stringstream filename;
			filename << "SSD-10GB/output_" << i << ".txt";
			std::ifstream *newInputFile = new std::ifstream(filename.str(), std::ios::binary);

			if (!newInputFile->is_open())
				std::cerr << "Error opening input file." << std::endl;

			inputFiles.push_back(newInputFile);
		}

		Plan *const plan = new SortPlan(new ScanPlan(10000000), EXTERNAL_PHASE_1, inputFiles, 0, s);
		Iterator *const it = plan->init();
		it->run();

		for (int i = 0; i < 100; ++i)
		{
			delete[] dataRecords[i];
		}

		dataRecords.clear();

		delete it;
		delete plan;

		// after outputting the 100 * 100MB (10GB) sorted file to HDD, clear the SSD
		for (int fileCount = 0; fileCount < 100; ++fileCount)
		{
			std::stringstream filename;
			filename << "SSD-10GB/output_" << fileCount << ".txt";

			// Convert the stringstream to a string and then to a path
			std::string file_to_delete = filename.str();

			if (std::remove(file_to_delete.c_str()) != 0)
				perror("Error deleting file");

			inputFiles.pop_back(); // clear the vector that stored pointers to 100 100MB files that was just sorted and outputted to HDD
		}
	}

	// external sort phase 2
	// clear the inputFiles vector and free the memory
	// for (std::ifstream *ptr : inputFiles)
	// 	delete ptr; // Delete the object pointed to by the pointer
	// inputFiles.clear();

	// // make the 10GB outputs from previous phase new inputs
	// for (int i = 0; i < numOf10GBs; i++)
	// {
	// 	std::stringstream filename;
	// 	filename << "HDD/output_10GB_" << i << ".txt";
	// 	std::ifstream *newInputFile = new std::ifstream(filename.str(), std::ios::binary);

	// 	if (!newInputFile->is_open())
	// 		std::cerr << "Error opening input file." << std::endl;

	// 	inputFiles.push_back(newInputFile);
	// }

	// Plan *const plan = new SortPlan(new ScanPlan(numOfRecord), EXTERNAL_PHASE_2, inputFiles, 0, numOf10GBs);
	// Iterator *const it = plan->init();
	// it->run();

	closeInputFiles(inputFiles);

	// Plan *const plan = new FilterPlan(new ScanPlan(7));
	// new SortPlan(new FilterPlan(new ScanPlan(7)));

	// SortPlan has private attribute _input, which is intialized to a ScanPlan.
	// In SortPlan 's init(), a SortIterator(this) is constructed and returned.
	// In SortIterator' s constructor, the object itself is passed in as argument.
	// The object itself 's private attribute _input is used to initialized SortIterator' s private attributes.
	// In the constructor, the object 's _input (a ScanPlan) is used to call _input->init().
	// ScanPlan' s init() will return and construct a ScanIterator(this), where input.txt will be read.
	return 0;
} // main
