#include "Sort.h"
#include <sys/types.h>
#include <sys/sysctl.h>

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
	DataRecord *records = new DataRecord[100]();

	int i = 0;
	while (_input->next())
	{
		++_consumed;
		DataRecord *record = _input->getCurrentRecord();
		records[i++] = *record;
		// traceprintf("incl: %d ,mem: %d, mgmt: %d\n",record->getIncl(),record->getMem(),record->getMgmt());
	}

	for (int i = 0; i < 100; i++)
	{
		DataRecord *record = &records[i];
		// traceprintf("for loop : incl: %s ,mem: %s, mgmt: %s\n", record->getIncl(), record->getMem(), record->getMgmt());
	}

	qs(records, 0, 99);

	for (int i = 0; i < 100; i++)
	{
		DataRecord *record = &records[i];
		// traceprintf("for loop after sort: incl: %s ,mem: %s, mgmt: %s\n", record->getIncl(), record->getMem(), record->getMgmt());
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