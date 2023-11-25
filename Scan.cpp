#include "Scan.h"
#include <time.h> 
#include <cstdlib>

ScanPlan::ScanPlan (RowCount const count) : _count (count)
{
	TRACE (true);
} // ScanPlan::ScanPlan

ScanPlan::~ScanPlan ()
{
	TRACE (true);
} // ScanPlan::~ScanPlan

Iterator * ScanPlan::init () const
{
	TRACE (true);
	return new ScanIterator (this);
} // ScanPlan::init

ScanIterator::ScanIterator (ScanPlan const * const plan) :
	_plan (plan), _count (0)
{
	TRACE (true);
} // ScanIterator::ScanIterator

ScanIterator::~ScanIterator ()
{
	TRACE (true);
	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_count),
			(unsigned long) (_plan->_count));
} // ScanIterator::~ScanIterator

bool ScanIterator::next ()
{
	TRACE (true);

	if (_count >= _plan->_count)
		return false;
	
	char* arr = new char[100];
	for (int i = 0; i < 99; ++i) {
		arr[i] = std::rand() % 2 == 0 ? 'a' + std::rand() % 26 : '0' + std::rand() % 10;
	}
	arr[100] = '\0';

	_currentRecord = new DataRecord(arr, arr, arr);
	traceprintf("x: %s ,y: %s, z: %s\n", arr, arr, arr);

	++ _count;
	delete[] arr;
	return true;
} // ScanIterator::next

DataRecord* ScanIterator::getCurrentRecord() {
    return _currentRecord;
}
