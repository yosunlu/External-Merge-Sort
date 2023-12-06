#ifndef DATA_RECORD_H
#define DATA_RECORD_H

#include <iostream>
#include <cstring>
#include "defs.h"

class DataRecord
{
public:
    // Default constructor
    DataRecord()
        : _incl(nullptr), _mem(nullptr), _mgmt(nullptr) {}

    // Parameterized constructor with dynamic size
    DataRecord(long inclSize, long memSize, long mgmtSize, const char incl[], const char mem[], const char mgmt[])
    {
        initialize(inclSize, memSize, mgmtSize);

        // Copy the provided char array to _incl
        std::strncpy(_incl, incl, static_cast<std::size_t>(_inclSize) - 1);
        _incl[_inclSize - 1] = '\0'; // Ensure null-termination

        std::strncpy(_mem, mem, static_cast<std::size_t>(_memSize) - 1);
        _mem[_memSize - 1] = '\0';

        std::strncpy(_mgmt, mgmt, static_cast<std::size_t>(_mgmtSize) - 1);
        _mgmt[_mgmtSize - 1] = '\0';
    }

   // Copy constructor
    DataRecord(const DataRecord& other)
    {
        initialize(other._inclSize, other._memSize, other._mgmtSize);

        std::strncpy(_incl, other._incl, static_cast<std::size_t>(_inclSize) - 1);
        _incl[_inclSize - 1] = '\0';

        std::strncpy(_mem, other._mem, static_cast<std::size_t>(_memSize) - 1);
        _mem[_memSize - 1] = '\0';

        std::strncpy(_mgmt, other._mgmt, static_cast<std::size_t>(_mgmtSize) - 1);
        _mgmt[_mgmtSize - 1] = '\0';
    }

    // Assignment operator
    DataRecord& operator=(const DataRecord& other)
    {
        if (this != &other) {
            DataRecord temp(other);
            swapContents(temp);
        }
        return *this;
    }
    // Public member function for swapping
    void swapContents(DataRecord &other)
    {
        std::swap(_incl, other._incl);
        std::swap(_mem, other._mem);
        std::swap(_mgmt, other._mgmt);
        std::swap(_inclSize, other._inclSize);
        std::swap(_memSize, other._memSize);
        std::swap(_mgmtSize, other._mgmtSize);
    }

    // Destructor to release dynamically allocated memory
    ~DataRecord()
    {
        delete[] _incl;
        delete[] _mem;
        delete[] _mgmt;
    }

    // Getter methods
    const char* getIncl() const { return _incl; }
    const char* getMem() const { return _mem; }
    const char* getMgmt() const { return _mgmt; }

private:
    char* _incl;
    char* _mem;
    char* _mgmt;
    long _inclSize;
    long _memSize;
    long _mgmtSize;

    // Helper function to initialize the dynamic memory
    void initialize(long inclSize, long memSize, long mgmtSize)
    {
        _inclSize = inclSize;
        _memSize = memSize;
        _mgmtSize = mgmtSize;

        _incl = new char[static_cast<std::size_t>(_inclSize)];
        _mem = new char[static_cast<std::size_t>(_memSize)];
        _mgmt = new char[static_cast<std::size_t>(_mgmtSize)];
    }
};

#endif // DATA_RECORD_H
