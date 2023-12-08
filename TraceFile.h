#ifndef TRACEFILE_H
#define TRACEFILE_H

#include <string>
#include <fstream>
#include "SortState.h"

class TraceFile
{
public:
    explicit TraceFile(const std::string &filename);
    ~TraceFile();

    void trace(SortState sortState);
    std::string sortStateToString(SortState state);

private:
    std::ofstream file;
    int stage;
};

#endif // TRACEFILE_H
