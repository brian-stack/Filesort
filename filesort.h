#ifndef FILESORT_H
#define FILESORT_H

#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <chrono>
#include "constants.h"
#include "minheap.h"
#include "fileobject.h"
using namespace std;

class fileSort
{
public:
    fileSort(size_t recordsPerBlock, size_t recordSize, string inputName, string outputName, string path = "");
    ~fileSort();
    inline size_t getTotalItemsExtracted() const {return _totalItemsExtracted;}

private:
    size_t _recordSize;
    size_t _recordsPerBlock;

    ifstream _inFile;             //file input stream for reading from unsorted file.
    char ** _theBlock;            //dynamic 2d char array for in memory sorting.
    vector<string> _subFilePaths; //vector of subfile paths
    size_t _totalItemsExtracted;  //count of total records extracted from input file.

    //file names
    string _filePath;             //optional prefix to specify a different directory
    string _inFileName;           //name of input file.
    string _outFileName;          //name of output file

    void initBlock();             //initialize _theBlock

    //verification functions.
    bool verifyFile() const;
    bool verifyItemCount() const;
    size_t countItems(const string& filePath) const;
    bool verifyOrder(const string& filePath) const;

    //sorting functions
    void createSubfiles(); //sort input file into N subfiles.
    void mergeSubFiles();  //merge N subfiles to sorted output file
};

//preconditions: all paths must be valid, this includes all items in:
//               _subfilePaths vector and the _outFileName
//postconditions: The N subfiles will be merged into the sorted output file as follows:
//                 Construct a fileObj for each subfile, note that a fileObj consists of an
//                 input stream to a subfile and the last record read from that stream.
//                 Since each subfile is sorted, there is always a guarentee that the absolute
//                 smallest item not yet output to the final sorted file is the minimum of
//                 the last record read from each subfile that is not yet eof.
//                 Repeat the following until all input streams are eof (i.e., the minheap is empty)
//                  1) Pop the fileObj at the top of the minheap.
//                  2) Write the record from this fileObj to the final output file.
//                  3) a) If the recently popped fileObj's ifstream is empty, then delete this fileObj.
//                  3) b) Otherwise, get the next record from this fileObj's ifstream and reinsert it to the heap.
void fileSort::mergeSubFiles()
{
    clock_t start = clock();

    //construct a file object for each subfile that was created in the previous step.
    // the constructor for fileObj will open an ifstream, and read the first item
    // from that stream to its _item member.
    MinHeap<fileObj*> theHeap;
    for(string fp : _subFilePaths)
    {
        fileObj *theFileObj = new fileObj(fp,_recordSize);
        theHeap.insert(theFileObj);
    }

    //open the output file in binary mode and assert that it is open.
    ofstream fileOuts(_outFileName.c_str(),ios_base::binary);
    assert(fileOuts.is_open());

    while(!theHeap.isEmpty())
    {
        fileObj *usedFileObj = theHeap.pop();
        fileOuts.write(usedFileObj->item(),_recordSize);
        fileOuts << "\n"; //new line after each record in output file

        if(usedFileObj->fileEmpty())
            delete usedFileObj;
        else
        {
            //attempt to get the next record from usedFileObj
            // If the ifstream is not yet EOF, reinsert the fileObj to the heap.
            // otherwise, delete the fileObj, closing the stream.
            // Note that this order was used because EOF will not be true
            //  until reading past the last char in the stream.
            usedFileObj->getNextRecord();
            if(!usedFileObj->fileEmpty())
                theHeap.insert(usedFileObj);
            else
                delete usedFileObj;
        }
    }
    fileOuts.close();

    cout << "Seconds to merge all sorted subfiles: "
         << (clock() - start) / static_cast<double>(CLOCKS_PER_SEC) << endl;

    //delete the temp files from the disk.
    for(string fName : _subFilePaths)
        remove(fName.c_str());
}

//preconditions: the input file must be a valid file directory.
//postconditions: RECORDS_PER_BLOCK records will be read from the input file into a 2d char array.
// Once that array is full (or infile eof) it will be sorted in memory and output to a new subfile.
// This process will be repeated until the input file is empty.
void fileSort::createSubfiles()
{
    clock_t start = clock();
    vector<string> filePaths; //vector of subfile paths.
    size_t fileIndex = 0;     //current subfile index.

    //while the input file is not empty.
    // - read N records at a time to memory.
    // - sort the list of N records in memory.
    // - write the sorted list to the ith subfile.
    while(!_inFile.eof())
    {
        size_t itemsInBlock = 0;
        while(_inFile.good() && itemsInBlock < _recordsPerBlock)
        {
            _inFile.getline(_theBlock[itemsInBlock],_recordSize+1,'\n');

            if(_inFile.good())
            {
                _theBlock[itemsInBlock][_recordSize] = '\0';
                itemsInBlock++;
            }
        }
        if(itemsInBlock > 0)
        {
            //track the total items extracted from the original file.
            _totalItemsExtracted += itemsInBlock;

            //using std::sort for the moment.
            std::sort(_theBlock, _theBlock + itemsInBlock, [] (char const *lhs,char const *rhs) { return strcmp(lhs, rhs) < 0; });

            //save each sub-file name.
            string fileName = _filePath + "_subFile" + to_string(fileIndex) + ".txt";

            ofstream fout(fileName, ios_base::binary);
            assert(fout.is_open());

            for(size_t i = 0; i < itemsInBlock; i++)
            {
                fout.write(_theBlock[i],_recordSize);
                fout << '\n'; //new line after each item in subfiles.
            }

            fout.close();
            _subFilePaths.push_back(fileName);
            fileIndex++;
        }
    }

    _inFile.close();

    clock_t stop = clock();
    cout << "Seconds to sort and write all subfiles to disk: "
         << (stop - start) / static_cast<double>(CLOCKS_PER_SEC) << endl;
}

