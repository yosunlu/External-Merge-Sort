#include <fstream>
#include "Sort.h"
#include "Scan.h"
#include <fstream>
#include "Dram.h"
#include "Leaf.h"
#include "PQ.h"
#include <sstream>
#include "global.h"
#include <memory>

// #define REC_SIZE 1000

SortPlan::SortPlan(Plan *const input, SortState state, std::vector<std::ifstream *> inputFiles, int fileCount, int HDD_10GB_count) : _input(input), _state(state), _inputFiles(inputFiles), _fileCount(fileCount), _HDD_10GB_count(HDD_10GB_count)
{
	TRACE(true);
} // SortPlan::SortPlan

SortPlan::~SortPlan()
{
	TRACE(true);
	delete _input;
} // SortPlan::~SortPlan

Iterator *SortPlan::init() const
{
	TRACE(true);
	return new SortIterator(this);
} // SortPlan::init

void swap(DataRecord &a, DataRecord &b)
{
	a.swapContents(b);
}

int part(DataRecord records[], int lower, int upper)
{
	int left = lower;
	for (int i = lower; i < upper; i++)
	{
		if (std::strcmp(records[i].getIncl(), records[upper].getIncl()) < 0)
		{
			swap(records[i], records[left++]);
		}
	}
	swap(records[left], records[upper]);
	return left;
}

void qs(DataRecord records[], int lower, int upper)
{
	if (lower < upper)
	{
		int pivot = part(records, lower, upper);
		qs(records, lower, pivot - 1);
		qs(records, pivot + 1, upper);
	}
}

bool isPowerOfTwo(int x)
{
	return (x > 0) && ((x & (x - 1)) == 0);
}

