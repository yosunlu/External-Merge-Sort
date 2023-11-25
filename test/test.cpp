#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include "DataRecord.h"

void generateRandomString(char *arr, int size)
{
    for (int i = 0; i < size - 1; ++i)
    {
        if (std::rand() % 2 == 0)
            arr[i] = 'a' + std::rand() % 26; // Lowercase letters
        else
            arr[i] = '0' + std::rand() % 10; // Numbers
    }
    arr[size - 1] = '\0';
}

int main()
{
    std::srand(std::time(NULL)); // Seed for random number generation

    std::ofstream outFile("data_records.txt", std::ios::out);

    if (!outFile)
    {
        std::cerr << "Error opening file for writing." << std::endl;
        return 1;
    }

    char incl[341], mem[341], mgmt[342];

    for (int i = 0; i < 1000000; ++i)
    {
        generateRandomString(incl, 341);
        generateRandomString(mem, 341);
        generateRandomString(mgmt, 342);

        DataRecord record(incl, mem, mgmt);
        // std::cout << sizeof(record) << std::endl;

        outFile << record.getIncl() << record.getMem() << record.getMgmt() << std::endl;
    }

    outFile.close();
    return 0;
}
