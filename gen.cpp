#include "gen.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sys/stat.h>
#include "global.h"
// #include <random>

// #define REC_SIZE 1000

int genDataRecords(int rec_num)
{
    std::ofstream outputFile("input/input.txt");
    char* rec_buf = new char[record_size];
    // std::mt19937 rng(std::time(0));
    if (!outputFile.is_open())
    {
        std::cerr << "Error opening input file." << std::endl;
        return 1;
    }

    for (int row = 0; row < rec_num; ++row)
    {
        // Part 1: incl_size bytes
        for (int i = 0; i < incl_size; ++i)
        {
            rec_buf[i] = std::rand() % 2 == 0 ? 'a' + std::rand() % 26 : '0' + std::rand() % 10;
        }

        rec_buf[incl_size] = ' ';

        // Part 2: mem_size bytes
        for (int i = incl_size + 1; i < incl_size + 1 + mem_size; ++i)
        {
            rec_buf[i] = std::rand() % 2 == 0 ? 'a' + std::rand() % 26 : '0' + std::rand() % 10;
        }

        rec_buf[incl_size + 1 + mem_size] = ' ';

        // Part 3: mgmt_size bytes
        for (int i = incl_size + 1 + mem_size + 1; i < incl_size + 1 + mem_size + 1 + mgmt_size; ++i)
        {
            rec_buf[i] = std::rand() % 2 == 0 ? 'a' + std::rand() % 26 : '0' + std::rand() % 10;
        }

        /* add 2 bytes of "break" data */
        rec_buf[incl_size + 1 + mem_size + 1 + mgmt_size] = '\r'; /* nice for Windows */
        rec_buf[incl_size + 1 + mem_size + 1 + mgmt_size + 1] = '\n';

        outputFile.write(rec_buf, record_size);
    }

    delete[] rec_buf;
    outputFile.close();

    std::cout << "Data generation complete." << std::endl;

    return 0;
}
