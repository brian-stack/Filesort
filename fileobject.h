#ifndef FILEOBJECT_H
#define FILEOBJECT_H
#include <fstream>
#include <cassert>
#include "constants.h"

using namespace std;

class fileObj
{
public:
    //Comparison operators are defined for fileObj so that two file objects can be directly compared by _record.
    friend bool operator<(const fileObj& lhs, const fileObj& rhs){ return strcmp(lhs._record, rhs._record) < 0;}
    friend bool operator>(const fileObj& lhs, const fileObj& rhs){ return strcmp(lhs._record, rhs._record) > 0;}

    friend ostream& operator<<(ostream& outs, const fileObj& theItem)
    {
        outs << theItem._record;
        return outs;
    }

    fileObj(const string& thePath, size_t recordSize);
    ~fileObj() { delete [] _record; }

    void getNextRecord();
    inline const char* item() const { return _record; }
    inline bool fileEmpty() { return _inFile.eof(); }

private:
    size_t _recordSize;
    ifstream _inFile;
    char * _record;    //the last item extracted from this stream.
};

//preconditions: thePath must be a valid directory.
//postconditions: thePath will be opened by _inFile in binary mode.
//                the first record will be read from _inFile to _record.
fileObj::fileObj(const string& thePath, size_t recordSize) : _recordSize(recordSize)
{
    _record = new char[_recordSize+1];  //+1 for '\0'
    _inFile.open(thePath.c_str(),ios_base::binary);
    assert(_inFile.is_open());
    getNextRecord();
}

//preconditions: none
//postconditions: extracts the next record from _inFile and places it in _record.
//        note, _record must be null terminated so that strcmp can be used later.
void fileObj::getNextRecord()
{
    _inFile.getline(_record,_recordSize+1,'\n');
    _record[_recordSize] = '\0';
}

#endif // FILEOBJECT_H
