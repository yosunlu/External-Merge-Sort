#include "PQ.h"
#include <iostream>
#include <limits.h>

using namespace std;
int const sizeOfColumn = 3;
int const numOfBucket = 10;
// char data[numOfBucket][sizeOfColumn+ 1];
std::vector< std::vector<char> > data(numOfBucket, std::vector<char>(sizeOfColumn + 1));
PQ::PQ(Level const h)
    : height(h), heap(new Node[1 << h])
{
}

PQ::~PQ()
{
    delete[] heap;
    // Perform any necessary cleanup
}

PQ::Node::Node(Index i, Key k) : index(i), key(k) {}

PQ::Index PQ::capacity() const { return Index(1 << height); }
PQ::Index PQ::root() const { return Index(0); }
void PQ::leaf(Index const index, Index &slot) const
{
    slot = capacity() + index;
}
void PQ::parent(Index &slot) const { slot /= 2; }
PQ::Key PQ::early_fence(Index const index) const { return Key(INT_MIN); }
PQ::Key PQ::late_fence(Index const index) const { return Key(INT_MAX); }

PQ::Offset offsetFromKey(PQ::Key key)
{
    // Key is ovc
    int off = sizeOfColumn - (key / 100);
    return PQ::Offset(off);
}

PQ::Key keyFromOffset(PQ::Index index, PQ::Offset offset)
{
    // calculate the ovc
    int arityoff = sizeOfColumn - offset;
    return PQ::Key((arityoff * 100) + (::data[index][offset] - '0'));
}

bool lessCheck(PQ::Index const left, PQ::Index const right, PQ::Offset &offset)
{
    while (++offset < sizeOfColumn)
        if (::data[left][offset] != ::data[right][offset])
            return ::data[left][offset] < ::data[right][offset];
    return false;
}

bool PQ::Node::less(Node &other, bool const full)
{
    Offset offset;
    if (full)
        offset = Offset(-1);
    else if (key != other.key)
        return (key < other.key);
    else if (key == -1 && other.key == -1)
        return false;
    else if (key == INT_MAX && other.key == INT_MAX)
        return false;
    else
        offset = offsetFromKey(key);
    bool const isLess = lessCheck(index, other.index, offset);
    Node &loser = (isLess ? other : *this);
    loser.key = keyFromOffset(loser.index, offset);
    // std::cout << "loser index:" << loser.index << ", key:" << loser.key << std::endl;
    return isLess;
}

void PQ::pass(Index const index, Key const key, bool full_comp)
{
    Node candidate(index, key);
    Index slot;
    // std::cout << "index:" << index << ", key:" << key << std::endl;
    for (leaf(index, slot); parent(slot),
                            slot != root() && heap[slot].index != index;)
    {
        if (heap[slot].less(candidate, full_comp))
        {
            // std::cout << "swap slot:" << slot << ", heap[slot].key:" << heap[slot].key
            // << ", heap[slot].index:" << heap[slot].index << std::endl;
            heap[slot].swap(candidate);

            // std::cout << "after swap slot:" << slot << ", heap[slot].key:" << heap[slot].key
            // << ", heap[slot].index:" << heap[slot].index << std::endl;
        }
    }
    if (candidate.index != -1)
    {
        heap[slot] = candidate;
        // std::cout << "slot:" << slot << std::endl;
    }
}

PQ::Index PQ::poptop(bool const invalidate)
{
    if (empty())
        return -1;
    if (invalidate)
        heap[root()].key = early_fence(heap[root()].index);
    return heap[root()].index;
}

bool PQ::empty()
{
    Node const &hr = heap[root()];
    while (hr.key == early_fence(hr.index))
        pass(hr.index, late_fence(hr.index), false);
    return hr.key == late_fence(hr.index);
}

void PQ::push(Index const index, Key const key)
{
    pass(index, key, false);
}

PQ::Index PQ::top() { return poptop(false); }
PQ::Index PQ::pop() { return poptop(true); }

bool isPowerOfTwo(int x) {
    return (x > 0) && ((x & (x - 1)) == 0);
}

