#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h> // For mkdir in POSIX systems

long records = 0;
long record_size = 0;

int main(int argc, char *argv[])
{
    int opt;
    std::string output_file;

    // Using getopt to parse command line arguments
    while ((opt = getopt(argc, argv, "c:s:o:")) != -1)
    {
        switch (opt)
        {
        case 'c':
            records = std::atol(optarg);
            break;
        case 's':
            record_size = std::atoi(optarg);
            break;
        case 'o':
            output_file = optarg;
            break;
        default: /* '?' */
            std::cerr << "Usage: " << argv[0] << " -c records -s size -o output_file" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // Check if all required arguments are provided
    if (records == 0 || record_size == 0 || output_file.empty())
    {
        std::cerr << "All options -c, -s, and -o must be provided" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Create input and output directories
    mkdir("input", 0777); // 0777 specifies read/write/execute for owner, group, and others
    mkdir("output", 0777);

    // Rest of your code
    std::cout << "Records: " << records << std::endl;
    std::cout << "Record Size: " << record_size << std::endl;
    std::cout << "Output File: " << output_file << std::endl;

    return 0;
}
