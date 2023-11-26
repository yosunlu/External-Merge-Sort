#include <fstream>
#include "Sort.h"
#include "Scan.h"
#include <sys/types.h>
#include <sys/sysctl.h>
#include <fstream>
#include "Dram.h"
#include "Leaf.h"
#include "PQ.h"

#define REC_SIZE 1000

SortPlan::SortPlan(Plan *const input, SortState state, std::ifstream *inputFile) : _input(input), _state(state), _inputFile(inputFile)
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

bool isPowerOfTwo(int x) {
    return (x > 0) && ((x & (x - 1)) == 0);
}

SortIterator::SortIterator(SortPlan const *const plan) : _plan(plan), _input(plan->_input->init()), // _input is a ScanPlan, so init() will return ScanIterator
														 _consumed(0), _produced(0), _inputFile(plan->_inputFile)
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

	// run generation phase1
	if(_plan->_state == RUN_PHASE_1) {
		DataRecord *records = new DataRecord[_consumed]();
		int i = 0;
		int j = _consumed;
		while (j--)
		{
			char row[REC_SIZE];
			_inputFile->read(row, sizeof(row));
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

		delete[] records;
	} else if(_plan->_state == RUN_PHASE_2) {
		// 1000 records per bucket
		int sizeOfBucket = 1000;
		int const buckets = _consumed/1000;
		int copyNum = buckets;
		int targetlevel = 0;
		while (copyNum >>= 1)
			++targetlevel;
		
		if(!isPowerOfTwo(buckets)){
			targetlevel++;
		}

		// Resize the leaf vector
		// numOfBucket = buckets;
		// leaf.resize(numOfBucket, std::vector<char>(sizeOfColumn));
		int *hashtable = new int[buckets]();
		
		// buckets from Dram
		for (int i = 0; i < buckets; ++i)
		{
			DataRecord *record = dataRecords[i][0];
			std::string inclString(record->getIncl());
			::leaf[i].assign(std::begin(inclString), std::end(inclString));
		}

		// capacity 2^targetlevel
    	PQ priorityQueue(targetlevel);

		// calculate the ovc
		for (int i = 0; i < buckets; ++i)
		{
			// size 3 - 0 = 3
			int intValue = ::leaf[i][0] - '0';
			priorityQueue.push(i, (sizeOfColumn - 0) * 100 + intValue);
			// std::cout << "intValue[" << i << "]: " << intValue << std::endl;
		}

		if(buckets < priorityQueue.capacity()) {
			for(int i = buckets; i < priorityQueue.capacity(); i++) {
				priorityQueue.push(i, priorityQueue.late_fence());
			}
		}

		std::ofstream outputFile("output.txt", std::ios::binary);

		if (!outputFile.is_open())
		{
			std::cerr << "Error opening output file." << std::endl;
		}
		int count = 0;
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
			DataRecord *output_record = dataRecords[idx][hashtable[idx]];
			outputFile.write(output_record->getIncl(), 332);
			outputFile.write(" ", 1);
			outputFile.write(output_record->getMem(), 332);
			outputFile.write(" ", 1);
			outputFile.write(output_record->getMgmt(), 332);
			outputFile.write("\r\n", 2);

			if (count == buckets * sizeOfBucket - 1)
				break;
			if (hashtable[idx] < sizeOfBucket - 1)
			{
				char preVal[sizeOfColumn + 1];
				// std::strcpy(preVal, ::data[idx]);
				std::strcpy(preVal, ::leaf[idx].data());
				hashtable[idx]++;
				//std::strcpy(::data[idx], buckets[idx][hashtable[idx]]);
				DataRecord *record = dataRecords[idx][hashtable[idx]];
				std::strcpy(::leaf[idx].data(), record->getIncl());
				char curVal[sizeOfColumn + 1];
				// std::strcpy(curVal, ::data[idx]);
				std::strcpy(curVal, ::leaf[idx].data());
				// find the offset
				int offset = -1;
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
				if (offset == -1)
				{
					offset = sizeOfColumn;
				}

				if (offset == sizeOfColumn)
				{
					priorityQueue.push(idx, 0);
				}
				else
				{
					int arityOffset = sizeOfColumn - offset;
					int intValue = curVal[offset] - '0';
					// std::cout << "push val:" << curVal << std::endl;
					// std::cout << "push idx:" << idx << ", key:" << arityOffset * 100 + intValue << std::endl;
					priorityQueue.push(idx, arityOffset * 100 + intValue);
				}
			}
			else
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