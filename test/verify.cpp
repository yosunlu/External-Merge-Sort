#include "verify.h"
#include "qs.h"
#include <fstream>
#include <zlib.h> /* use crc32() function */
#include <cstring>

#define REC_SIZE 1000
#define REC_NUM 100

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

int main()
{
    std::ifstream inputFile("../input/input.txt", std::ios::binary);

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
        char row[REC_SIZE];
        inputFile.read(row, sizeof(row));
        temp16.lo8 = crc32(0, reinterpret_cast<const Bytef *>(row), REC_SIZE);
        input_summary.checksum = add16(input_summary.checksum, temp16);
        input_summary.rec_count = add16(input_summary.rec_count, one);
    }

    inputFile.close();

    std::ifstream outputFile("../output/final_output.txt", std::ios::binary);

    if (!outputFile.is_open())
    {
        std::cerr << "Error opening output file." << std::endl;
        return 1;
    }

    struct summary output_summary = {};
    int i = 0;
    char prev[333];
    bool isOrdered = true;
    while (outputFile.peek() != EOF)
    {
        char row[REC_SIZE];
        outputFile.read(row, sizeof(row));
        temp16.lo8 = crc32(0, reinterpret_cast<const Bytef *>(row), REC_SIZE);
        output_summary.checksum = add16(output_summary.checksum, temp16);
        output_summary.rec_count = add16(output_summary.rec_count, one);

        // Extracting data from the row
        char incl[333];
        strncpy(incl, row, 332);
        incl[332] = '\0';

        if (i != 0 && strcmp(prev, incl) > 0)
        {
            std::cout << "i:" << i << ",prev:" << prev << ",cur incl:" << incl << std::endl;
            isOrdered = false;
        }

        strncpy(prev, incl, 333);
        i++;
    }

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
        std::cerr << "output file is unordered." << std::endl;
    }
}