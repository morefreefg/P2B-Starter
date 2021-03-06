// Project identifier: 9504853406CBAC39EE89AA3AD238AA12CA198043

#ifndef PAIRINGPQ_H
#define PAIRINGPQ_H

#include "Eecs281PQ.h"
#include <deque>
#include <utility>
#include <iostream>
#include <queue>

// A specialized version of the 'priority queue' ADT implemented as a pairing heap.
template<typename TYPE, typename COMP_FUNCTOR = std::less <TYPE>>
class PairingPQ : public Eecs281PQ<TYPE, COMP_FUNCTOR> {
    // This is a way to refer to the base class object.
    using BaseClass = Eecs281PQ<TYPE, COMP_FUNCTOR>;

public:
    // Each node within the pairing heap
    class Node {
    public:
        explicit Node(const TYPE &val)
                : elt{val}, child{nullptr}, sibling{nullptr}, previous{nullptr} {}

        // Description: Allows access to the element at that Node's position.
        // There are two versions, getElt() and a dereference operator, use
        // whichever one seems more natural to you.
        // Runtime: O(1) - this has been provided for you.
        const TYPE &getElt() const { return elt; }

        const TYPE &operator*() const { return elt; }

        // The following line allows you to access any private data members of this
        // Node class from within the PairingPQ class. (ie: myNode.elt is a legal
        // statement in PairingPQ's add_node() function).
        friend PairingPQ;

    private:
        TYPE elt;
        Node *child;
        Node *sibling;
        Node *previous;
    }; // Node


    // Description: Construct an empty pairing heap with an optional comparison functor.
    // Runtime: O(1)
    explicit PairingPQ(COMP_FUNCTOR comp = COMP_FUNCTOR()) :
            BaseClass{comp} {
        root = nullptr;
    } // PairingPQ()


    // Description: Construct a pairing heap out of an iterator range with an optional
    //              comparison functor.
    // Runtime: O(n) where n is number of elements in range.
    template<typename InputIterator>
    PairingPQ(InputIterator start, InputIterator end, COMP_FUNCTOR comp = COMP_FUNCTOR()) :
            BaseClass{comp} {
        root = nullptr;
        count = 0;
        for (InputIterator iterator = start; iterator != end; ++iterator) {
            addNode(*iterator);
        }
    } // PairingPQ()


    // Description: Copy constructor.
    // Runtime: O(n)
    PairingPQ(const PairingPQ &other) :
            BaseClass{other.compare} {
        root = nullptr;
        if (other.root == nullptr) {
            return;
        }

        copy_non_recursive(other.root);
        count = other.size();
    } // PairingPQ()

    void copy_non_recursive(Node* other) {
        std::queue<Node*> this_queue;
        std::queue<Node*> other_queue;

        this->root = new Node(other->getElt());
        this_queue.push(this->root);
        other_queue.push(other);

        while (!other_queue.empty()) {
            Node* other_front = other_queue.front();
            Node* this_front = this_queue.front();
            other_queue.pop();
            this_queue.pop();

            Node* other_child = other_front->child;
            Node* other_sibling = other_front->sibling;

            Node* this_child = other_child == nullptr ? nullptr : new Node(other_child->getElt());
            Node* this_sibling = other_sibling == nullptr ? nullptr : new Node(other_sibling->getElt());
            // assemble connections
            this_front->child = this_child;
            this_front->sibling = this_sibling;
            if (this_child != nullptr) {
                this_child->previous = this_front;
            }

            if (this_sibling != nullptr) {
                this_sibling->previous = this_front;
            }

            if (other_child != nullptr) {
                other_queue.push(other_child);
            }

            if (other_sibling != nullptr) {
                other_queue.push(other_sibling);
            }

            if (this_child != nullptr) {
                this_queue.push(this_child);
            }

            if (this_sibling != nullptr) {
                this_queue.push(this_sibling);
            }
        }
    }

    // Description: Copy assignment operator.
    // Runtime: O(n)
    PairingPQ &operator=(const PairingPQ &rhs) {
        std::queue<Node*> q = to_queue();
        while (!q.empty()) {
            Node* node = q.front();
            node->previous = nullptr;
            node->child = nullptr;
            node->sibling = nullptr;
            q.pop();
            delete node;
        }

        copy_non_recursive(rhs.root);
        count = rhs.size();
        return *this;
    } // operator=()


    // Description: Destructor
    // Runtime: O(n)
    ~PairingPQ() {
        std::queue<Node*> q = to_queue();

        while (!q.empty()) {

            Node* node = q.front();
            node->previous = nullptr;
            node->child = nullptr;
            node->sibling = nullptr;
            q.pop();
            delete node;
        }
        count = 0;
    } // ~PairingPQ()


    // Description: Assumes that all elements inside the pairing heap are out of order and
    //              'rebuilds' the pairing heap by fixing the pairing heap invariant.
    //              You CANNOT delete 'old' nodes and create new ones!
    // Runtime: O(n)
    virtual void updatePriorities() {
        std::queue<Node*> new_queue = to_queue();
        root = nullptr;
        while (!new_queue.empty()) {
            Node* front = new_queue.front();
            front->sibling = nullptr;
            front->child = nullptr;
            front->previous = nullptr;
            root = meld(root, front);
            new_queue.pop();
        }
    } // updatePriorities()


