#include "qs.h"
#include <fstream>

#define REC_SIZE 1000
#define REC_NUM 100

void swap(DataRecord& a, DataRecord& b) {
    DataRecord temp = a;
    a = b;
    b = temp;
}

int part(DataRecord records[], int lower, int upper)
{
	int left = lower;
	for (int i = lower; i < upper; i++)
	{
		if (std::strcmp(records[i].getIncl(), records[upper].getIncl()) < 0)
		{
			swap(records[i], records[left++]);
		}
	}
	swap(records[left], records[upper]);
	return left;
}

void qs(DataRecord records[], int lower, int upper)
{
	if (lower < upper)
	{
		int pivot = part(records, lower, upper);
		qs(records, lower, pivot - 1);
		qs(records, pivot + 1, upper);
	}
}

int main() {
    std::ifstream inputFile("input.txt", std::ios::binary);

    if (!inputFile.is_open()) {
        std::cerr << "Error opening input file." << std::endl;
        return 1;
    }

    DataRecord *records = new DataRecord[REC_NUM]();
    int i = 0;
    while (inputFile.peek() != EOF) {
        char row[REC_SIZE];
        inputFile.read(row, sizeof(row));
        row[sizeof(row) - 2] = '\0'; // last 2 bytes are newline characters

        // Extracting data from the row
        char incl[333], mem[333], mgmt[333];
        std::strncpy(incl, row, 332);
        incl[332] = '\0';

        std::strncpy(mem, row + 333, 332);
        mem[332] = '\0';

        std::strncpy(mgmt, row + 666, 332);
        mgmt[332] = '\0';

        // Creating a DataRecord
        DataRecord record(incl, mem, mgmt);
        records[i++] = record;
        // Outputting the content
        // std::cout << "incl: " << record.getIncl() << "\n";
        // std::cout << "mem: " << record.getMem() << "\n";
        // std::cout << "mgmt: " << record.getMgmt() << "\n";
        // std::cout << "-----------------\n";
    }

    inputFile.close();

    // quick sort
    qs(records, 0, REC_NUM - 1);

    // for(int j = 0; j < REC_NUM; j++) {
    //     std::cout << "after sort incl: " << records[j].getIncl() << "\n";
    //     std::cout << "after sort mem: " << records[j].getMem() << "\n";
    //     std::cout << "after sort mgmt: " << records[j].getMgmt() << "\n";
    //     std::cout << "-----------------\n";
    // }

    // write to output.txt
    std::ofstream outputFile("output.txt", std::ios::binary);

    if (!outputFile.is_open()) {
        std::cerr << "Error opening output file." << std::endl;
        return 1;
    }

    for (int j = 0; j < REC_NUM; j++) {
        outputFile.write(records[j].getIncl(), 332);
        outputFile.write(" ", 1);
        outputFile.write(records[j].getMem(), 332);
        outputFile.write(" ", 1);
        outputFile.write(records[j].getMgmt(), 332);
        outputFile.write("\r\n", 2);
    }

    outputFile.close();

    delete[] records;
    return 0;
}