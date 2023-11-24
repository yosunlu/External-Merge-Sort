#include <algorithm> // for std::swap

class PQ
{
public:
    typedef int Index;
    typedef int Key;
    typedef char Level;
    typedef int Offset;

    PQ(Level const h);
    ~PQ();

    Index capacity() const;
    Index root() const;
    void leaf(Index const index, Index &slot) const;
    void parent(Index &slot) const;
    void leaf(Index const index, Index &slot, Level &level) const;
    void parent(Index &slot, Level &level) const;
    Key early_fence() const;
    Key late_fence() const;

    bool empty();
    Index poptop(bool const invalidate);
    Index top();
    Index pop();
    void push(Index const index, Key const key);
    void insert(Index const index, Key const key);
    void update(Index const index, Key const key);
    void remove(Index const index);
    void pass(Index const index, Key const key, bool full_comp);

private:
    struct Node
    {
        Index index;
        Key key;
        Node() : index(-1), key(-1) {}
        Node(Index index, Key key);
        void swap(Node &other)
        {
            std::swap(index, other.index);
            std::swap(key, other.key);
        }

        bool less(Node &other, bool const full);
    } *const heap;
    Level const height;
};