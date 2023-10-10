#pragma once

#include "defs.h"

typedef uint64_t RowCount;

class DataRecord {
public:
	DataRecord(){}
    DataRecord(int incl, int mem, int mgmt) : _incl(incl), _mem(mem), _mgmt(mgmt) {}
    int getIncl() const { return _incl; }
    int getMem() const { return _mem; }
	int getMgmt() const { return _mgmt; }
private:
    int _incl;
    int _mem;
	int _mgmt;
};

class Plan
{
	friend class Iterator;
public:
	Plan ();
	virtual ~Plan ();
	virtual class Iterator * init () const = 0;
private:
}; // class Plan

class Iterator
{
public:
	Iterator ();
	virtual ~Iterator ();
	void run ();
	virtual bool next () = 0;
	virtual DataRecord* getCurrentRecord() = 0;
private:
	RowCount _count;
}; // class Iterator
