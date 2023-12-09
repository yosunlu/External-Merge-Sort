#include "DataRecord.h"
#include "Dram.h"
#include "Filter.h"
#include "Iterator.h"
#include "Scan.h"
#include "Sort.h"
#include "SortState.h"
#include "TraceFile.h"
#include "gen.h"
#include "global.h"
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

// long numOfRecord = 0;
// long record_size = 0;
#define MB 1000000LL

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

// Function to delete all files in a directory
void deleteFilesInDirectory(const char *directoryName)
{
	DIR *dir = opendir(directoryName);
	if (dir != nullptr)
	{
		struct dirent *entry;
		while ((entry = readdir(dir)) != nullptr)
		{
			const std::string fileName = entry->d_name;

			// Skip deletion of . and ..
			if (fileName != "." && fileName != "..")
			{
				std::string filePath = std::string(directoryName) + "/" + fileName;
				if (remove(filePath.c_str()) != 0)
				{
					std::cerr << "Error deleting file: " << filePath << std::endl;
				}
			}
		}
		closedir(dir);
	}
	else
	{
		std::cerr << "Error opening directory." << std::endl;
	}
}

// Function to create a directory if it does not exist
void createDirectory(const char *directoryName, bool ifDeletedAllFiles)
{
	struct stat info;

	if (stat(directoryName, &info) != 0)
	{
		// Directory does not exist, try to create it
		int dirCreationResult = mkdir(directoryName, 0777);
		if (dirCreationResult == 0)
		{
			std::cout << directoryName << " created successfully." << std::endl;
		}
		else
		{
			std::cerr << "Error creating " << directoryName << "." << std::endl;
		}
	}
	else if (info.st_mode & S_IFDIR)
	{
		// Directory already exists
		std::cout << directoryName << " already exists." << std::endl;
		if (ifDeletedAllFiles)
		{
			// Delete all files in the directory
			deleteFilesInDirectory(directoryName);

			std::cout << "All files in SSD directory deleted." << std::endl;
		}
	}
	else
	{
		// Path exists but is not a directory
		std::cerr << directoryName << "exists but is not a directory." << std::endl;
	}
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

	TraceFile tracefile(output_file);
	// tracefile.trace(RUN_PHASE_1);
	// tracefile.trace(RUN_PHASE_2);
	// tracefile.trace(EXTERNAL_PHASE_1_1);
	// tracefile.trace(EXTERNAL_PHASE_1_2);
	// tracefile.trace(EXTERNAL_PHASE_2_1);
	// tracefile.trace(EXTERNAL_PHASE_2_2);

	// create input directory
	const char *directoryName = "input";
	// Check if the directory exists and create if necessary
	createDirectory(directoryName, false);

	// create SSD directory
	directoryName = "SSD-10GB";
	// Check if the directory exists and create if necessary
	createDirectory(directoryName, true);

	// create HDD directory
	directoryName = "HDD";
	// Check if the directory exists and create if necessary
	createDirectory(directoryName, true);

	// create output directory
	directoryName = "output";
	// Check if the directory exists and create if necessary
	createDirectory(directoryName, true);

	// 1MB number of record
	long long numOfRec1MB = MB / record_size;
	// 100MB number of record
	long long numOfRec100MB = 100 * MB / record_size;
	// 1GB number of record
	long long numOfRec1GB = 1000 * MB / record_size;
	// 10GB number of record
	long long numOfRec10GB = 10 * 1000 * MB / record_size;
	// how many 10GB file to be generated
	// int numOf10GBs = numOfRecord / 10000000;			   // 10GB is 10,000,000 records
	int numOf10GBs = numOfRecord / numOfRec10GB; // 10GB is 10,000,000 records
	// int numRecord_leftOverOf10GB = numOfRecord % 10000000; // if 115GB, leftover is 5GB, which is 5,000,000 records
	int numRecord_leftOverOf10GB = numOfRecord % numOfRec10GB; // if 115GB, leftover is 5GB, which is 5,000,000 records
	// int numRecord_leftOverOf100MB = numRecord_leftOverOf10GB % 100000;
	int numRecord_leftOverOf100MB = numRecord_leftOverOf10GB % numOfRec100MB;
	// int numRecord_leftOverOf1MB = numRecord_leftOverOf100MB % 1000;
	int numRecord_leftOverOf1MB = numRecord_leftOverOf100MB % numOfRec1MB;
	totalSize = numOfRecord * record_size;

	/**

	 125MB, 50 byte, what is numRecord_leftOverOf100MB?


	 *
	*/

	/**
	120GB: 120,000,000 records
	112.123123GB: 112,123,123 records

	10GB is 10,000,000 records
	leftover for 10GB is 112,123,123 % 10,000,000 = 2,123,123 records

	100MB is 100,000 records
	leftover for 100MB is 2,123,123 % 100,000 records = 23,123 records

	1MB is 1,000 records
	leftover for 1MB is 23,123 % 1,000 = 123 records
	*/

	std::vector<std::ifstream *> inputFiles; // stores the input pointers of file streams

	/***********************************************************/
	/*************** Data generation ***************************/
	/***********************************************************/
	// compute record's column size
	long totalColumnSize = record_size - 4;
	incl_size = totalColumnSize / 3;
	mem_size = totalColumnSize / 3;
	mgmt_size = totalColumnSize / 3 + totalColumnSize % 3;

	genDataRecords(numOfRecord); // generate and store a single unsorted file in input/input.txt

	// create a pointer to that 10GB * numOf10GBs unsorted file
	std::ifstream *inputFile = new std::ifstream("input/input.txt", std::ios::binary);
	if (!inputFile->is_open())
		std::cerr << "Error opening input file." << std::endl;

	inputFiles.push_back(inputFile);

	/***********************************************************/
	/*************** Data larger than 10GB *********************/
	/***********************************************************/

	// each iteration will create a 10GB sorted file to HDD
	// when the for loop ends, there will be numOf10GBs * 10GB sorted files in HDD
	bool ifGraceful = numOf10GBs == 1 && (numRecord_leftOverOf10GB / numOfRec1GB) == 2 ? true : false;
	for (int s = 0; s < numOf10GBs; ++s)
	{
		// each iteration will create a sorted 100MB file to SSD
		// when the for loop ends, there will be 100 * 100MB files in SSD
		for (int i = 0; i < 100; i++) // 100 * 100MB dataRecords
		{
			// each iteration will push_back one pointer to 1000 records (1MB) to dataRecords;
			// when the for loop ends, there will be one dataRecord which is 100MB
			for (int j = 0; j < 100; ++j)
			{
				if (s == 0 && i == 0 && j == 0)
					tracefile.trace(RUN_PHASE_1);
				Plan *const plan = new SortPlan(new ScanPlan(numOfRec1MB), RUN_PHASE_1, inputFiles, 0, 0, false, 0); // quick sort 1MB of data and repeat 100 times
				Iterator *const it = plan->init();
				it->run();

				delete it;
				delete plan;
			}
			if (s == 0 && i == 0)
				tracefile.trace(RUN_PHASE_2);
			// merge sort the 100MB data created in DRAM, and output to SSD
			Plan *plan;
			Iterator *it;
			if (ifGraceful)
			{
				plan = new SortPlan(new ScanPlan(numOfRec100MB), RUN_PHASE_2, inputFiles, i, 0, ifGraceful, (numRecord_leftOverOf10GB / numOfRec1GB)); // 100000 records is 100MB
				it = plan->init();
				it->run();
			}
			else
			{
				plan = new SortPlan(new ScanPlan(numOfRec100MB), RUN_PHASE_2, inputFiles, i, 0, false, 0); // 100000 records is 100MB
				it = plan->init();
				it->run();
			}

			for (std::vector<DataRecord *>::size_type i = 0; i < dataRecords.size(); ++i)
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
		if (!ifGraceful)
		{
			for (int i = 0; i < 100; i++)
			{
				std::stringstream filename;
				filename << "SSD-10GB/output_" << i << ".txt";
				std::ifstream *newInputFile = new std::ifstream(filename.str(), std::ios::binary);
				traceprintf("%i\n", s);

				if (!newInputFile->is_open())
					std::cerr << "Error opening input file." << std::endl;

				inputFiles.push_back(newInputFile);
			}

			// merge sort the 100 * 100MB files in SSD, and output to HDD
			// For 1KB: 100 * 100MB = 10,000,000 records
			// For 50 bytes: 100 * 100MB = 200,000,000 records
			Plan *const plan = new SortPlan(new ScanPlan(numOfRec10GB), EXTERNAL_PHASE_1, inputFiles, 0, s, false, 0);
			Iterator *const it = plan->init();
			it->run();

			if (s == 0)
			{
				tracefile.trace(EXTERNAL_PHASE_1_1);
				tracefile.trace(EXTERNAL_PHASE_1_2);
			}

			for (std::vector<DataRecord *>::size_type i = 0; i < dataRecords.size(); ++i)
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
				delete inputFiles[fileCount + 1];
				inputFiles.pop_back();
			}
		}
	}

	// last step: merge sort 10GB files from HDD
	// make the 10GB outputs from previous phase new inputs
	// only do the final merge here if input size is 10x GB (not, for example, 12.5 GB)
	if (numOf10GBs && !numRecord_leftOverOf10GB)
	{
		// close the initial large file
		closeInputFiles(inputFiles);
		// delete the large 120GB file
		delete inputFiles[0];
		// pop-back the 0th large input file (120GB)
		inputFiles.pop_back();
		inputFiles.clear();

		for (int i = 0; i < numOf10GBs; i++)
		{
			std::stringstream filename;
			filename << "HDD/output_10GB_" << i << ".txt";
			std::ifstream *newInputFile = new std::ifstream(filename.str(), std::ios::binary);

			if (!newInputFile->is_open())
				std::cerr << "Error opening input file." << std::endl;

			inputFiles.push_back(newInputFile);
		}

		Plan *const plan = new SortPlan(new ScanPlan(numOfRecord), EXTERNAL_PHASE_2, inputFiles, 0, numOf10GBs, false, 0);
		Iterator *const it = plan->init();
		it->run();
		tracefile.trace(EXTERNAL_PHASE_2_1);
		tracefile.trace(EXTERNAL_PHASE_2_2);

		// clear the 10GB sorted input files in SSD
		closeInputFiles(inputFiles);
		// for (int i = 0; i < numOf10GBs; ++i)
		// {
		// 	delete inputFiles[i];
		// 	inputFiles.pop_back();
		// }

		// after outputting the 120GB sorted file to output, clear the HDD
		for (int fileCount = 0; fileCount < numOf10GBs; ++fileCount)
		{
			std::stringstream filename;
			filename << "HDD/output_10GB_" << fileCount << ".txt";

			// Convert the stringstream to a string and then to a path
			std::string file_to_delete = filename.str();

			if (std::remove(file_to_delete.c_str()) != 0)
				perror("Error deleting file");
			delete inputFiles[fileCount];
			inputFiles.pop_back();
		}
	}

	/***** The operation above deals with data size 10GB and above *****/

	/***************************************************************************/
	/******************* 1GB ~ 9GB data leftover *******************************/
	/****** does not consider decimals, for example 1.5GB) *********************/
	/***************************************************************************/

	// 12GB: leftover of 10GB is 2,000,000 ; no leftover of 100MB
	// 125MB: leftover of 10GB is 125,000; leftover of 100MB is 25,000
	if (numRecord_leftOverOf10GB && !numRecord_leftOverOf100MB)
	{
		// for example 3GB; 3GB is 3,000,000 records; 1GB is 1,000,000 records
		// For 1KB record: 1GB is 1,000,000 records
		// For 50 byte record: 1GB is 20,000,000 records
		int numOfGBs = numRecord_leftOverOf10GB / numOfRec1GB; // 3
		int numOf100MBruns = numOfGBs * 10;
		// each iteration will create a sorted numOf100MBruns file to SSD
		// when the for loop ends, there will be numOf100MBruns * 100MB files in SSD
		for (int i = 0; i < numOf100MBruns; i++) // numOfGBs * 10 * 100MB dataRecords
		{
			// each iteration will push_back one pointer to 1000 records (1MB) to dataRecords;
			// when the for loop ends, there will be one dataRecord which is 100MB
			for (int j = 0; j < 100; ++j)
			{
				Plan *const plan = new SortPlan(new ScanPlan(numOfRec1MB), RUN_PHASE_1, inputFiles, 0, 0, false, 0); // quick sort 1MB of data and repeat 100 times
				Iterator *const it = plan->init();
				it->run();

				delete it;
				delete plan;
			}

			// merge sort the 100MB data created in DRAM, and output to SSD
			Plan *const plan = new SortPlan(new ScanPlan(numOfRec100MB), RUN_PHASE_2, inputFiles, i, 0, false, 0); // 100000 records is 100MB
			Iterator *const it = plan->init();
			it->run();

			for (std::vector<DataRecord *>::size_type i = 0; i < dataRecords.size(); ++i)
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

		// make the numOf100MBruns outputs from last for loop the new inputs
		bool ifGraceful = numOfGBs == 2 ? true : false;
		if (!ifGraceful)
		{
			for (int i = 0; i < numOf100MBruns; i++)
			{
				std::stringstream filename;
				filename << "SSD-10GB/output_" << i << ".txt";
				std::ifstream *newInputFile = new std::ifstream(filename.str(), std::ios::binary);

				if (!newInputFile->is_open())
					std::cerr << "Error opening input file." << std::endl;

				inputFiles.push_back(newInputFile);
			}

			// merge sort the numOf100MBruns * 100MB files in SSD, and output to HDD
			// Plan *const plan = new SortPlan(new ScanPlan(numOfGBs * 1000000), EXTERNAL_PHASE_1, inputFiles, 0, 0);
			Plan *const plan = new SortPlan(new ScanPlan(numOfGBs * numOfRec1GB), EXTERNAL_PHASE_1, inputFiles, 0, 0, false, 0);
			Iterator *const it = plan->init();
			it->run();

			for (int i = 0; i < numOf100MBruns; ++i)
			{
				delete[] dataRecords[i];
			}

			dataRecords.clear();

			delete it;
			delete plan;

			// after outputting the numOf100MBruns * 100MB sorted file to HDD, clear the SSD
			for (int fileCount = 0; fileCount < numOf100MBruns; ++fileCount)
			{
				std::stringstream filename;
				filename << "SSD-10GB/output_" << fileCount << ".txt";

				// Convert the stringstream to a string and then to a path
				std::string file_to_delete = filename.str();

				if (std::remove(file_to_delete.c_str()) != 0)
					perror("Error deleting file");
				delete inputFiles[fileCount + 1];
				inputFiles.pop_back();
			}
		}
		else
		{
			for (int i = 0; i < 100; i++)
			{
				std::stringstream filename;
				filename << "SSD-10GB/output_graceful_" << i << ".txt";
				std::ifstream *newInputFile = new std::ifstream(filename.str(), std::ios::binary);

				if (!newInputFile->is_open())
					std::cerr << "Error opening input file." << std::endl;

				inputFiles.push_back(newInputFile);
			}

			for (int i = 0; i < 20; i++)
			{
				std::stringstream filename;
				filename << "SSD-10GB/output_" << i << ".txt";
				std::ifstream *newInputFile = new std::ifstream(filename.str(), std::ios::binary);

				if (!newInputFile->is_open())
					std::cerr << "Error opening input file." << std::endl;

				inputFiles.push_back(newInputFile);
			}

			for (int i = 0; i < 100; i++)
			{
				std::stringstream filename;
				filename << "HDD/output_graceful_" << i << ".txt";
				std::ifstream *newInputFile = new std::ifstream(filename.str(), std::ios::binary);

				if (!newInputFile->is_open())
					std::cerr << "Error opening input file." << std::endl;

				inputFiles.push_back(newInputFile);
			}

			// merge sort the numOf100MBruns * 100MB files in SSD, and output to HDD
			// Plan *const plan = new SortPlan(new ScanPlan(numOfGBs * 1000000), EXTERNAL_PHASE_1, inputFiles, 0, 0);
			Plan *const plan = new SortPlan(new ScanPlan((numOfGBs + 10) * numOfRec1GB), EXTERNAL_PHASE_1, inputFiles, 0, 0, ifGraceful, numOfGBs);
			Iterator *const it = plan->init();
			it->run();
			std::cout << "debug 12GB - 2" << std::endl;
			for (int i = 0; i < numOf100MBruns; ++i)
			{
				delete[] dataRecords[i];
			}

			dataRecords.clear();
			std::cout << "debug 12GB - 3" << std::endl;
			delete it;
			delete plan;

			// after outputting the numOf100MBruns * 100MB sorted file to HDD, clear the SSD
			for(size_t i = 0; i < inputFiles.size(); i++) {
				delete inputFiles[i];
			}

			inputFiles.clear();
			std::cout << "debug 12GB - 4" << std::endl;
			// Delete all files in the directory
			const char *dir = "HDD";
			deleteFilesInDirectory(dir);
			dir = "SSD-10GB";
			deleteFilesInDirectory(dir);
			std::cout << "debug 12GB - 5" << std::endl;
			
			tracefile.trace(END);
			return 0;
		}
		
	}

	// Last step: external merge sort that deals with data size 12.5GB
	if (numRecord_leftOverOf10GB && !numRecord_leftOverOf100MB)
	{
		// remove the large input file (12.5GB)
		closeInputFiles(inputFiles);
		delete inputFiles[0];
		// pop-back the 0th large input file (12.5GB)
		inputFiles.pop_back();
		inputFiles.clear();

		for (int i = 0; i < numOf10GBs + 1; i++)
		{
			std::stringstream filename;
			filename << "HDD/output_10GB_" << i << ".txt";
			std::ifstream *newInputFile = new std::ifstream(filename.str(), std::ios::binary);

			if (!newInputFile->is_open())
				std::cerr << "Error opening input file." << std::endl;

			inputFiles.push_back(newInputFile);
		}

		Plan *const plan = new SortPlan(new ScanPlan(numOfRecord), EXTERNAL_PHASE_2, inputFiles, 0, numOf10GBs, false, 0);
		Iterator *const it = plan->init();
		it->run();

		tracefile.trace(EXTERNAL_PHASE_2_1);
		tracefile.trace(EXTERNAL_PHASE_2_2);

		// delete the 10GB files in SSD
		closeInputFiles(inputFiles);
		// for (int i = 0; i < numOf10GBs + 1; ++i)
		// {
		// 	delete inputFiles[i];
		// 	inputFiles.pop_back();
		// }

		// after outputting the 12GB sorted file to output, clear the HDD
		for (int fileCount = 0; fileCount < numOf10GBs + 1; ++fileCount)
		{
			std::stringstream filename;
			filename << "HDD/output_10GB_" << fileCount << ".txt";

			// Convert the stringstream to a string and then to a path
			std::string file_to_delete = filename.str();

			if (std::remove(file_to_delete.c_str()) != 0)
				perror("Error deleting file");
			delete inputFiles[fileCount];
			inputFiles.pop_back();
		}
	}

	/***************************************************************************/
	/******************* 100mb ~ 1GB data leftover *****************************/
	/****** does not consider decimals, for example 1.5GB) *********************/
	/***************************************************************************/

	// if (numRecord_leftOverOf100MB && numOfRecord > 100000)
	if (numRecord_leftOverOf100MB && numOfRecord > numOfRec100MB)
	{

		// for example: 125MB, 125,000 records, only has 1 * 100000
		// int numOf100MB = numOfRecord / 100000;
		int numOf100MB = numOfRecord * record_size / (100 * MB);

		// leftover of the 100MBs
		// if the leftover is 25MB, will sort this 25MB is DRAM and output to SSD
		// int MBleft = numRecord_leftOverOf100MB / 1000;
		int MBleft = numRecord_leftOverOf100MB / numOfRec1MB;

		// each iteration will create a 100MB sorted file to SSD
		bool ifGraceful = MBleft / 100 <= 0.25 ? true : false;
		std::cout << "numOfRec100MB:" << numOfRec100MB << std::endl;
		std::cout << "ifGraceful:" << ifGraceful << std::endl;
		std::cout << "numOf100MB:" << numOf100MB << std::endl;
		int i = 0;
		for (; i < numOf100MB; i++)
		{
			// each iteration will push_back one pointer to 1000 records (1MB) to dataRecords;
			// when the for loop ends, there will be one dataRecord (DRAM) which is 100MB
			for (int j = 0; j < 100; ++j)
			{
				Plan *const plan = new SortPlan(new ScanPlan(numOfRec1MB), RUN_PHASE_1, inputFiles, 0, 0, false, 0); // quick sort 1MB of data and repeat 100 times
				Iterator *const it = plan->init();
				it->run();
				// if is graceful degradation, output the last 25% of each 1MB run to SSD
				if (i == numOf100MB - 1 && ifGraceful)
				{
					// if numOfRec1MB of 50 bytes record is 20,000
					// this will output inner[15,000]~inner[19,999] to SSD
					std::stringstream filename;
					filename << "SSD-10GB/output_graceful_" << j << ".txt";
					std::ofstream outputFile(filename.str(), std::ios::binary | std::ios::app);
					if (!outputFile.is_open())
						std::cerr << "Error opening output file." << std::endl;
					int startOutIdx = numOfRec1MB * 0.75;
					for (int k = startOutIdx; k < numOfRec1MB; k++)
					{
						DataRecord *inner = dataRecords.at(j);
						DataRecord output_record = inner[k];
						outputFile.write(output_record.getIncl(), incl_size);
						outputFile.write(" ", 1);
						outputFile.write(output_record.getMem(), mem_size);
						outputFile.write(" ", 1);
						outputFile.write(output_record.getMgmt(), mgmt_size);
						outputFile.write("\r\n", 2);

						// free the output record
						// delete (&inner)[k];
						// std::cout << "-----debug graceful 2-----"  << std::endl;
						// std::unique_ptr<DataRecord> myUniquePtr = nullptr;
						// (&inner)[k] = nullptr;
						// std::cout << "-----debug graceful 3-----"  << std::endl;
					}

					outputFile.close();
				}

				delete it;
				delete plan;

				if (i == 0 && j == 0)
					tracefile.trace(RUN_PHASE_1);
			}

			// if is graceful degradation, read the last 25MB from input
			if (i == numOf100MB - 1 && ifGraceful)
			{
				for (int j = 0; j < 25; ++j)
				{
					Plan *const plan = new SortPlan(new ScanPlan(numOfRec1MB), RUN_PHASE_1, inputFiles, 0, 0, false, 0); // quick sort 1MB of data and repeat 100 times
					Iterator *const it = plan->init();
					it->run();
				}
			}

			Plan *plan;
			Iterator *it;
			if (i == numOf100MB - 1 && ifGraceful)
			{
				// merge sort the 100MB data created in DRAM, and output to SSD
				// 100 * 0.75MB data + 25 * 1MB
				for (int j = 0; j < 100; j++)
				{
					std::stringstream filename;
					filename << "SSD-10GB/output_graceful_" << j << ".txt";
					std::ifstream *newInputFile = new std::ifstream(filename.str(), std::ios::binary);

					if (!newInputFile->is_open())
						std::cerr << "Error opening input file." << std::endl;

					inputFiles.push_back(newInputFile);
				}

				plan = new SortPlan(new ScanPlan(numOfRec100MB + ((MBleft * MB) / record_size)), RUN_PHASE_2, inputFiles, i, 0, ifGraceful, MBleft); // 100000 records is 100MB
				it = plan->init();
				it->run();

				// after outputting the 100 * 100MB (10GB) sorted file to HDD, clear the SSD
				for (int fileCount = 0; fileCount < 100; ++fileCount)
				{
					std::stringstream filename;
					filename << "SSD-10GB/output_graceful_" << fileCount << ".txt";

					// Convert the stringstream to a string and then to a path
					std::string file_to_delete = filename.str();

					if (std::remove(file_to_delete.c_str()) != 0)
						perror("Error deleting file");
					delete inputFiles[fileCount + 1];
					inputFiles.pop_back();
				}
			}
			else
			{
				// merge sort the 100MB data created in DRAM, and output to SSD
				plan = new SortPlan(new ScanPlan(numOfRec100MB), RUN_PHASE_2, inputFiles, i, 0, ifGraceful, 0); // 100000 records is 100MB
				it = plan->init();
				it->run();
			}

			for (std::vector<DataRecord *>::size_type i = 0; i < dataRecords.size(); ++i)
			{
				delete[] dataRecords[i];
			}
			if (i == 0)
				tracefile.trace(RUN_PHASE_2);

			dataRecords.clear();

			delete it;
			delete plan;
		}

		if (!ifGraceful)
		{
			for (int j = 0; j < MBleft; ++j)
			{
				Plan *const plan = new SortPlan(new ScanPlan(numOfRec1MB), RUN_PHASE_1, inputFiles, 0, 0, false, 0); // quick sort 1MB of data and repeat numRecord_leftOverOf100MB times
				Iterator *const it = plan->init();
				it->run();

				delete it;
				delete plan;
			}
			// merge sort the numRecord_leftOverOf100MB data created in DRAM, and output to SSD
			// 25MB is 25000 records
			// Plan *const plan = new SortPlan(new ScanPlan(numRecord_leftOverOf100MB * 1000), RUN_PHASE_2, inputFiles, i, 0); // 100000 records is 100MB
			Plan *const plan = new SortPlan(new ScanPlan(MBleft * numOfRec1MB), RUN_PHASE_2, inputFiles, i, 0, false, 0); // 100000 records is 100MB
			Iterator *const it = plan->init();
			it->run();

			for (int i = 0; i < MBleft; ++i)
			{
				delete[] dataRecords[i];
			}

			dataRecords.clear();
			delete it;
			delete plan;
			// merge files from SSD to HDD
			// make the numOf100MBruns outputs from last for loop the new inputs
			for (int j = 0; j < numOf100MB + 1; j++)
			{
				std::stringstream filename;
				filename << "SSD-10GB/output_" << j << ".txt";
				std::ifstream *newInputFile = new std::ifstream(filename.str(), std::ios::binary);

				if (!newInputFile->is_open())
					std::cerr << "Error opening input file." << std::endl;

				inputFiles.push_back(newInputFile);
			}

			Plan *const plan_1 = new SortPlan(new ScanPlan(numOfRecord), EXTERNAL_PHASE_1, inputFiles, 0, 0, false, 0);
			Iterator *const it_1 = plan_1->init();
			it_1->run();

			for (int i = 0; i < numOf100MB + 1; ++i)
			{
				delete[] dataRecords[i];
			}

			dataRecords.clear();

			delete it_1;
			delete plan_1;
			// after outputting the 100 * 100MB (10GB) sorted file to HDD, clear the SSD
			for (int fileCount = 0; fileCount < numOf100MB + 1; ++fileCount)
			{
				std::stringstream filename;
				filename << "SSD-10GB/output_" << fileCount << ".txt";

				// Convert the stringstream to a string and then to a path
				std::string file_to_delete = filename.str();

				if (std::remove(file_to_delete.c_str()) != 0)
					perror("Error deleting file");
				delete inputFiles[fileCount + 1];
				inputFiles.pop_back();
			}
		}

		// remove the large input file (12.5GB)
		closeInputFiles(inputFiles);
		delete inputFiles[0];
		// pop-back the 0th large input file (12.5GB)
		inputFiles.pop_back();
		inputFiles.clear();
	}

	/***************************************************************************/
	/******************* less than 100MB data leftover *************************/
	/****************** can handle decimals, like 55MB ***********************/
	/***************************************************************************/

	if (totalSize < 100000000)
	{
		// for example : 55MB = 55000 records

		// numRecord_leftOverOf100MB = 55,500
		// int numOf1MBruns = numRecord_leftOverOf100MB / 1000; // 55
		int numOf1MBruns = numRecord_leftOverOf100MB / numOfRec1MB; // 55
		traceprintf("================%i==========\n", numOf1MBruns);

		// // quick sort the 50.5MB and store in dram (daraRecords)
		// // when completed, there will be 50 sorted 1MB runs in dram
		for (int i = 0; i < numOf1MBruns; i++)
		{
			Plan *const plan_phase1 = new SortPlan(new ScanPlan(numOfRec1MB), RUN_PHASE_1, inputFiles, 0, 0, false, 0);
			Iterator *const it_1 = plan_phase1->init();
			it_1->run();

			if (i == 0)
				tracefile.trace(RUN_PHASE_1);
			delete it_1;
			delete plan_phase1;
		}

		// // there are less than 1MB of data left, for example 55,500 records: 500 records left, which is 500KB
		if (numRecord_leftOverOf1MB)
		{
			Plan *const plan_phase1 = new SortPlan(new ScanPlan(numRecord_leftOverOf1MB), RUN_PHASE_1, inputFiles, 0, 0, false, 0);
			Iterator *const it_1 = plan_phase1->init();
			it_1->run();

			delete it_1;
			delete plan_phase1;
		}

		Plan *const plan_phase2 = new SortPlan(new ScanPlan(numRecord_leftOverOf100MB), RUN_PHASE_2, inputFiles, 0, 0, false, 0);
		Iterator *const it_2 = plan_phase2->init();
		it_2->run();
		tracefile.trace(RUN_PHASE_2);

		delete it_2;
		delete plan_phase2;

		closeInputFiles(inputFiles);
		inputFiles.pop_back();
	}

	tracefile.trace(END);
	return 0;
} // main

// SortPlan has private attribute _input, which is intialized to a ScanPlan.
// In SortPlan 's init(), a SortIterator(this) is constructed and returned.
// In SortIterator' s constructor, the object itself is passed in as argument.
// The object itself 's private attribute _input is used to initialized SortIterator' s private attributes.
// In the constructor, the object 's _input (a ScanPlan) is used to call _input->init().
// ScanPlan' s init() will return and construct a ScanIterator(this), where input.txt will be read.

// //****** debugging *********//

// // std::ifstream *inputFile = new std::ifstream("SSD-10GB/input.txt", std::ios::binary);
// // if (!inputFile->is_open())
// // 	std::cerr << "Error opening input file." << std::endl;

// // inputFiles.push_back(inputFile);

// // for (int i = 0; i < 100; i++)
// // {
// // 	std::stringstream filename;
// // 	filename << "SSD-10GB/output_" << i << ".txt";
// // 	std::ifstream *newInputFile = new std::ifstream(filename.str(), std::ios::binary);

// // 	if (!newInputFile->is_open())
// // 		std::cerr << "Error opening input file." << std::endl;

// // 	inputFiles.push_back(newInputFile);
// // }

// // for (int fileCount = 0; fileCount < 100; ++fileCount)
// // {
// // 	std::stringstream filename;
// // 	filename << "SSD-10GB/output_" << fileCount << ".txt";

// // 	// Convert the stringstream to a string and then to a path
// // 	std::string file_to_delete = filename.str();

// // 	if (std::remove(file_to_delete.c_str()) != 0)
// // 		perror("Error deleting file");
// // 	delete inputFiles[fileCount + 1];
// // 	inputFiles.pop_back();
// // }

// //****** debugging ends *********//

// HDD_page_size = 100 * MB;
// numOfPage = totalSize / HDD_page_size;
// singlePageTransferTime = HDD_page_size / HDD_bandwidth;
// double total_latency_100MB_HDD = numOfPage * (HDD_latency + singlePageTransferTime);

// file << "latency: " << numOfPage * HDD_latency << "ms;  ";
// file << "transfer time: " << static_cast<double>(numOfPage) * singlePageTransferTime << "ms;  ";
// file << "total " << total_latency_100MB_HDD << "ms\n";