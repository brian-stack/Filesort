#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <string>

//Upper limits for system resource usage
const size_t MAX_MEMORY = 8000000000; // 8 GB Memory Limit
const size_t MAX_SUBFILES = 250;      // 250 file limit
const size_t MAX_RECORD_SIZE = 1024;

//File Directories
const std::string FILE_PATH_PREFIX = ""; //use the default project directory
const std::string INPUT_FILE_NAME = "inputfile.txt";
const std::string OUTPUT_FILE_NAME = "outputfile.txt";


#endif // CONSTANTS_H
