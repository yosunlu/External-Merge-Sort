#include "PQ.h"
#include <iostream>
#include <limits.h>

using namespace std;
int const sizeOfColumn = 100; // the size of the value being compared (e.g. 3 for 907)

// representing 8 leaves, each a string with size sizeOfColumn
char data[8][sizeOfColumn];

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
    : height(h), heap(new Node[1 << h])
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
    return PQ::Key((arityoff * 100) + (::data[index][offset] - '0'));
}

/**
 * Compares two leaves with given 2 indexes of leaves
 * @param offset the index of the string that we we start comparing
 */
bool lessCheck(PQ::Index const left, PQ::Index const right, PQ::Offset &offset)
{
    while (++offset < sizeOfColumn)
        if (::data[left][offset] != ::data[right][offset])
            return ::data[left][offset] < ::data[right][offset];
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

int main()
{
    int const sizeOfBucket = 3; // each mini run has 3 values
    int const numOfBucket = 8;  // 8 leaves
    // 8 leaves, 3 values for each run, each value is of size 101 (including null terminator)
    char buckets[numOfBucket][sizeOfBucket][sizeOfColumn + 1] = {
        {"076o27ml575352r58sb37ot081niktr55r4jnv1415jht5vtfoe8zyz896vq08x0vd6548pjwyr9u7f05z5334n711k676i8ov1",
         "po9m2j6b4687eo3p1gwqgm8e1d8uy0e1chilb28lg8j015471ol3578b19s5089u3y40ez54p3664acfk3rup207922qbhmxmp8",
         "tk8a3g59iid582k10x1y16lj69rg2b31z5o0f4fj4fg46i77sm6tt795h8u12a2x614n7jt141aip8u31z4dk3o2p8451h132p4"},
        {"856w2cap1c92wcr79a2ac8z6pv8t1rms805p52mxw8806ig1xo4l9fyygq4cs317pg579bc24hv96hu34z5q7zf38384hd1881j",
         "98yq9xq39s97z8f6lf91265rd3uqeli438pj346941548l2azi7887c158befo80sle121nbqb6q9h0660ysoc8wf77d5f0c54b",
         "r00m18epgi6w7wedyt27739w48gx20qal4100v4kk195y9zmm572i97wepa48a56c822m6i1m94khk276199k1939c9zh6w0f42"},
        {"1mvhi25b4eo60onzhdvq4sdz21702vd0cbbh8i887pnm344ekc5a03b45n4m0r22ab93l1m6ym8ps023tx08dre4ft79gdpys6y",
         "um33g9510fp2c0q9to05xjq2u88ipd00460orq0n9vsluue58x3z1hjkkv7oc3833f021465vod41vx4o1gi5559w932w951va9",
         "up400p5icp7dwj76a58q3z1k05ug0e8vyy4i0pquf7fcmr2806555bd916tx5ly23t4x1964kkv33vc950780780yu9ncn2x887"},
        {"66tbrrtnfxwl062xi18p4vepurkw8136220b79g9xf41s48ep7pi45934382m5m4vngqb6wodakp95m1012fsfobh8q88kjh5nj",
         "d510dtc614rx0m293qz5rx2ig5650osomin5mx66t3y7s45m41y9x7mjogy1pk038v491pky14pdh93t49050r2rm66b7or9k14",
         "v905x9k257gyv9wjv8hj5x1z42z1j0y7tq06goo11qz1q453d7552k9e6f9966ed517kg36328o0o7w936u7l77vx56y2t331c2"},
        {"61fi421a7lh33m74dzms428vl99id9rqumsfd7hc6odplj47559p294y0s9jzo616yi74l4a192390q96vcpp5i3ge6jl2sxh37",
         "69a463930151jw6kk1c9uy9f7bj5n6h7b4smc88dlqv1ahgbb4v30g94b9kldi38j0h878e3n6ls808la36kj467d64fksf7fqb",
         "n67i1l4bb94rpn00159p83o6l420wjtl58i3u6o56y46n8u1ftq5qz32855of5omc5577e5bnbrpc5s4h1yk007ysb2x6jf2514"},
        {"4ff893i72dwx0wx8dvt1e8nit2b6o53bvm2h1sn356z0et45i771954492h2pilcsag779z12ljdw41k5ox305f987m3602x381",
         "b9m36agma441k22uhw3jv95p7s33mt9jg2i6c4r75v1uh784415ez01w727662c8ln9696z816hc10m3nilbg4395v7b6pwvp9x",
         "i6u9dw6c956x2y7v30079pm65vk0zob06zdwn1y0b122768t76y0jxj23d8a9i305uu7ch0sa18a9ki697k3sk7xmsk3x441ia3"},
        {"08ow09kt01y6f78b55b969n728e06347c2ipfb97704h9t473pd4s05k75i7jc9910qw0x7sn416y4i792pbzvc5j72i88w248l",
         "7xub75mv7y526sczq9w5e03707a826294dsx4i56d0d73p9cpo73ifxj5j1ui312xs0709vt6ra5s3868ua0vv9hkdi6265m79n",
         "hg802eco1e2no6e3bg72u8o756ix7n5yb31mcq66f7aim0j3470czir17r533009r68153i415n0w99dh0e54g585e1wp98513w"},
        {"25428434j47vo1rz438x2t72b42k1ylik50b74534dptz72yd59i951769gox43r59ac21kzab44x7518pztedvl40s85415q85",
         "2zg1zf320t782c3sm8q8zg927av9700qo9g3204j387e72wrhice50ur86txp2levl0g5qz7w22vgqu59dl9f5991m5z6wpy038",
         "alt0lu6elr0u5246ad5hf3jn85d9ch6x37jny6ja1thko04u990b7607ub06dbyl1ls1o5zj18z2i5o10pnbf6ot2022x3dz8hg"}};

    // a hash table to store the how many values are left (or pointer) in each bucket
    int hashtable[numOfBucket] = {0};
    // TODO: output to a file instead of array
    char output[numOfBucket * sizeOfBucket][sizeOfColumn + 1];

    // copy the first value of each run to leaf
    for (int i = 0; i < numOfBucket; ++i)
    {
        std::strcpy(::data[i], buckets[i][0]);
    }

    for (int i = 0; i < numOfBucket; ++i)
    {
        std::cout << "data[" << i << "]: " << ::data[i] << std::endl;
    }

    int copyNum = numOfBucket; // 8
    int targetlevel = 0;

    // calculate the height of the tree
    while (copyNum >>= 1)
        ++targetlevel; // capacity = 2 ^ targetlevel = 2 ^ 3 = 8

    // initialize the tournament tree-of-loser implemented with PQ
    PQ priorityQueue(targetlevel);

    // intialize the tree with the initial leaves
    for (int i = 0; i < numOfBucket; ++i) // numOfBucket = 8
    {
        // comparing with early fence, so offset is 0th (the 0 in data[i][0])
        int intValue = ::data[i][0] - '0';
        // PQ::push(index of the leaf, ovc of the value)
        priorityQueue.push(i, (sizeOfColumn - 0) * 100 + intValue);
        // std::cout << "intValue[" << i << "]: " << intValue << std::endl;
    }

    std::cout << "===============================" << std::endl;
    int count = 0;

    // count < total number of values to be merge-sorted
    while (count < numOfBucket * sizeOfBucket)
    {
        // pop the root of the tree
        // assign index of the leaf that the root originated from
        int idx = priorityQueue.pop();
        if (idx == -1)
        {
            break;
        }
        // copy the value of the leaf (was the root) to output
        std::strcpy(output[count], ::data[idx]);

        if (count == numOfBucket * sizeOfBucket - 1)
            break;

        // check if the current bucket has value left
        // each bucket is initialized with 0, and all of them have been pushed to tree,
        // so we can only use sizeOfBucket - 1
        // E.g. 3 values in each bucket: the first value has already been pushed,
        // we can only use 2
        if (hashtable[idx] < sizeOfBucket - 1)
        {

            char preVal[sizeOfColumn + 1];
            // copy the value that was popped to preVal
            std::strcpy(preVal, ::data[idx]);
            // increment the current bucket
            hashtable[idx]++;
            // update the leaf with the next value in bucket
            std::strcpy(::data[idx], buckets[idx][hashtable[idx]]);
            char curVal[sizeOfColumn + 1];
            std::strcpy(curVal, ::data[idx]);
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
            // two values of the same bucket are equal
            if (offset == -1)
            {
                offset = sizeOfColumn;
            }
            // if equal, the new value should be pushed right away to the output
            if (offset == sizeOfColumn)
            {
                priorityQueue.push(idx, 0);
            }
            else
            // ovc for the new value should be calculated based on old ovc
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
            priorityQueue.push(idx, priorityQueue.late_fence());
        }

        count++;
    }

    for (int i = 0; i < numOfBucket * sizeOfBucket; i++)
    {
        std::cout << "output[" << i << "]: " << output[i] << std::endl;
    }

    return 0;
}