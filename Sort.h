#include "Iterator.h"
#include "SortState.h"

class SortPlan : public Plan
{
	friend class SortIterator;

public:
	SortPlan (Plan * const input, SortState state);
	~SortPlan ();
	Iterator * init () const;
private:
	Plan * const _input;
	SortState _state;
}; // class SortPlan

class SortIterator : public Iterator
{
public:
	SortIterator(SortPlan const *const plan);
	~SortIterator();
	bool next();
	DataRecord *getCurrentRecord();

private:
	SortPlan const *const _plan;
	Iterator *const _input;
	RowCount _consumed, _produced;
	DataRecord *_currentRecord;
}; // class SortIterator