int main()
{
    int const sizeOfBucket = 2;
    char buckets[numOfBucket][sizeOfBucket][sizeOfColumn + 1] = {
        {"123", "345"},
        {"096", "144"},
        {"322", "411"},
        {"266", "278"},
        {"054", "678"},
        {"876", "899"},
        {"765", "805"},
        {"167", "649"},
        {"799", "902"},
        {"914", "977"}};

    //int hashtable[numOfBucket] = {0};
    int *hashtable = new int[numOfBucket]();
    char output[numOfBucket * sizeOfBucket][sizeOfColumn + 1];

    for (int i = 0; i < numOfBucket; ++i)
    {
        // Copy each bucket to the data vector
        ::data[i].assign(std::begin(buckets[i][0]), std::end(buckets[i][0]));
        // std::strcpy(::data[i], buckets[i][0]);
    }

    // for (int i = 0; i < numOfBucket; ++i)
    // {
    //     std::cout << "data[" << i << "]: " << ::data[i] << std::endl;
    // }

    int copyNum = numOfBucket;
    int targetlevel = 0;
    while (copyNum >>= 1)
        ++targetlevel;
    
    if(!isPowerOfTwo(numOfBucket)){
        targetlevel++;
    }

    // capacity 2^targetlevel
    PQ priorityQueue(targetlevel);

    // calculate the ovc
    for (int i = 0; i < numOfBucket; ++i)
    {
        // size 3 - 0 = 3
        int intValue = ::data[i][0] - '0';
        priorityQueue.push(i, (sizeOfColumn - 0) * 100 + intValue);
        // std::cout << "intValue[" << i << "]: " << intValue << std::endl;
    }

    if(numOfBucket < priorityQueue.capacity()) {
        for(int i = numOfBucket; i < priorityQueue.capacity(); i++) {
            priorityQueue.push(i, priorityQueue.late_fence(i));
        }
    }

    std::cout << "===============================" << std::endl;
    int count = 0;

    while (count < numOfBucket * sizeOfBucket)
    {
        int idx = priorityQueue.pop();
        if (idx == -1)
        {
            break;
        }
        // std::strcpy(output[count], ::data[idx]);
        std::strcpy(output[count], ::data[idx].data());
        if (count == numOfBucket * sizeOfBucket - 1)
            break;
        if (hashtable[idx] < sizeOfBucket - 1)
        {
            char preVal[sizeOfColumn + 1];
            // std::strcpy(preVal, ::data[idx]);
            std::strcpy(preVal, ::data[idx].data());
            hashtable[idx]++;
            //std::strcpy(::data[idx], buckets[idx][hashtable[idx]]);
            std::strcpy(::data[idx].data(), buckets[idx][hashtable[idx]]);
            char curVal[sizeOfColumn + 1];
            // std::strcpy(curVal, ::data[idx]);
            std::strcpy(curVal, ::data[idx].data());
            // find the offset
            int offset = -1;
            for (int i = 0; i < sizeOfColumn; i++)
            {
                if (preVal[i] == curVal[i])
                {
                    continue;
                }
                else
                {
                    offset = i;
                    break;
                }
            }
            if (offset == -1)
            {
                offset = sizeOfColumn;
            }

            if (offset == sizeOfColumn)
            {
                priorityQueue.push(idx, 0);
            }
            else
            {
                int arityOffset = sizeOfColumn - offset;
                int intValue = curVal[offset] - '0';
                // std::cout << "push val:" << curVal << std::endl;
                // std::cout << "push idx:" << idx << ", key:" << arityOffset * 100 + intValue << std::endl;
                priorityQueue.push(idx, arityOffset * 100 + intValue);
            }
        }
        else
        {
            priorityQueue.push(idx, priorityQueue.late_fence(idx));
        }

        count++;
    }

    for (int i = 0; i < numOfBucket * sizeOfBucket; i++)
    {
        std::cout << "output[" << i << "]: " << output[i] << std::endl;
    }

    return 0;
}