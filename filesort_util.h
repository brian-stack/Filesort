#ifndef FILESORT_UTIL_H
#define FILESORT_UTIL_H

#include <fstream>
#include <random>
#include <cassert>
#include <cstdlib>
using namespace std;

//preconditions: none
//postconditions: an output stream will be opened with path:
// - N records will be randomly generated and output to the file.
// - blockSize is the size of the record in bytes, itemSize is the number of those bytes that will be randomized.
//   A record will be have leading 0's if itemSize < blockSize.
void generateInputFile(const char * path, const size_t &blockSize, const size_t &itemSize, const size_t &n)
{
    ofstream outs(path,ios_base::binary);
    assert(outs.is_open());

    //declare and initialize the block (front is padded with 0's)
    //note that only the last itemSize characters of each block are random.
    char * theBlock = new char[blockSize+1];
    for(size_t i = 0; i < blockSize; i++)
        theBlock[i] = '0';
    theBlock[blockSize] = '\0';

    for(size_t i = 0; i < n; i++)
    {
        for(size_t j = 0; j < itemSize; j++)
            theBlock[blockSize-1-j] = '0' + (rand() % 10);

        outs.write(theBlock,blockSize);
        outs << "\n"; //new line after each item.
    }

    delete [] theBlock;
    outs.close();
}

#endif // FILESORT_UTIL_H
