#include "Scan.h"
#include <time.h>
#include <cstdlib>

#define REC_SIZE 1000
ScanPlan::ScanPlan (RowCount const count) : _count (count)
{
	TRACE(true);
} // ScanPlan::ScanPlan

ScanPlan::~ScanPlan()
{
	TRACE(true);
} // ScanPlan::~ScanPlan

Iterator *ScanPlan::init() const
{
	TRACE(true);
	return new ScanIterator(this);
} // ScanPlan::init

ScanIterator::ScanIterator(ScanPlan const *const plan) : _plan(plan), _count(0)
{
	TRACE(true);
} // ScanIterator::ScanIterator

ScanIterator::~ScanIterator()
{
	TRACE(true);
	traceprintf("produced %lu of %lu rows\n",
				(unsigned long)(_count),
				(unsigned long)(_plan->_count));
} // ScanIterator::~ScanIterator

bool ScanIterator::next()
{
	// TRACE(true);

	// traceprintf("%llu...........", _plan->_count);

	if (_count >= _plan->_count)
		return false;

	// char* arr = new char[100];
	// for (int i = 0; i < 99; ++i) {
	// 	arr[i] = std::rand() % 2 == 0 ? 'a' + std::rand() % 26 : '0' + std::rand() % 10;
	// }
	// arr[100] = '\0';

	// _currentRecord = new DataRecord(arr, arr, arr);
	// traceprintf("x: %s ,y: %s, z: %s\n", arr, arr, arr);


	++ _count;
	// delete[] arr;
	return true;
} // ScanIterator::next

DataRecord *ScanIterator::getCurrentRecord()
{
	return _currentRecord;
}

// class ScanIterator {
// public:
//     // ... (other members)

//     bool next();

// private:
//     // ... (other members)

//     std::ifstream inputFile;
// };

// bool ScanIterator::next()
// {
//     TRACE(true);

//     if (_count >= _plan->_count || !inputFile.is_open())
//         return false;

//     // Assuming 'getInputRow' is the function to get a row from input.txt
//     std::string row = getInputRow();

//     // Convert the row to a DataRecord object
//     _currentRecord = new DataRecord(row.c_str(), row.c_str(), row.c_str());

//     traceprintf("x: %s, y: %s, z: %s\n", row.c_str(), row.c_str(), row.c_str());

//     ++_count;
//     return true;
// }

// std::string ScanIterator::getInputRow()
// {
//     if (!inputFile.is_open()) {
//         std::cerr << "Error: Input file not open." << std::endl;
//         return "";
//     }

//     std::string row;
//     if (!std::getline(inputFile, row)) {
//         // End of file or error reading line
//         inputFile.close(); // Close the file when all lines have been read
//         return "";
//     }

//     return row;
// }