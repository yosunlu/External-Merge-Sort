#include "PQ.h"
#include "Leaf.h"
#include <iostream>
#include <limits.h>

using namespace std;
int sizeOfColumn = 332; // the size of the value being compared (e.g. 3 for 907)
int numOfBucket = 128;
std::vector< std::vector<char> > leaf(numOfBucket, std::vector<char>(sizeOfColumn + 1));

/**
 * Constructor for the PQ (Priority Queue) class.
 * Initializes a new priority queue with a specified height.
 *
 * The height determines the initial capacity of the queue,
 * as the internal heap structure is allocated space for
 * 2^height elements (1 << height).
 *
 * @param h The height of the priority queue, which dictates
 *          the size of the internal heap.
 */
PQ::PQ(Level const h)
    : heap(new Node[1 << h]), height(h)
{
}

PQ::~PQ()
{
    delete[] heap;
    // Perform any necessary cleanup
}

/**
 * Constructor for the nested class Node within PQ.
 * @param i The index of the leaf.
 * @param k The key of the value to be sorted (in our case the offset-value code).
 */
PQ::Node::Node(Index i, Key k) : index(i), key(k) {}

/**
 * Capacity is calculated as 2 ^ height.
 * @return capacity of the the heap
 */
PQ::Index PQ::capacity() const { return Index(1 << height); }

/**
 * root is the 0th index of the heap
 * @return 0 casted as Index
 */
PQ::Index PQ::root() const { return Index(0); }

/**
 * Return (modify value of slot) the index of the heap (loser tree) that represents the leaf
 */
void PQ::leaf(Index const index, Index &slot) const
{
    slot = capacity() + index;
}

/**
 * Return (modify value of slot) the index of the heap (loser tree) that represent the slot's parent
 */
void PQ::parent(Index &slot) const { slot /= 2; }

/**
 * Defines early_fence (negative infinity) as INT_MIN.
 */
PQ::Key PQ::early_fence() const { return Key(INT_MIN); }

/**
 * Defines late_fence (positive infinity) as INT_MAX.
 */
PQ::Key PQ::late_fence() const { return Key(INT_MAX); }

/**
 * @return the index of the value where it was compared with
 */
PQ::Offset offsetFromKey(PQ::Key key)
{
    // Key is ovc
    int off = sizeOfColumn - (key / 100);
    return PQ::Offset(off); // cast off to Offset
}

/**
 * Calculates the offset-value coding.
 * offset: the index where the two values start to differentiate.
 * Example 1: 908 vs early fence
 * offset: 0 (0th index of 908 is where 908 and early fence differentiate)
 * arityoff: 3 - 0 (size of 908 - 0)
 * 0th of 908 is 9
 * ovc = 3 * 100 + 9 = 309
 *
 * Example 2: 170 vs early fence
 * offset: 0 (0th index of 170 is where 170 and early fence differentiate)
 * arityoff: 3 - 0
 * 0th of 170 is 1
 * ovc = 3 * 100 + 1 = 301
 *
 * We're using loser-tournament tree, so 309 will lose and stay at the node.
 *
 * @param index the index of the leaf
 * @param offset the index where the two value differentiate
 * @return the offset-value coding for the leaf
 */
PQ::Key keyFromOffset(PQ::Index index, PQ::Offset offset)
{
    // calculate the ovc
    int arityoff = sizeOfColumn - offset;
    return PQ::Key((arityoff * 100) + (::leaf[index][offset] - '0'));
}

/**
 * Compares two leaves with given 2 indexes of leaves
 * @param offset the index of the string that we we start comparing
 */
bool lessCheck(PQ::Index const left, PQ::Index const right, PQ::Offset &offset)
{
    while (++offset < sizeOfColumn)
        if (::leaf[left][offset] != ::leaf[right][offset])
            return ::leaf[left][offset] < ::leaf[right][offset];
    return false;
}

/**
 * Compares this Node vs other node based on ovc(key)
 * @param other the node being compared with
 * @param full false (won't be used in our implementation)
 */
bool PQ::Node::less(Node &other, bool const full)
{
    Offset offset;
    if (full) // full will always be false in our implementation
        offset = Offset(-1);
    else if (key != other.key)
        return (key < other.key); // compare the two ovc
    else if (key == -1 && other.key == -1)
        // Nodes are intialized with -1; we shouldn't be comparing two -1
        return false;
    else if (key == INT_MAX && other.key == INT_MAX)
        return false;
    else
        // key == other.key
        // e.g. both ovc of 087 and 092 vs early fence is 300
        // offset will be modified to 1 (index 1 of 092)
        offset = offsetFromKey(key);

    // start checking from index 1st of 2 values 087 and 092
    bool const isLess = lessCheck(index, other.index, offset);

    // 087 isLess than 092, loser will be 092
    Node &loser = (isLess ? other : *this);

    // assign the new key
    loser.key = keyFromOffset(loser.index, offset);

    // std::cout << "loser index:" << loser.index << ", key:" << loser.key << std::endl;
    return isLess;
}

/**
 * Given the index of the leaf and its key (ovc), create a new node and pass it into the tree.
 * Pass the node in to the tree, and determine the new winner (last candidate)
 *
 * @param index index of the leaf
 * @param key ovc of the new node
 * @param full_comp will always be false
 */
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

/**
 * Checks if the heap is empty
 */
bool PQ::empty()
{
    Node const &hr = heap[root()];
    while (hr.key == early_fence())
        pass(hr.index, late_fence(), false);
    return hr.key == late_fence();
}

/**
 * Calls PQ::pass that creates a node with given index of the leaf and its value's ovc.
 * Pushes the node to the correct position.
 *
 * @param index index of the leaf
 * @param key the ovc of the value
 */

void PQ::push(Index const index, Key const key)
{
    pass(index, key, false);
}

PQ::Index PQ::poptop(bool const invalidate)
{
    if (empty())
        return -1;
    if (invalidate)
        heap[root()].key = early_fence();
    return heap[root()].index;
}

PQ::Index PQ::pop() { return poptop(true); }
PQ::Index PQ::top() { return poptop(false); }