//preconditions: path + inputName, path + outputName must be valid directories.
//postconditions: 1) N records from the input file will be read and sorted in memory at a time,
//                   then, the N records will be output to a subfile. This process will be repeated
//                   until the input file is empty, resulting in M sorted subfiles.
//                2) Then, the subfiles will be combined into 1 sorted file by:
//                    a) Initially, insert the first item from each file, and a pointer
//                       to the input stream from which it came to a minimum heap.
//                       (this is an object that consists of an ifstream* and record r)
//                    b) i) Pop the: (record r, ifstream* if) pair at the top of the heap,
//                       ii) Write the record from that pair to the final output file.
//                       iii) If if is not eof, Read the next record from if to r
//                            and re-insert the pair to the heap. Otherwise, close the eof sub-file.
//                       Until the heap is empty, repeat step b:
fileSort::fileSort(size_t recordsPerBlock, size_t recordSize, string inputName, string outputName, string path)
         : _recordSize(recordSize), _recordsPerBlock(recordsPerBlock), _totalItemsExtracted(0), _filePath(path)
{
    _inFileName = path + inputName;
    _outFileName = path + outputName;
    _inFile.open(_inFileName,ios_base::binary);
    assert(_inFile.is_open());

    initBlock();
    createSubfiles();
    mergeSubFiles();
    verifyFile();
}

//preconditions: none
//postconditions: delete all dynamic memory and close the input file.
fileSort::~fileSort()
{
    for(size_t i = 0; i < _recordsPerBlock; i++)
        delete [] _theBlock[i];
    delete [] _theBlock;

    _inFile.close();
}

//preconditions: none
//postconditions: allocates RECORDS_PER_BLOCK char*,
// each of which will point to a char[] of size RECORD_SIZE+1 (+1 for '\0')
void fileSort::initBlock()
{
    _theBlock = new char*[_recordsPerBlock];
    for(size_t i = 0; i < _recordsPerBlock; i++)
        _theBlock[i] = new char[_recordSize+1];
}

//preconditions: _outFileName and _inFileName must be valid directories.
//postconditions: returns true if the number of records in
// _outFileName == that of _inFileName and if _outFileName is sorted.
//  otherwise, returns false.
bool fileSort::verifyFile() const
{
    bool isSorted = verifyOrder(_outFileName);
    if(isSorted)
        cout << _outFileName << " : is sorted." << endl;
    else
        cout << _outFileName << " : is NOT sorted." << endl;

    size_t inFileItems = countItems(_inFileName);
    size_t outFileItems = countItems(_outFileName);
    bool itemCountOk = (inFileItems == outFileItems);

    cout << "Item Count: " << endl
         << "Input File: " << inFileItems << endl
         << "Output File: " << outFileItems << endl;

    return (isSorted && itemCountOk);
}

//preconditions: filePath be a valid directory.
//postconditions: returns the number of items of size: RECORD_SIZE in the file.
size_t fileSort::countItems(const string& filePath) const
{
    ifstream theFile(filePath.c_str(),ios_base::binary);
    assert(theFile.is_open());
    size_t count = 0;
    char *record = new char[_recordSize+1];

    while(!theFile.eof())
    {
        theFile.getline(record,_recordSize+1,'\n');
        if(!theFile.eof())
            count++;
    }

    theFile.close();
    delete [] record;

    return count;
}

//preconditions: filePath be a valid directory.
//postconditions: return true if the file is sorted, otherwise false.
bool fileSort::verifyOrder(const string& filePath) const
{
    ifstream theSortedFile(filePath.c_str(),ios_base::binary);
    assert(theSortedFile.is_open());
    bool sorted = true;

    //allocate 2 char arrays for record comparison.
    char * current = new char[_recordSize+1];
    char * next = new char[_recordSize+1];

    //read the first record from the file.
    //theSortedFile.read(current,RECORD_SIZE);
    theSortedFile.getline(current,_recordSize+1,'\n');
    current[_recordSize] = '\0';

    //while the records are sorted and we have not reached eof:
    // 1) read the next record from the file.
    // 2) verify that current is less than next.
    // 3) delete current, store next in current,
    //    make next point to a new array.
    while(!theSortedFile.eof() && sorted)
    {
        //theSortedFile.read(next,RECORD_SIZE);
        theSortedFile.getline(next,_recordSize+1,'\n');
        next[_recordSize] = '\0';

        if(!theSortedFile.fail() && (strcmp(current,next) > 0 ))
            sorted = false;

        delete [] current;
        current = next;
        next = new char[_recordSize];
    }

    delete [] next;
    delete [] current;
    theSortedFile.close();

    return sorted;
}

#endif // FILESORT_H
