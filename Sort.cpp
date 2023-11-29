#include <fstream>
#include "Sort.h"
#include "Scan.h"
#include <sys/types.h>
#include <sys/sysctl.h>
#include <fstream>
#include "Dram.h"
#include "Leaf.h"
#include "PQ.h"
#include <sstream>

#define REC_SIZE 1000

SortPlan::SortPlan(Plan *const input, SortState state, std::vector<std::ifstream *> inputFiles, int fileCount) : _input(input), _state(state), _inputFiles(inputFiles), _fileCount(fileCount)
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
	DataRecord temp = a;
	a = b;
	b = temp;
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
														 _consumed(0), _produced(0), _inputFiles(plan->_inputFiles), _fileCount(plan->_fileCount)
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
		DataRecord *records = new DataRecord[_consumed]();
		int i = 0;
		int j = _consumed;
		while (j--)
		{
			char row[REC_SIZE];
			_inputFiles[0]->read(row, sizeof(row));
			row[sizeof(row) - 2] = '\0'; // last 2 bytes are newline characters
			// Extracting data from the row
			char incl[333], mem[333], mgmt[333];
			std::strncpy(incl, row, 332);
			incl[332] = '\0';

			std::strncpy(mem, row + 333, 332);
			mem[332] = '\0';

			std::strncpy(mgmt, row + 666, 332);
			mgmt[332] = '\0';

			// Creating a DataRecord
			DataRecord record(incl, mem, mgmt);
			records[i++] = record;
		}

		qs(records, 0, _consumed - 1);
		dataRecords->push_back(records);
	}
	else if (_plan->_state == RUN_PHASE_2)
	{
		// 1000 records per bucket
		int sizeOfBucket = 1000;
		int const buckets = _consumed / 1000; // _consumed = 100000
		int copyNum = buckets;				  // copyNum = buckets = 100
		int targetlevel = 0;
		while (copyNum >>= 1)
			++targetlevel;

		if (!isPowerOfTwo(buckets))
		{
			targetlevel++;
		}
		// targetlevel = 7

		// traceprintf("buckets: %lu\n", buckets);

		// Resize the leaf vector
		// numOfBucket = buckets;
		// leaf.resize(numOfBucket, std::vector<char>(sizeOfColumn));

		int *hashtable = new int[buckets](); // initializes to 0; stores pointer to the next record to be pushed for the leaf

		// buckets from Dram
		for (int i = 0; i < buckets; ++i)
		{
			DataRecord *inner = dataRecords->at(i);
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
		if (buckets < priorityQueue.capacity()) // capacity = 128
		{
			for (int i = buckets; i < priorityQueue.capacity(); i++)
				priorityQueue.push(i, priorityQueue.late_fence());
		}

		std::stringstream filename;
		filename << "SSD-10GB/output_" << _fileCount << ".txt";

		std::ofstream outputFile(filename.str(), std::ios::binary | std::ios::app); // std::ios::app for appending
		if (!outputFile.is_open())
			std::cerr << "Error opening output file." << std::endl;

		int count = 0; // current count of the records being popped
		while (count < buckets * sizeOfBucket)
		{
			int idx = priorityQueue.pop();
			if (idx == -1)
			{
				break;
			}
			// std::strcpy(output[count], ::data[idx]);
			// std::strcpy(output[count], ::leaf[idx].data());

			// write to output file
			// inner is a pointer to 1000 records, each 1kb
			DataRecord *inner = dataRecords->at(idx);
			// idx tells which leaf; hashtable[idx] returns the next pointer to the record
			DataRecord output_record = inner[hashtable[idx]];
			outputFile.write(output_record.getIncl(), 332);
			outputFile.write(" ", 1);
			outputFile.write(output_record.getMem(), 332);
			outputFile.write(" ", 1);
			outputFile.write(output_record.getMgmt(), 332);
			outputFile.write("\r\n", 2);

			if (count == buckets * sizeOfBucket - 1)
				break;
			if (hashtable[idx] < sizeOfBucket - 1) // check if there are record left in the bucket
			{
				char preVal[sizeOfColumn + 1];

				// assign the data outputted, so preVal can be used to compare with next value in the same bucket
				std::strcpy(preVal, ::leaf[idx].data());
				hashtable[idx]++; // update the pointer

				// up till now, the record of the leaf is still the same as the outputted one
				DataRecord *inner_cur = dataRecords->at(idx);  // inner_cur is a pointer to 1000 records, each 1kb
				DataRecord record = inner_cur[hashtable[idx]]; // hashtable[idx] has been updated

				std::strcpy(::leaf[idx].data(), record.getIncl()); // assign new key to leaf
				char curVal[sizeOfColumn + 1];

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
			}

			else // no more record left
			{
				priorityQueue.push(idx, priorityQueue.late_fence());
			}

			count++;
		}

		outputFile.close();
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