#ifndef _MIN_HEAP_H
#define _MIN_HEAP_H


/*
 * Min heaps are implemented using pairing heaps, as described in the paper at
 * https://www.cs.cmu.edu/~sleator/papers/pairing-heaps.pdf
 *
 *
 *
 * Elements of the heap, from the heap's point of view, only have a key value
 * (used in determining the heap invariant), a sibling/parent pointer, and a
 * left child pointer. To remove or decrease a node, a pointer to the min heap
 * node struct must be supplied by the user. On extract min/find min, a pointer
 * to the min heap node is returned, and it is up to the user to translate this
 * back to whatever structure contains this node
 *
 * The easiest way to accomplish this is by inlining the heap node struct in
 * whatever structure is to be stored in the heap
 *
 */

#include <stdint.h>
#include <stdlib.h>

typedef int64_t heap_key_t;


/*
 * Tree is implemented using leftmost child/right-sibling pointers only, and 
 * to make deletion possible, right sibling is set to parent for the rightmost
 * child
 *
 * To make delete and decrease key more efficient, make heap a half-ordered
 * binary tree, where the key of a root is no greater than any node in its left
 * subtree. This is equivalent to having a single heap node with many children,
 * except each successive child, starting from the left child, is the right
 * child of its left sibling
 *
 * We also need to store a bit in each node indicating whether it is a left
 * sibling or not. We put this in the rsib pointer
 */
typedef struct heap_node {
    // leftmost child of this node in tree. Is set to self if this node has
    // no children
    struct heap_node * lchild;
    // can also be parent if there is no right sibling
    // the LEFT_CHILD bit in this field is set iff it is the left child
    struct heap_node * rsib;

    // key associated with node, determines its priority
    heap_key_t key;
} heap_node;


// initialize heap node with key value k
#define HEAP_NODE_SET(heap_node, k) \
    ((heap_node)->key = ((heap_key_t) k))


typedef struct min_heap {
    // points to the root of the tree. Aliases lchild field of heap_node, so
    // we just keep the LEFT_CHILD bit of the root set and we don't have to
    // worry about the case of unlinking the root and needing to unset this
    // field
    heap_node * root;
} heap_t;



int heap_init(heap_t *h);

void heap_destroy(heap_t *h);


/*
 * returns a pointer to the heap node at the top of the heap (having minimum
 * key value of all heap nodes)
 */
heap_node * heap_find_min(heap_t *h);

/*
 * deletes the min heap node from the heap
 */
void heap_delete_min(heap_t *h);

/*
 * combines the effect of find_min and delete_min, returning the deleted min
 */
heap_node * heap_extract_min(heap_t *h);


/*
 * inserts a node into the tree. The node must already be initialized, i.e. its
 * key must be set
 *
 * returns 0 on success, nonzero if fails
 */
int heap_insert(heap_t *h, heap_node * node);


/*
 * joins heaps h1 and h2 into one heap, which is stored in the h1 struct. Both
 * heaps h1 and h2 are corrupted by this operation, i.e. they would need to be
 * cloned if you wanted to access them after doing this operation
 */
int heap_meld(heap_t *h1, heap_t *h2);

/*
 * decreases key value of node to the new key value
 */
int heap_decrease_key(heap_t *h, heap_node * node, heap_key_t new_key);

/*
 * deletes a node from the heap, assuming the node is actually in the heap.
 * returns 0 on success, nonzero if fails
 *
 * note: undefined behavior of node is not in the heap
 */
int heap_delete(heap_t *h, heap_node * node);




void print_heap(heap_t *h);

/*
 * validates the heap, aborting on failure and returning on success
 */
void heap_validate(heap_t *h);


#endif /* _MIN_HEAP_H */
