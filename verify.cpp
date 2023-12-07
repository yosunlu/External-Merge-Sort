#include "verify.h"
#include <cstring>
#include <fstream>
#include <zlib.h> /* use crc32() function */
#include <iostream>

/**
    add16 - add two 16-byte numbers
    u16 is defined as a structure containing two 64-bit unsigned integers (u8): hi8 and lo8.
 */
u16 add16(u16 a, u16 b)
{
    u16 sum;
    u8 reshibit, hibit0, hibit1;

    sum.hi8 = a.hi8 + b.hi8;

    hibit0 = (a.lo8 & 0x8000000000000000LL);
    hibit1 = (b.lo8 & 0x8000000000000000LL);
    sum.lo8 = a.lo8 + b.lo8;
    reshibit = (sum.lo8 & 0x8000000000000000LL);
    if ((hibit0 & hibit1) || ((hibit0 ^ hibit1) && reshibit == 0LL))
        sum.hi8++; /* add carry bit */
    return (sum);
}

int main(int argc, char *argv[])
{
    int opt;
    // Using getopt to parse command line arguments
    // ./verify -i input/input.txt -o output/output.txt -s 1000
    std::string input_file;
    std::string output_file;
    int record_size;
    while ((opt = getopt(argc, argv, "i:o:s:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            // number following -i will be the unsorted input records
            input_file = optarg;
            break;
        case 'o':
            output_file = optarg;
            break;
        case 's':
            record_size = std::atoi(optarg);
            break;
        default: /* '?' */
            std::cerr << "Usage: " << argv[0] << " -i input/input.txt -o output/output.txt -s 1000" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    std::ifstream inputFile(input_file, std::ios::binary);

    if (!inputFile.is_open())
    {
        std::cerr << "Error opening input file." << std::endl;
        return 1;
    }

    u16 temp16 = {0LL, 0LL};
    u16 one = {0LL, 1LL};
    // This structure contains two u16 fields: rec_count and checksum.
    // rec_count is used to keep track of the total number of records processed.
    // checksum is used to store the cumulative checksum of all records. Each checksum field itself is a 128-bit value represented by the u16 structure.
    struct summary input_summary = {};

    while (inputFile.peek() != EOF)
    {
        // char row[record_size];
        char* row = new char[record_size];
        inputFile.read(row, record_size);
        temp16.lo8 = crc32(0, reinterpret_cast<const Bytef *>(row), record_size);
        input_summary.checksum = add16(input_summary.checksum, temp16);
        input_summary.rec_count = add16(input_summary.rec_count, one);
        delete[] row;
    }

    inputFile.close();

    std::ifstream outputFile(output_file, std::ios::binary);

    if (!outputFile.is_open())
    {
        std::cerr << "Error opening output file." << std::endl;
        return 1;
    }

    struct summary output_summary = {};
    int i = 0;
    int key_size = record_size / 3;
    // char prev[key_size + 1];
    char* prev = new char[key_size + 1];
    bool isOrdered = true;
    int cnt = 0;
    while (outputFile.peek() != EOF)
    {
        char* row = new char[record_size];
        outputFile.read(row, record_size);
        temp16.lo8 = crc32(0, reinterpret_cast<const Bytef *>(row), record_size);
        output_summary.checksum = add16(output_summary.checksum, temp16);
        output_summary.rec_count = add16(output_summary.rec_count, one);

        // Extracting data from the row
        // char incl[key_size + 1];
        char* incl = new char[key_size + 1];
        strncpy(incl, row, key_size);
        incl[key_size] = '\0';
        if (i != 0 && strcmp(prev, incl) > 0)
        {
            std::cout << "i:" << i << ",prev:" << prev << ",cur incl:" << incl << std::endl;
            cnt++;
            isOrdered = false;
        }

        strncpy(prev, incl, key_size + 1);
        i++;
        delete[] row;
        delete[] incl;
    }

    delete[] prev;
    outputFile.close();

    if (output_summary.checksum.hi8 != input_summary.checksum.hi8 ||
        output_summary.checksum.lo8 != input_summary.checksum.lo8)
    {
        std::cerr << "Input and output files differ in the content of records." << std::endl;
    }
    else
    {
        std::cout << "Input and output files equal in the content of records." << std::endl;
    }

    if (output_summary.rec_count.hi8 != input_summary.rec_count.hi8 ||
        output_summary.rec_count.lo8 != input_summary.rec_count.lo8)
    {
        std::cerr << "Input and output files differ in the number of records." << std::endl;
    }
    else
    {
        std::cout << "Input and output files equal in the number of records." << std::endl;
    }

    if (isOrdered)
    {
        std::cout << "output file is ordered." << std::endl;
    }
    else
    {
        std::cout << "cnt:" << cnt << std::endl;
        std::cerr << "output file is unordered." << std::endl;
    }
}