#ifndef HEAP_H
#define HEAP_H
#include <iostream>
#include <iomanip>
#include <cassert>

using namespace std;

template<typename T>
class MinHeap
{
    //define insertion operator for heap objects.
    friend ostream& operator << (ostream& outs, const MinHeap<T>& print_me)
    {
        unsigned root = 0, level = 0;
        print_me.print_tree(root,level,outs);
        outs << endl << endl;
        return outs;
    }

public:
    MinHeap();

    //big 3
    ~MinHeap();
    MinHeap(const MinHeap<T>& other);
    MinHeap<T>& operator=(const MinHeap<T>& rhs);

    size_t size() const;                    //return the current size
    size_t capacity() const;                //return the current capacity
    void insert(const T& insert_me);        //insert item at the end of the heap.
    T pop();                                //remove & return item at top of heap and replace with last item.
    bool isEmpty() const;                   //returns true if the heap is empty, otherwise false.

private:
    //recursively prints the contents of the heap to outs.
    void print_tree(size_t root, size_t level = 0, ostream& outs = cout) const;

    bool isLeaf(size_t i) const;            //returns true if i is a leaf.
    size_t parentIndex(size_t i) const;     //returns the index of i's parent.
    size_t leftChildIndex(size_t i)const;   //returns the index of i's left child.
    size_t rightChildIndex(size_t i) const; //returns the index of i's right child.
    bool hasRightChild(size_t i) const;     //returns true if i has a right child.
    bool hasLeftChild(size_t i) const;      //returns true if i has a left child.
    size_t smallChildIndex(size_t i) const;   //returns the index of i's larger child.
    void swapWithParent(size_t i);          //swaps item at index i with item at index (i-1) / 2.
    void reHeapUp();    //called after insert to restore max-heap state
    void reheapDown();  //called after pop to restore max-heap state

    //helper function to copy an array.
    void copyArray(const T * copyFrom, T *&copyTo, const size_t & copyFromSize);

     //resizes the _arr if needed if: (size = capacity, or, size <= 1/4 capacity)
    void resize();

    size_t _capacity;
    size_t _size;
    T *_arr;

};

//preconditions: copyFrom and copyTo must be initialized.
//postconditions: range: [0, copyFromSize) in copyFrom is coppied to copyTo,
// copyTo is returned by ref.
template<typename T>
void MinHeap<T>::copyArray(const T * copyFrom, T *& copyTo, const size_t & copyFromSize)
{
    for(size_t i = 0; i < copyFromSize; i++)
        copyTo[i] = copyFrom[i];
}

//preconditions: none
//postconditions: the heap will be printed from bottom right to bottom left.
template<typename T>
void MinHeap<T>::print_tree(size_t root, size_t level, ostream& outs) const
{
    if(root < _size)
    {
        if(hasRightChild(root))
            print_tree(rightChildIndex(root),level+1,outs);

        outs << setw(level*5) << *_arr[root] << endl;

        if(hasLeftChild(root))
            print_tree(leftChildIndex(root),level+1,outs);
    }
}

//preconditions: none
//postconditions: an empty heap is constructed,
// _arr is initialized to nullptr.
template<typename T>
MinHeap<T>::MinHeap()
{
    _size = 0;
    _capacity = 0;
    _arr = nullptr;
}

//preconditions: none
//postconditions: _arr is deallocated.
template<typename T>
MinHeap<T>::~MinHeap()
{
    delete [] _arr;
}

//preconditions: self assignment is not allowed.
//postconditions: this heap will be constructed
// with the contents and size / capacity of other.
template<typename T>
MinHeap<T>::MinHeap(const MinHeap<T>& other)
{
    assert(&other != this);

    _capacity = other._capacity;
    _size = other._size;
    _arr = new T[_capacity];

    copyArray(other._arr,this->_arr,_size);
}

//preconditions: self assignment is not allowed.
//postconditions: this heap will be assigned the
// contents and size / capacity of rhs.
template<typename T>
MinHeap<T>& MinHeap<T>::operator=(const MinHeap<T>& rhs)
{
    assert(&rhs != this);

    delete [] this->_arr;
    _size = rhs.size();
    _capacity = rhs.capacity();
    _arr = new T[_capacity];

    copyArray(rhs._arr,this->_arr,_size);
}

//preconditions: none
//postconditions: the current capacity will be returned.
template<typename T>
size_t MinHeap<T>::capacity() const
{
    return _capacity;
}

//preconditions: The heap must not already be empty.
//postconditions: Item at index 0 in the array will be replaced
// with the item at the end of the array and size will be decremented.
// reheapdown will be called to restore the heap property that every node
// must be larger than its children. Then, the heap size will be decreased
// if needed with a call to resize()
template<typename T>
T MinHeap<T>::pop()
{
    assert(!isEmpty());

    //replace the topmost item with the last item.
    T item = _arr[0];

    //If this is not the last item,
    //  replace the topmost item with the last item.
    if(_size > 1)
        _arr[0] = _arr[_size-1];

    _size--;

    //reheapify down.
    reheapDown();

    //decrease the size if size < 1/4 capacity.
    resize();

    return item;
}

//preconditions: none
//postconditions: returns the current size of the heap.
template<typename T>
size_t MinHeap<T>::size() const
{
    return _size;
}

//preconditions: none
//postconditions: returns true if the heap is empty, otherwise false.
template<typename T>
bool MinHeap<T>::isEmpty() const
{
    return (!_size);
}

