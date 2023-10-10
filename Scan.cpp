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

	int x = 1+ (rand() % 10000);
	int y = 1+ (rand() % 10000);
	int z = 1+ (rand() % 10000);
	_currentRecord = new DataRecord(x, y, z);
	traceprintf("x: %d ,y: %d, z: %d\n",x,y,z);

	++ _count;
	return true;
} // ScanIterator::next

DataRecord* ScanIterator::getCurrentRecord() {
    return _currentRecord;
}
