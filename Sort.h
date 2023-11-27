#include "Iterator.h"
#include "SortState.h"
#include <fstream>

class SortPlan : public Plan
{
	friend class SortIterator;

public:
	SortPlan(Plan *const input, SortState state, std::ifstream *inputFile, int fileCount);
	~SortPlan();
	Iterator *init() const;

private:
	Plan *const _input;
	SortState _state;
	std::ifstream *_inputFile;
	int _fileCount;

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
	std::ifstream *_inputFile;
	int _fileCount;
}; // class SortIterator