//preconditions:  throws bad_alloc if dynamic allocation fails.
//postconditions: Increase size: if size = capacity, double the capacity.
//                Decrease size: if size < 1/4 capacity, half the capacity.
//                The contents of the current array will be coppied to the new array.
template<typename T>
void MinHeap<T>::resize()
{
    bool resize = false;
    size_t newCap = 0;

    //determine the new capacity, if the array needs to be resized.
    // Increase size: if size = capacity, double the capacity.
    // Decrease size: if size < 1/4 capacity, half the capacity.
    if(_size == _capacity)
    {
        newCap = (_capacity > 0) ? _capacity * 2 : 1;
        resize = true;
    }
    else if(_size < (_capacity / 4))
    {
        newCap = _capacity / 2;
        resize = true;
    }

    //If the array is to be resized, perform the copy operation.
    if(resize)
    {
        try
        {
            //Try to allocate a new array of newCap items.
            T *tempArr = new T[newCap];
            copyArray(_arr,tempArr,_size);

            //delete old array and make _arr point to new array.
            _capacity = newCap;
            delete [] _arr;
            _arr = tempArr;

        } catch (std::bad_alloc & ex) {
            cout << "Exception thrown when attempting to allocate: "
                 << sizeof(T)*newCap << " bytes."
                 << endl << "ex.what() : " << ex.what() << endl;
            throw;
        }
    }
}

//preconditions: if the array must be resized, bad alloc might be thrown by reSize();
//postconditions: the item will be inserted at the end of the heap.
template<typename T>
void MinHeap<T>::insert(const T& insert_me)
{
    try
    {
        //ensure that we have enough capacity to insert
        resize();
        _arr[_size] = insert_me;
        _size++;
        reHeapUp();
    } catch (...)
    {
        cout << "Caught exception rethrown by resize()" << endl
             << " Insertion failed: the heap could not be resized. " << endl;
    }
}

//preconditions: none
//postconditions: the last item in the heap is swapped with its parent
//until it is smaller than its parent, or until it is at the top of the heap.
template<typename T>
void MinHeap<T>::reHeapUp()
{
    if(_size > 1)
    {
        unsigned itemIndex = _size-1;
        unsigned parent = parentIndex(itemIndex);

        while(parent >= 0 && *_arr[itemIndex] < *_arr[parent])
        {
            swapWithParent(itemIndex);
            itemIndex = parent;
            if(parent > 0)
                parent = parentIndex(itemIndex);

        }
    }
}

//preconditions: none
//postconditions: the item at the top of the heap is swapped
// with its larger child until it is larger than both of its children,
// or until it is a leaf.
template<typename T>
void MinHeap<T>::reheapDown()
{
    unsigned root = 0;

    if(_size > 1)
    {
        unsigned smallerChildIndex = smallChildIndex(root);
        while(!isLeaf(root) && *_arr[root] > *_arr[smallerChildIndex])
        {
            swapWithParent(smallerChildIndex);
            root = smallerChildIndex;

            if(!isLeaf(root))
                smallerChildIndex = smallChildIndex(root);
        }
    }
}

//preconditions: none
//postconditions: returns true if i is a leaf, otherwise false.
// a node without a left child defined in the array is a leaf.
template<typename T>
bool MinHeap<T>::isLeaf(size_t i) const
{
    return ((i*2)+1 >= _size);
}

//preconditions: _size must be greater than 1, i must be greater than 0.
//postconditions: the index of the parent whose child is at index i is returned.
template<typename T>
size_t MinHeap<T>::parentIndex(size_t i) const
{
    //if size is 1, only index 0 is defined in the array.
    // 0 - 1 = UNSIGNED_MAX, assert to prevent segfault.
    assert(_size > 1 && i > 0);
    return ((i - 1) / 2);
}

//preconditions: i must have a left child.
//postconditions: returns the index of the left child of i.
template<typename T>
size_t MinHeap<T>::leftChildIndex(size_t i) const
{
    assert(hasLeftChild(i));
    return (2 * i) + 1;
}

//preconditions: i must have a right child.
//postconditions: returns the index of the right child of i.
template<typename T>
size_t MinHeap<T>::rightChildIndex(size_t i) const
{
    assert(hasRightChild(i));
    return (2 * i) + 2;
}

//preconditions: none
//postconditions: returns true if i has a right child, otherwise false.
template<typename T>
bool MinHeap<T>::hasRightChild(size_t i) const
{
    return ((2 * i) + 2 < _size);
}

//preconditions: none
//postconditions: returns true if i has a left child, otherwise false.
template<typename T>
bool MinHeap<T>::hasLeftChild(size_t i) const
{
    return ((2 * i) + 1 < _size);
}

//preconditions: item at i cannot be a leaf.
//postconditions: when comparing i's children, the index of the
//   child in _arr with the larger value will be returned.
template<typename T>
size_t MinHeap<T>::smallChildIndex(size_t i) const
{   
    assert(!isLeaf(i));
    size_t smallerChildIndex;

    //If the node at i has a right child, it also has a left child.
    // otherwise, it only has a left child and we can simply return that value,
    // because we have asserted that i cannot be a leaf.
    if(hasRightChild(i))
    {
        size_t leftChild = leftChildIndex(i);
        size_t rightChild = rightChildIndex(i);
        smallerChildIndex = (*_arr[leftChild] < *_arr[rightChild]) ? leftChild : rightChild;
    }
    else
    {
        smallerChildIndex = leftChildIndex(i);
    }

    return smallerChildIndex;
}

//preconditions: i cannot be at the top of a heap, as it would not have a defined parent.
//postconditions: the items at _arr[i] and _arr[(i-1)/2] are swapped.
template<typename T>
void MinHeap<T>::swapWithParent(size_t i)
{
    assert(i != 0);
    std::swap(_arr[i], _arr[parentIndex(i)]);
}

#endif // HEAP_H
