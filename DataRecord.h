#ifndef DATA_RECORD_H
#define DATA_RECORD_H
#include <iostream>
#include <cstring>
#include "defs.h"

class DataRecord
{
public:
    DataRecord() {}
    DataRecord(const char incl[333], const char mem[333], const char mgmt[333])
    {
        // Copy the provided char array to _incl
        std::strncpy(_incl, incl, sizeof(_incl) - 1);
        _incl[sizeof(_incl) - 1] = '\0'; // Ensure null-termination

        std::strncpy(_mem, mem, sizeof(_mem) - 1);
        _mem[sizeof(_mem) - 1] = '\0';

        std::strncpy(_mgmt, mgmt, sizeof(_mgmt) - 1);
        _mgmt[sizeof(_mgmt) - 1] = '\0';
    }
    const char *getIncl() const { return _incl; }
    const char *getMem() const { return _mem; }
    const char *getMgmt() const { return _mgmt; }

private:
    char _incl[333];
    char _mem[333];
    char _mgmt[333];
};

#endif // DATA_RECORD_H
