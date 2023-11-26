#include "Iterator.h"

class SortPlan : public Plan
{
	friend class SortIterator;

public:
	SortPlan(Plan *const input);
	~SortPlan();
	Iterator *init() const;

private:
	Plan *const _input;
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