SortIterator::SortIterator(SortPlan const *const plan) : _plan(plan), _input(plan->_input->init()), // _input is a ScanPlan, so init() will return ScanIterator
														 _consumed(0), _produced(0), _inputFiles(plan->_inputFiles), _fileCount(plan->_fileCount), _HDD_10GB_count(plan->_HDD_10GB_count)
{
	TRACE(true);
	// int i = 0;
	while (_input->next())
	{
		++_consumed;
		// DataRecord *record = _input->getCurrentRecord();
		// records[i++] = *record;
		// traceprintf("incl: %d ,mem: %d, mgmt: %d\n",record->getIncl(),record->getMem(),record->getMgmt());
	}
	// traceprintf("debug0-0\n");

	//  run generation phase1
	if (_plan->_state == RUN_PHASE_1)
	{
		DataRecord *records = new DataRecord[_consumed](); // a pointer to 100MB file
		int i = 0;
		int j = _consumed;
		while (j--)
		{
			char *row = new char[record_size];
			_inputFiles[0]->read(row, record_size);
			row[record_size - 2] = '\0'; // last 2 bytes are newline characters
			// Extracting data from the row
			char *incl = new char[incl_size + 1];
			char *mem = new char[mem_size + 1];
			char *mgmt = new char[mgmt_size + 1];

			std::strncpy(incl, row, incl_size);
			incl[incl_size] = '\0';

			std::strncpy(mem, row + incl_size + 1, mem_size);
			mem[mem_size] = '\0';

			std::strncpy(mgmt, row + incl_size + 1 + mem_size + 1, mgmt_size);
			mgmt[mgmt_size] = '\0';

			// Creating a DataRecord
			// DataRecord* record = new DataRecord(incl_size + 1, mem_size + 1, mgmt_size + 1, incl, mem, mgmt);
			std::unique_ptr<DataRecord> record(new DataRecord(incl_size + 1, mem_size + 1, mgmt_size + 1, incl, mem, mgmt));
			records[i++] = *record;
			// traceprintf("-----run1, record[i-1].getIncl:%s------\n", records[i-1].getIncl());
			delete[] incl;
			delete[] mem;
			delete[] mgmt;
			delete[] row;
		}

		qs(records, 0, _consumed - 1);
		dataRecords.push_back(records);
	}
	else if (_plan->_state == RUN_PHASE_2)
	{

		int sizeOfBucket = 1000; // 1000 records per bucket
		int numRecord_leftOverOf1MB = _consumed % 1000;
		int const buckets = dataRecords.size();
		int copyNum = buckets; // copyNum = buckets = 100
		int targetlevel = 0;

		// table to store the number of records for each buckets
		// useful when there are leftovers (32 * 1000 records, and 1 * 500 records)
		int *bucketSizeTable = new int[buckets]();
		for (int i = 0; i < buckets - 1; i++)
			bucketSizeTable[i] = 1000;
		bucketSizeTable[buckets - 1] = numRecord_leftOverOf1MB ? numRecord_leftOverOf1MB : 1000;

		while (copyNum >>= 1)
			++targetlevel;

		if (!isPowerOfTwo(buckets))
			targetlevel++;

		int *hashtable = new int[buckets](); // initializes to 0; stores pointer to the next record to be pushed for the leaf
		// buckets from Dram
		for (int i = 0; i < buckets; ++i)
		{
			DataRecord *inner = dataRecords.at(i);
			DataRecord record = inner[0];
			std::string inclString(record.getIncl());
			::leaf[i].assign(std::begin(inclString), std::end(inclString)); // assign only key to leaf
		}
		// capacity 2^targetlevel
		PQ priorityQueue(targetlevel);
		// calculate the ovc
		for (int i = 0; i < buckets; ++i)
		{
			int intValue = ::leaf[i][0] - '0';
			// 907 vs early-fence: arity = 3 (key has 3 columns); offset = 0 (compare with early-fence so differentiating index must be 0)
			// intValue = 9; arity - offset = 3 - 0 = 3; 3 * 100 + 9 = 309
			priorityQueue.push(i, (sizeOfColumn - 0) * 100 + intValue);

			// std::cout << "intValue[" << i << "]: " << intValue << std::endl;
		}

		// input is 100,000 records; if each leaf (bucket) contains 1000 records, we only need 100 buckets;
		// for the leftover buckets, fill in late fence and push
		if (buckets < priorityQueue.capacity())
		{
			for (int i = buckets; i < priorityQueue.capacity(); i++)
				priorityQueue.push(i, priorityQueue.late_fence());
		}

		std::stringstream filename;
		filename << "SSD-10GB/output_" << _fileCount << ".txt";

		// if (_consumed == 100000)
		// { // more than 10GB size records
		// 	filename << "SSD-10GB/output_" << _fileCount << ".txt";
		// }
		// else if (_consumed < 100000)
		// { // less than 100MB size records
		// 	filename << "HDD/final_output"
		// 			 << ".txt";
		// }

		std::ofstream outputFile(filename.str(), std::ios::binary | std::ios::app); // std::ios::app for appending
		traceprintf("%s\n", filename.str().c_str());
		if (!outputFile.is_open())
			std::cerr << "Error opening output file." << std::endl;

		int count = 0; // current count of the records being popped
		while (static_cast<RowCount>(count) < _consumed)
		{
			int idx = priorityQueue.pop();
			if (idx == -1)
				break;

			// write to output file
			// inner is a pointer to 1000 records, each 1kb
			DataRecord *inner = dataRecords.at(idx);
			// idx tells which leaf; hashtable[idx] returns the next pointer to the record
			DataRecord output_record = inner[hashtable[idx]];
			outputFile.write(output_record.getIncl(), incl_size);
			outputFile.write(" ", 1);
			outputFile.write(output_record.getMem(), mem_size);
			outputFile.write(" ", 1);
			outputFile.write(output_record.getMgmt(), mgmt_size);
			outputFile.write("\r\n", 2);

			if (static_cast<RowCount>(count) == _consumed - 1)
				break;
			if (hashtable[idx] < bucketSizeTable[idx] - 1) // check if there are record left in the bucket
			{
				char *preVal = new char[sizeOfColumn + 1];
				// assign the data outputted, so preVal can be used to compare with next value in the same bucket
				std::strcpy(preVal, ::leaf[idx].data());
				hashtable[idx]++; // update the pointer

				// up till now, the record of the leaf is still the same as the outputted one
				DataRecord *inner_cur = dataRecords.at(idx);   // inner_cur is a pointer to 1000 records, each 1kb
				DataRecord record = inner_cur[hashtable[idx]]; // hashtable[idx] has been updated

				std::strcpy(::leaf[idx].data(), record.getIncl()); // assign new key to leaf
				char *curVal = new char[sizeOfColumn + 1];

				std::strcpy(curVal, ::leaf[idx].data());

				// find the offset
				int offset = -1; // the index of the value that two values start to differ
				for (int i = 0; i < sizeOfColumn; i++)
				{
					if (preVal[i] == curVal[i])
					{
						continue;
					}
					else
					{
						offset = i;
						break;
					}
				}

				// two values of the same bucket are equal
				if (offset == -1)
				{
					offset = sizeOfColumn;
				}

				// if equal, the new value should be pushed right away to the output
				if (offset == sizeOfColumn)
				{
					priorityQueue.push(idx, 0);
				}

				// two values differ, the latter in the bucket will compare with the one popped
				else
				{
					int arityOffset = sizeOfColumn - offset;
					int intValue = curVal[offset] - '0';
					// std::cout << "push val:" << curVal << std::endl;
					// std::cout << "push idx:" << idx << ", key:" << arityOffset * 100 + intValue << std::endl;
					priorityQueue.push(idx, arityOffset * 100 + intValue);
				}

				delete[] preVal;
				delete[] curVal;
			}

			else // no more record left
			{
				priorityQueue.push(idx, priorityQueue.late_fence());
			}
			count++;
		}

		outputFile.close();
	}

	/***************************************/
	/*********External Phase 1**************/
	/***************************************/

	// 120GB: merges 100*100MB (10GB) runs from SSD to HDD (and repeate 12 times)
	// 125MB: merges 100MB and 25MB to output
	else if (_plan->_state == EXTERNAL_PHASE_1)
	{
		// mode 0: 2GB or 10xGB, mode 1: 125MB
		bool mode = _consumed < 1000000; // 1GB

		int numOf100MBruns = _consumed / 100000;
		// if handling 100MB's leftover, for example 25MB in 125MB
		// number of bucket should ++
		if (mode)
			numOf100MBruns++;

		// if handling 10GB's leftover, for example 2GB in 12GB
		// 12GB is 120,000,000; devided it by 10GB size of record
		// this 2GB will become another HDD output
		// this variable will be used later to name file
		int singleDigitGBleft = _consumed % 10000000 == 0 ? 0 : 1;

		// read the 1MB from each 100MB on SSD
		for (int i = 1; i < numOf100MBruns + 1; i++)
		{
			DataRecord *records = new DataRecord[1000]();

			for (int j = 0; j < 1000; j++)
			{
				char *row = new char[record_size];
				_inputFiles[i]->read(row, record_size);
				row[record_size - 2] = '\0'; // last 2 bytes are newline characters
				// Extracting data from the row
				char *incl = new char[incl_size + 1];
				char *mem = new char[mem_size + 1];
				char *mgmt = new char[mgmt_size + 1];

				std::strncpy(incl, row, incl_size);
				incl[incl_size] = '\0';

				std::strncpy(mem, row + incl_size + 1, mem_size);
				mem[mem_size] = '\0';

				std::strncpy(mgmt, row + incl_size + 1 + mem_size + 1, mgmt_size);
				mgmt[mgmt_size] = '\0';

				// Creating a DataRecord
				// DataRecord* record = new DataRecord(incl_size + 1, mem_size + 1, mgmt_size + 1, incl, mem, mgmt);
				std::unique_ptr<DataRecord> record(new DataRecord(incl_size + 1, mem_size + 1, mgmt_size + 1, incl, mem, mgmt));
				records[j] = *record;
				// traceprintf("records[j] incl: %s ,mem: %s, mgmt: %s\n", records[j].getIncl(),records[j].getMem(),records[j].getMgmt());
				delete[] incl;
				delete[] mem;
				delete[] mgmt;
				delete[] row;
			}

			dataRecords.push_back(records);
			// traceprintf("dataRecords incl: %s \n", dataRecords[dataRecords.size() - 1][0].getIncl());
		}

		// Dram (dataRecords) is 100MB
		// 1 MB = 8KB * 125
		// 8KB = 8 records
		// 8 * 125 records per bucket
		int sizeOfBucket = 1000;
		int const numOfbuckets = numOf100MBruns; // 100MB/1MB = 100 = fan-in
		int copyNum = numOfbuckets;				 // copyNum = buckets = 100
		int targetlevel = 0;

		int *bucketTotalSizeTable = new int[numOfbuckets]();
		for (int i = 0; i < numOfbuckets - 1; i++)
		{
			bucketTotalSizeTable[i] = 100000; // for normal 100MB sorted file , number of record is 100000
		}

		// mode 0: 2GB or 10xGB, mode 1: 125MB
		bucketTotalSizeTable[numOfbuckets - 1] = mode ? _consumed % 100000 : 100000;
		traceprintf("=====%i=====\n", bucketTotalSizeTable[1]);

		while (copyNum >>= 1)
			++targetlevel;

		if (!isPowerOfTwo(numOfbuckets))
		{
			targetlevel++;
		}

		// initialized to 0; stores pointer to the next record to be pushed for the leaf
		int *hashtable = new int[numOfbuckets]();

		// for how many records already output from each bucket
		int *cntPerBucket = new int[numOfbuckets]();

		// int *cnt1MBPerBucket = new int[buckets]();

		// already push 1 record from each bucket, thus init this array with 1
		for (int i = 0; i < numOfbuckets; ++i)
		{
			cntPerBucket[i] = 1;
		}

		// buckets from Dram
		for (int i = 0; i < numOfbuckets; ++i)
		{
			DataRecord *inner = dataRecords.at(i);
			DataRecord record = inner[0];
			std::string inclString(record.getIncl());
			::leaf[i].assign(std::begin(inclString), std::end(inclString)); // assign only key to leaf
		}

		// capacity 2^targetlevel
		PQ priorityQueue(targetlevel);

		// calculate the ovc
		for (int i = 0; i < numOfbuckets; ++i)
		{
			int intValue = ::leaf[i][0] - '0';
			// 907 vs early-fence: arity = 3 (key has 3 columns); offset = 0 (compare with early-fence so differentiating index must be 0)
			// intValue = 9; arity - offset = 3 - 0 = 3; 3 * 100 + 9 = 309
			priorityQueue.push(i, (sizeOfColumn - 0) * 100 + intValue);

			// std::cout << "intValue[" << i << "]: " << intValue << std::endl;
		}

		// input is 100,000 records; if each leaf (bucket) contains 1000 records, we only need 100 buckets;
		// for the leftover buckets, fill in late fence and push
		if (numOfbuckets < priorityQueue.capacity()) // capacity = 128
		{
			for (int i = numOfbuckets; i < priorityQueue.capacity(); i++)
				priorityQueue.push(i, priorityQueue.late_fence());
		}

		std::stringstream HDD_file;
		// mode 0: 2GB or 10xGB, mode 1: 125MB
		if (mode)
			HDD_file << "output/final_output.txt";
		else
			HDD_file << "HDD/output_10GB_" << _HDD_10GB_count + singleDigitGBleft << ".txt";
		// TODO: Can only do 12 GB currently

		std::ofstream outputFile(HDD_file.str(), std::ios::binary | std::ios::app); // std::ios::app for appending
		if (!outputFile.is_open())
			std::cerr << "Error opening output file." << std::endl;

		int count = 0; // current count of the records being popped

		// output buffer : 1MB
		DataRecord *outputBuf = new DataRecord[1000]();
		int outputBufCnt = 0;
		// int outputTotalCnt = 0;
		while (static_cast<RowCount>(count) < _consumed)
		{
			// std::cout << "count:"<< count << std::endl;
			int idx = priorityQueue.pop();
			if (idx == -1)
			{
				break;
			}
			// std::strcpy(output[count], ::data[idx]);
			// std::strcpy(output[count], ::leaf[idx].data());
			if (outputBufCnt < 1000)
			{
				DataRecord *inner = dataRecords.at(idx);
				// idx tells which leaf; hashtable[idx] returns the next pointer to the record
				DataRecord output_record(inner[hashtable[idx]]);
				// add to the output buffer
				outputBuf[outputBufCnt++] = output_record;
			}
			else
			{
				// if output buffer has 1000 records (1MB)
				// write to HDD
				for (int i = 0; i < 1000; i++)
				{
					DataRecord output_record = outputBuf[i];
					outputFile.write(output_record.getIncl(), incl_size);
					outputFile.write(" ", 1);
					outputFile.write(output_record.getMem(), mem_size);
					outputFile.write(" ", 1);
					outputFile.write(output_record.getMgmt(), mgmt_size);
					outputFile.write("\r\n", 2);
				}
				outputBufCnt = 0;
				// outputBuf = new DataRecord[1000]();

				DataRecord *inner = dataRecords.at(idx);
				// idx tells which leaf; hashtable[idx] returns the next pointer to the record
				DataRecord output_record(inner[hashtable[idx]]);
				// add to the output buffer
				outputBuf[outputBufCnt++] = output_record;
			}

			if (static_cast<RowCount>(count) == _consumed - 1)
				break;

			// the total size of a bucket is 100MB = 100*1000 records
			// %8 == 0 means current page had all beeb outputted
			// from SSD input another page
			if (cntPerBucket[idx] % 8 == 0 && cntPerBucket[idx] <= bucketTotalSizeTable[idx] - 1000)
			{
				// read 8KB = 8 records from SSD
				DataRecord *inner = dataRecords.at(idx);
				// traceprintf("inner incl: %s\n", inner[hashtable[idx]].getIncl());
				int curHtPointer = hashtable[idx];
				int startToFillPointer = curHtPointer - 7;
				// std::cout << "idx:"<< idx << ", startToFillPointer:" << startToFillPointer << ", to curHtPointer:"<< curHtPointer << std::endl;
				for (int j = 0; j < 8; j++)
				{
					char *row = new char[record_size];
					_inputFiles[idx + 1]->read(row, record_size);
					row[record_size - 2] = '\0'; // last 2 bytes are newline characters
					// Extracting data from the row
					char *incl = new char[incl_size + 1];
					char *mem = new char[mem_size + 1];
					char *mgmt = new char[mgmt_size + 1];

					std::strncpy(incl, row, incl_size);
					incl[incl_size] = '\0';

					std::strncpy(mem, row + incl_size + 1, mem_size);
					mem[mem_size] = '\0';

					std::strncpy(mgmt, row + incl_size + 1 + mem_size + 1, mgmt_size);
					mgmt[mgmt_size] = '\0';

					// Creating a DataRecord
					// DataRecord* record = new DataRecord(incl_size + 1, mem_size + 1, mgmt_size + 1, incl, mem, mgmt);
					std::unique_ptr<DataRecord> record(new DataRecord(incl_size + 1, mem_size + 1, mgmt_size + 1, incl, mem, mgmt));
					inner[startToFillPointer++] = *record;
					// traceprintf("inner[startToFillPointer++] incl: %s\n", inner[startToFillPointer -1].getIncl());
					delete[] incl;
					delete[] mem;
					delete[] mgmt;
					delete[] row;
				}

				// if(curHtPointer == 999) {
				// 	cnt1MBPerBucket[idx]++;
				// }
			}
			// current 1MB bucket in Dram is empty, and there are records left in the 100MB: assign the pointer back to the beginning of the bucket
			if (hashtable[idx] == 999 && cntPerBucket[idx] <= 100000)
			{
				// std::cout << "------external phase 1 debug 8---------" << std::endl;
				// std::cout << "idx:"<< idx << ", cntPerBucket[idx]:" << cntPerBucket[idx] << std::endl;
				hashtable[idx] = -1;
			}

			if (hashtable[idx] < sizeOfBucket - 1) // check if there are record left in the bucket
			{
				char *preVal = new char[sizeOfColumn + 1];

				// assign the data outputted, so preVal can be used to compare with next value in the same bucket
				std::strcpy(preVal, ::leaf[idx].data());
				hashtable[idx]++;	 // update the pointer
				cntPerBucket[idx]++; // update the count per bucket

				// up till now, the record of the leaf is still the same as the outputted one
				DataRecord *inner_cur = dataRecords[idx];	   // inner_cur is a pointer to 1000 records, each 1kb
				DataRecord record = inner_cur[hashtable[idx]]; // hashtable[idx] has been updated

				// traceprintf("cntPerBucket[idx]: %d, cnt1MBPerBucket[idx]: %d, idx: %d ,hashtable[idx]: %d\n", cntPerBucket[idx], cnt1MBPerBucket[idx], idx, hashtable[idx]);
				// traceprintf("inner_cur incl: %s\n", record.getIncl());

				std::strcpy(::leaf[idx].data(), record.getIncl()); // assign new key to leaf
				char *curVal = new char[sizeOfColumn + 1];

				std::strcpy(curVal, ::leaf[idx].data());

				// find the offset
				int offset = -1; // the index of the value that two values start to differ
				for (int i = 0; i < sizeOfColumn; i++)
				{
					if (preVal[i] == curVal[i])
					{
						continue;
					}
					else
					{
						offset = i;
						break;
					}
				}

				// two values of the same bucket are equal
				if (offset == -1)
				{
					offset = sizeOfColumn;
				}

				// if equal, the new value should be pushed right away to the output
				if (offset == sizeOfColumn)
				{
					priorityQueue.push(idx, 0);
				}

				// two values differ, the latter in the bucket will compare with the one popped
				else
				{
					int arityOffset = sizeOfColumn - offset;
					int intValue = curVal[offset] - '0';
					// std::cout << "push val:" << curVal << std::endl;
					// std::cout << "push idx:" << idx << ", key:" << arityOffset * 100 + intValue << std::endl;
					priorityQueue.push(idx, arityOffset * 100 + intValue);
				}

				delete[] preVal;
				delete[] curVal;
			}
			else // no more record left
			{
				priorityQueue.push(idx, priorityQueue.late_fence());
			}

			count++;
		}

		// output last output buffer
		for (int i = 0; i < 1000; i++)
		{
			// outputTotalCnt++;
			DataRecord output_record = outputBuf[i];
			outputFile.write(output_record.getIncl(), incl_size);
			outputFile.write(" ", 1);
			outputFile.write(output_record.getMem(), mem_size);
			outputFile.write(" ", 1);
			outputFile.write(output_record.getMgmt(), mgmt_size);
			outputFile.write("\r\n", 2);
		}
		outputFile.close();

		delete[] hashtable;
		delete[] cntPerBucket;
		delete[] outputBuf;

		// for(int i = 0; i < 100; i++) {
		// 	std::cout << "cnt1MBPerBucket["<< i << "]:" << cnt1MBPerBucket[i] << std::endl;
		// }
	}

	/***************************************/
	/*********External Phase 2**************/
	/***************************************/

	else if (_plan->_state == EXTERNAL_PHASE_2)
	{
		// check if there's data between 1-10GB left, for example 12.5GB
		int singleDigitGBleft = _consumed % 10000000 == 0 ? 0 : 1;
		int const numOfbuckets = _HDD_10GB_count + singleDigitGBleft; // 12
		int recordPerBucket = 100 / numOfbuckets * 1000;
		// 120GB: (100 / 12) * 1000 records = 8MB  = 8MB; 8MB * 12 (buckets) = 96MB (4MB wasted for dram's 100MB)
		// 110GB: (100 / 11) * 1000 records = 9MB  = 9MB; 9MB * 11 (buckets) = 99MB (1MB wasted for dram's 100MB)
		// 12.5GB: (100 / 2) * 1000 records = 50MB = 50mb; 50MB * 2 (buckets) = 100MB
		int sizeOfBucket = recordPerBucket; // How many records are there in each of Dram's buckets

		// table to store the number of records for each buckets
		// useful when there are leftovers (32 * 1000 records, and 1 * 500 records)
		int *bucketTotalSizeTable = new int[numOfbuckets]();
		for (int i = 0; i < numOfbuckets - 1; i++)
			bucketTotalSizeTable[i] = 10000000; // for normal 10GB sorted file in HDD, size is 10000000
		bucketTotalSizeTable[numOfbuckets - 1] = singleDigitGBleft ? _consumed % 10000000 : 10000000;

		for (int i = 0; i < numOfbuckets; i++) // input data from HDD to dram
		{
			DataRecord *records = new DataRecord[recordPerBucket](); // 8MB / 1000 = 8000
			for (int k = 0; k < recordPerBucket; k++)
			{
				char *row = new char[record_size];
				_inputFiles[i]->read(row, record_size);
				row[record_size - 2] = '\0'; // last 2 bytes are newline characters
				// Extracting data from the row
				char *incl = new char[incl_size + 1];
				char *mem = new char[mem_size + 1];
				char *mgmt = new char[mgmt_size + 1];

				std::strncpy(incl, row, incl_size);
				incl[incl_size] = '\0';

				std::strncpy(mem, row + incl_size + 1, mem_size);
				mem[mem_size] = '\0';

				std::strncpy(mgmt, row + incl_size + 1 + mem_size + 1, mgmt_size);
				mgmt[mgmt_size] = '\0';

				// Creating a DataRecord
				// DataRecord* record = new DataRecord(incl_size + 1, mem_size + 1, mgmt_size + 1, incl, mem, mgmt);
				std::unique_ptr<DataRecord> record(new DataRecord(incl_size + 1, mem_size + 1, mgmt_size + 1, incl, mem, mgmt));
				records[k] = *record;
				delete[] incl;
				delete[] mem;
				delete[] mgmt;
				delete[] row;
			}
			dataRecords.push_back(records);
		}

		int copyNum = numOfbuckets;
		int targetlevel = 0;
		while (copyNum >>= 1)
			++targetlevel;

		if (!isPowerOfTwo(numOfbuckets))
			targetlevel++;

		// initialized to 0; stores pointer to the next record to be pushed for the leaf
		int *hashtable = new int[numOfbuckets]();

		// for how many records already output from each bucket (max would be 10GB / 1KB = 10,000,000 records)
		int *cntPerBucket = new int[numOfbuckets]();

		// already push 1 record from each bucket, thus init this array with 1
		for (int i = 0; i < numOfbuckets; ++i)
			cntPerBucket[i] = 1;

		leaf.resize(numOfbuckets);

		// initialize the tree leaves from dram
		for (int i = 0; i < numOfbuckets; ++i)
		{
			DataRecord *inner = dataRecords.at(i);
			DataRecord record = inner[0];
			std::string inclString(record.getIncl());
			::leaf[i].assign(std::begin(inclString), std::end(inclString)); // assign only key to leaf
																			// traceprintf("incl: %s ,mem: %s, mgmt: %s\n", record.getIncl(), record.getMem(), record.getMgmt());
		}

		// capacity = 2^targetlevel
		PQ priorityQueue(targetlevel);

		// calculate the ovc
		for (int i = 0; i < numOfbuckets; ++i)
		{
			int intValue = ::leaf[i][0] - '0';
			// 907 vs early-fence: arity = 3 (key has 3 columns); offset = 0 (compare with early-fence so differentiating index must be 0)
			// intValue = 9; arity - offset = 3 - 0 = 3; 3 * 100 + 9 = 309
			priorityQueue.push(i, (sizeOfColumn - 0) * 100 + intValue);
			// std::cout << "intValue[" << i << "]: " << intValue << std::endl;
		}

		// for the leftover buckets, fill in late fence and push
		if (numOfbuckets < priorityQueue.capacity())
		{
			for (int i = numOfbuckets; i < priorityQueue.capacity(); i++)
				priorityQueue.push(i, priorityQueue.late_fence());
		}

		// create the final output file
		std::ofstream outputFile("output/final_output.txt", std::ios::binary);
		if (!outputFile.is_open())
			std::cerr << "Error opening input file." << std::endl;
		int count = 0; // current count of the records being popped

		// output buffer : 1MB
		DataRecord *outputBuf = new DataRecord[1000]();
		int outputBufCnt = 0;

		while (static_cast<RowCount>(count) < _consumed)
		{
			int idx = priorityQueue.pop();
			if (idx == -1)
				break;

			if (outputBufCnt < 1000)
			{
				DataRecord *inner = dataRecords.at(idx); // inner is a pointer to 1000 records, each 1kb; dataRecords store 100 such pointers
				// idx tells which leaf; hashtable[idx] returns the next pointer to the record
				DataRecord output_record(inner[hashtable[idx]]);
				// add to the output buffer
				outputBuf[outputBufCnt++] = output_record;
			}
			else
			{
				// if output buffer have 1000 records(1MB)
				// write to HDD
				for (int i = 0; i < 1000; i++)
				{
					// outputTotalCnt++;
					DataRecord output_record = outputBuf[i];
					outputFile.write(output_record.getIncl(), incl_size);
					outputFile.write(" ", 1);
					outputFile.write(output_record.getMem(), mem_size);
					outputFile.write(" ", 1);
					outputFile.write(output_record.getMgmt(), mgmt_size);
					outputFile.write("\r\n", 2);
				}
				outputBufCnt = 0;
				// outputBuf = new DataRecord[1000]();

				DataRecord *inner = dataRecords.at(idx);
				// idx tells which leaf; hashtable[idx] returns the next pointer to the record
				DataRecord output_record(inner[hashtable[idx]]);
				// add to the output buffer
				outputBuf[outputBufCnt++] = output_record;
			}

			if (static_cast<RowCount>(count) == _consumed - 1)
				break;

			// %1000 == 0 means current 8MB bucket has 1MB (1000 records) outputted
			// from HDD input another 1MB
			// 10GB is 10,000,000 records; the first 8,000 records was in the initialization of Dram
			if (cntPerBucket[idx] % 1000 == 0 && cntPerBucket[idx] <= bucketTotalSizeTable[idx] - recordPerBucket) // 9,992,000
			{
				// read 1MB = 1000 records from HDD
				DataRecord *inner = dataRecords.at(idx);
				int curHtPointer = hashtable[idx];
				int startToFillPointer = curHtPointer - 999; // the records before startToFillPointer have been outputted
				// std::cout << "idx:"<< idx << ", startToFillPointer:" << startToFillPointer << ", to curHtPointer:"<< curHtPointer << std::endl;
				for (int j = 0; j < 1000; j++)
				{
					char *row = new char[record_size];
					_inputFiles[idx]->read(row, record_size);
					row[record_size - 2] = '\0'; // last 2 bytes are newline characters
					// Extracting data from the row
					char *incl = new char[incl_size + 1];
					char *mem = new char[mem_size + 1];
					char *mgmt = new char[mgmt_size + 1];

					std::strncpy(incl, row, incl_size);
					incl[incl_size] = '\0';

					std::strncpy(mem, row + incl_size + 1, mem_size);
					mem[mem_size] = '\0';

					std::strncpy(mgmt, row + incl_size + 1 + mem_size + 1, mgmt_size);
					mgmt[mgmt_size] = '\0';

					// Creating a DataRecord
					// DataRecord* record = new DataRecord(incl_size + 1, mem_size + 1, mgmt_size + 1, incl, mem, mgmt);
					std::unique_ptr<DataRecord> record(new DataRecord(incl_size + 1, mem_size + 1, mgmt_size + 1, incl, mem, mgmt));
					inner[startToFillPointer++] = *record;
					delete[] incl;
					delete[] mem;
					delete[] mgmt;
					delete[] row;
				}
			}
			// current 8MB bucket in Dram is empty, and there are records left in the 10GB bucket: assign the pointer back to the beginning of the bucket
			if (hashtable[idx] == recordPerBucket - 1 && cntPerBucket[idx] <= 10000000)
			{
				// std::cout << "------external phase 1 debug 8---------" << std::endl;
				// std::cout << "idx:"<< idx << ", cntPerBucket[idx]:" << cntPerBucket[idx] << std::endl;
				hashtable[idx] = -1;
			}

			if (hashtable[idx] < sizeOfBucket - 1) // check if there are record left in the bucket
			{
				char *preVal = new char[sizeOfColumn + 1];

				// assign the data outputted, so preVal can be used to compare with next value in the same bucket
				std::strcpy(preVal, ::leaf[idx].data());
				hashtable[idx]++;	 // update the pointer
				cntPerBucket[idx]++; // update the count per bucket

				// up till now, the record of the leaf is still the same as the outputted one
				DataRecord *inner_cur = dataRecords.at(idx);   // inner_cur is a pointer to 1000 records, each 1kb
				DataRecord record = inner_cur[hashtable[idx]]; // hashtable[idx] has been updated

				std::strcpy(::leaf[idx].data(), record.getIncl()); // assign new key to leaf
				char *curVal = new char[sizeOfColumn + 1];

				std::strcpy(curVal, ::leaf[idx].data());

				// find the offset
				int offset = -1; // the index of the value that two values start to differ
				for (int i = 0; i < sizeOfColumn; i++)
				{
					if (preVal[i] == curVal[i])
					{
						continue;
					}
					else
					{
						offset = i;
						break;
					}
				}

				// two values of the same bucket are equal
				if (offset == -1)
				{
					offset = sizeOfColumn;
				}

				// if equal, the new value should be pushed right away to the output
				if (offset == sizeOfColumn)
				{
					priorityQueue.push(idx, 0);
				}

				// two values differ, the latter in the bucket will compare with the one popped
				else
				{
					int arityOffset = sizeOfColumn - offset;
					int intValue = curVal[offset] - '0';
					// std::cout << "push val:" << curVal << std::endl;
					// std::cout << "push idx:" << idx << ", key:" << arityOffset * 100 + intValue << std::endl;
					priorityQueue.push(idx, arityOffset * 100 + intValue);
				}

				delete[] preVal;
				delete[] curVal;
			}
			else // no more record left
			{
				priorityQueue.push(idx, priorityQueue.late_fence());
			}

			count++;
		}

		// output last output buffer
		for (int i = 0; i < 1000; i++)
		{
			// outputTotalCnt++;
			DataRecord output_record = outputBuf[i];
			outputFile.write(output_record.getIncl(), incl_size);
			outputFile.write(" ", 1);
			outputFile.write(output_record.getMem(), mem_size);
			outputFile.write(" ", 1);
			outputFile.write(output_record.getMgmt(), mgmt_size);
			outputFile.write("\r\n", 2);
		}
		outputFile.close();
		delete[] hashtable;
		delete[] cntPerBucket;
		delete[] outputBuf;

		// for(int i = 0; i < 100; i++) {
		// 	std::cout << "cnt1MBPerBucket["<< i << "]:" << cnt1MBPerBucket[i] << std::endl;
		// }
	}

	delete _input;

	traceprintf("consumed %lu rows\n",
				(unsigned long)(_consumed));
} // SortIterator::SortIterator

SortIterator::~SortIterator()
{
	TRACE(true);

	traceprintf("produced %lu of %lu rows\n",
				(unsigned long)(_produced),
				(unsigned long)(_consumed));
} // SortIterator::~SortIterator

bool SortIterator::next()
{
	// TRACE(true);

	if (_produced >= _consumed)
		return false;

	++_produced;
	return true;
} // SortIterator::next

DataRecord *SortIterator::getCurrentRecord()
{
	return _currentRecord;
}