    // Description: Add a new element to the pairing heap. This is already done.
    //              You should implement push functionality entirely in the addNode()
    //              function, and this function calls addNode().
    // Runtime: O(1)
    virtual void push(const TYPE &val) {
        addNode(val);
    } // push()


    // Description: Remove the most extreme (defined by 'compare') element from
    //              the pairing heap.
    // Note: We will not run tests on your code that would require it to pop an
    // element when the pairing heap is empty. Though you are welcome to if you are
    // familiar with them, you do not need to use exceptions in this project.
    // Runtime: Amortized O(log(n))
    virtual void pop() {
        if (root == nullptr) {
            count = 0;
            return;
        }
        if (root->child == nullptr) {
            delete root;
            root = nullptr;
            count = 0;
            return;
        }
        // using multi-pass approach

        // 1. Take all the children and break all of their existing sibling connections.
        std::queue<Node *> child_queue;
        Node* child = root->child;

        // if child doesn't own a sibling, it's an orphan, just let it be a new rootNode.
        if (child->sibling == nullptr) {
            delete root;
            root = child;
            count--;
            return;
        }

        while (child != nullptr) {
            // 2. Push all the children into a queue
            child_queue.push(child);
            child = child->sibling;
        }

        // 3. Take two heaps from the front of the queue, meld them, and push the result to the back of the queue.
        while (!child_queue.empty()) {
            Node* first = child_queue.front();
            child_queue.pop();

            if (!child_queue.empty()) {
                Node* second = child_queue.front();
                child_queue.pop();

                Node* melded = meld(first, second);
                child_queue.push(melded);
            } else {
                // 4. There is only one paring heap remaining, Done.
                delete root;
                count--;
                root = first;
                return;
            }
        }

    } // pop()


    // Description: Return the most extreme (defined by 'compare') element of
    //              the heap.  This should be a reference for speed.  It MUST be
    //              const because we cannot allow it to be modified, as that
    //              might make it no longer be the most extreme element.
    // Runtime: O(1)
    virtual const TYPE &top() const {
        return root->getElt();
    } // top()

    // pop all items to a queue
    // it can serve size() && updatePriorities();
    std::queue<Node*> to_queue() const {
        std::queue<Node *> ret_queue;

        if (root == nullptr) return ret_queue;
        std::queue<Node *> child_queue;
        child_queue.push(root);

        while (!child_queue.empty()) {
            Node* head = child_queue.front();
            ret_queue.push(head);

            Node* node = head->child;

            // push child.
            if (node != nullptr) {
                child_queue.push(node);
            }

            // push child's siblings.
            while (node != nullptr && node->sibling != nullptr) {
                node = node->sibling;
                child_queue.push(node);
            }

            // pop first
            child_queue.pop();
        }

        return ret_queue;
    }

    // Description: Get the number of elements in the pairing heap.
    // Runtime: O(1)
    virtual std::size_t size() const {
        return count;
    } // size()

    // Description: Return true if the pairing heap is empty.
    // Runtime: O(1)
    virtual bool empty() const {
        return root == nullptr;
    } // empty()


    // Description: Updates the priority of an element already in the pairing heap by
    //              replacing the element refered to by the Node with new_value.
    //              Must maintain pairing heap invariants.
    //
    // PRECONDITION: The new priority, given by 'new_value' must be more extreme
    //               (as defined by comp) than the old priority.
    //
    // Runtime: As discussed in reading material.
    void updateElt(Node* node, const TYPE &new_value) {
        node->elt = new_value;
        // it's a left most node
        if (node->previous->child == node) {
            if (node->previous != nullptr) {
                node->previous->child = node->sibling;
            }

            if (node->sibling != nullptr) {
                node->sibling->previous = node->previous;
            }
        } else {
            if (node->previous != nullptr) {
                node->previous->sibling = node->sibling;
            }
            if (node->sibling != nullptr) {
                node->sibling->previous = node->previous;
            }
        }

        node->sibling = nullptr;
        node->previous = nullptr;
        root = meld(root, node);
    } // updateElt()


    // Description: Add a new element to the pairing heap. Returns a Node* corresponding
    //              to the newly added element.
    // Runtime: O(1)
    // NOTE: Whenever you create a node, and thus return a Node *, you must be sure to
    //       never move or copy/delete that node in the future, until it is eliminated
    //       by the user calling pop().  Remember this when you implement updateElt() and
    //       updatePriorities().
    Node *addNode(const TYPE &val) {
        Node* new_node = new Node(val);

        if (root == nullptr) {
            root = new_node;
            count = 1;
            return new_node;
        }

        root = meld(new_node, root);
        count++;
        return new_node;
    } // addNode()


private:

    Node* root;

    // O(1)
    Node* meld(Node *a, Node *b) {
        if (a == nullptr) return b;
        if (b == nullptr) return a;

        // node arbitrament
        Node* greater_node = this->compare(b->getElt(), a->getElt()) ? a : b;
        Node* smaller_node = this->compare(b->getElt(), a->getElt()) ? b : a;

        Node* left_most = greater_node->child;

        smaller_node->sibling = left_most;
        if (left_most != nullptr) {
            left_most->previous = smaller_node;
        }
        smaller_node->previous = greater_node;
        greater_node->child = smaller_node;

        // return new heap header. (b)
        return greater_node;
    }

    void swap(Node* a, Node* b) {
        TYPE tmp = a->getElt();
        a->elt = b->getElt();
        b->elt = tmp;
    }

    size_t count = 0;
};


#endif // PAIRINGPQ_H
