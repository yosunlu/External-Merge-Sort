#include "Sort.h"
#include "Scan.h"
#include <sys/types.h>
#include <sys/sysctl.h>
#include <fstream>

#define REC_SIZE 1000

SortPlan::SortPlan(Plan *const input) : _input(input)
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

SortIterator::SortIterator(SortPlan const *const plan) : _plan(plan), _input(plan->_input->init()),
														 _consumed(0), _produced(0), inputFile("input/input.txt")
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


	// run generation
	std::ifstream inputFile("input/input.txt", std::ios::binary);

	// for (int i = 0; i < 100; i++)
	// {
	// 	DataRecord *record = &records[i];
	// 	// traceprintf("for loop : incl: %s ,mem: %s, mgmt: %s\n", record->getIncl(), record->getMem(), record->getMgmt());
	// }


    if (!inputFile.is_open()) {
        std::cerr << "Error opening input file." << std::endl;
    }

	DataRecord *records = new DataRecord[_consumed]();
	int i = 0;
    while (inputFile.peek() != EOF) {
        char row[REC_SIZE];
        inputFile.read(row, sizeof(row));
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
        // Outputting the content
        // std::cout << "incl: " << record.getIncl() << "\n";
        // std::cout << "mem: " << record.getMem() << "\n";
        // std::cout << "mgmt: " << record.getMgmt() << "\n";
        // std::cout << "-----------------\n";
    }

    inputFile.close();

	// for (int i = 0; i < 100; i++)
	// {
	// 	DataRecord *record = &records[i];
	// 	traceprintf("for loop : incl: %s ,mem: %s, mgmt: %s\n", record->getIncl(), record->getMem(), record->getMgmt());
	// }

	qs(records, 0, _consumed - 1);

	// for (int i = 0; i < _consumed; i++)
	// {
	// 	DataRecord *record = &records[i];
	// 	traceprintf("for loop after sort: incl: %s ,mem: %s, mgmt: %s\n", record->getIncl(), record->getMem(), record->getMgmt());
	// }

	 // write to output.txt
    std::ofstream outputFile("output.txt", std::ios::binary);

    if (!outputFile.is_open()) {
        std::cerr << "Error opening output file." << std::endl;
    }

    for (int j = 0; j < _consumed; j++) {
        outputFile.write(records[j].getIncl(), 332);
        outputFile.write(" ", 1);
        outputFile.write(records[j].getMem(), 332);
        outputFile.write(" ", 1);
        outputFile.write(records[j].getMgmt(), 332);
        outputFile.write("\r\n", 2);
    }

    outputFile.close();

    delete[] records;

	// for (int i = 0; i < 100; i++)
	// {
	// 	DataRecord *record = &records[i];
	// 	// traceprintf("for loop after sort: incl: %s ,mem: %s, mgmt: %s\n", record->getIncl(), record->getMem(), record->getMgmt());
	// }


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
	TRACE(true);

	if (_produced >= _consumed)
		return false;

	++_produced;
	return true;
} // SortIterator::next

DataRecord *SortIterator::getCurrentRecord()
{
	return _currentRecord;
}