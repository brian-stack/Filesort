/*************************************************************************************************************************
 * Author: Brian Stack
 * Assignment: Filesort
 * Date: 11/26/18
 * Class: CS 8
 * CRN: 74231
 * ***********************************************************************************************************************
 * This program provides an interactive function to test the filesort algorithm with a user
 *  specified record size, records per file, and maximum memory usage for in memory sorting.
 *  Additional documentation for algorithm can be found in the filesort header.
 ************************************************************************************************************************/
#include <iostream>
#include "filesort.h"
#include "filesort_util.h"
#include "constants.h"

using namespace std;

void interactiveFileSortTest();
size_t getNumInRange(const string& message,size_t min, size_t max);

int main()
{
    srand(time(0));
    interactiveFileSortTest();
    return 0;
}

//preconditions: none
//postconditions: the fileSort algorithm will be tested on a random input file
// whose total number of records and record size are determined by the user.
void interactiveFileSortTest()
{
    cout << string(50,'-')
         << endl << "File Sort Interactive Test: "
         << endl << string(50,'-') << endl << endl;

    //Obtain the characters per record and total number of records to sort.
    size_t recordSize = getNumInRange("Characters per record? ",1,MAX_RECORD_SIZE);
    size_t totalRecords = getNumInRange("Total records to sort? ", 1,(MAX_MEMORY * MAX_SUBFILES) / MAX_RECORD_SIZE);
    size_t inputFileSize =  totalRecords * recordSize;

    //Check that the minimum required memory to sort the input file will not excede MAX_MEMORY
    // The minimum required memory will be determined by dividing inputFileSize by MAX_SUBFILES
    size_t minReqMem = std::max((inputFileSize / MAX_SUBFILES),recordSize);
    assert(minReqMem <= MAX_MEMORY);

    //Obtain the maximum number of bytes for in memory sorting from the user
    size_t maxMemory = getNumInRange("Max bytes for in memory sorting? ", minReqMem, MAX_MEMORY);
    size_t totalSubfiles = ((inputFileSize / maxMemory) == 0) ? 1 : inputFileSize / maxMemory;
    size_t recordsPerBlock = totalRecords / totalSubfiles;

    //Check that the values entered will not excede the upper limits.
    assert((inputFileSize / maxMemory) <= MAX_SUBFILES);

    //Generate the random input file based on the collected info.
    cout << endl << "Creating random input file." << endl;
    const string inputPath = FILE_PATH_PREFIX + INPUT_FILE_NAME;
    generateInputFile(inputPath.c_str(),recordSize,recordSize,totalRecords);

    cout << string(50,'-') << endl
         << "Records per subfile: " << recordsPerBlock << endl
         << "Characters per record: " << recordSize << endl
         << "Number of subfiles: " << totalSubfiles << endl
         << "Input file path: " << INPUT_FILE_NAME << endl
         << "Output file path: " << OUTPUT_FILE_NAME << endl
         << string(50,'-') << endl;

    cout << endl << "Starting File Sort" << endl << string(50,'-') << endl;

    fileSort(recordsPerBlock,recordSize,INPUT_FILE_NAME,OUTPUT_FILE_NAME,FILE_PATH_PREFIX);
}

//preconditions: min <= max
//postconditions: returns a size_t value between (inclusive) min and max
size_t getNumInRange(const string& message,size_t min, size_t max)
{
    size_t size;
    do
    {
        if(!cin.good())
        {
            cout << "Error: invalid input." << endl;
            cin.clear();
            cin.ignore(INT_MAX,'\n');
        }

        cout << message << "in range: [" << min << ", " << max << "]: ";
        cin >> size;

    } while(!cin.good() || size < min || size > max);

    return size;
}
