
#include <assert.h>
#include <stdio.h>

#include <data_structs/min_heap.h>



#define HEAP_ASSERT(expr) \
    assert(expr)


#define LEFT_CHILD 0x1

// pointers must be aligned by 2 bytes for this to work (need 1 bit in rsib)
#define ALIGNMENT_REQUIREMENT 2


#define IS_LEFT_CHILD(heap_node) \
    ((((uint64_t) (heap_node)->rsib) & LEFT_CHILD) != 0)

#define IS_RIGHT_CHILD(heap_node) (!IS_LEFT_CHILD(heap_node))


#define SET_LEFT_CHILD(node) \
    ((node)->rsib = (heap_node *) (((uint64_t) (node)->rsib) | LEFT_CHILD))

#define SET_RIGHT_CHILD(node) \
    ((node)->rsib = (heap_node *) (((uint64_t) (node)->rsib) & ~LEFT_CHILD))


// this node has a parent (i.e. is not the root) if it has a right sibling.
// Note that the only child bits will not be set for the root, meaning rsib
// having value (heap_node*) 0x1 or 0x2 is not possible
#define HAS_PARENT(heap_node) \
    (((heap_node)->rsib) != NULL)

#define RSIB(node) \
    ((heap_node*) (((uint64_t) (node)->rsib) & ~LEFT_CHILD))


#define IS_ALIGNED(ptr) \
    ((((uint64_t) (ptr)) & 0x7) == 0)


// returns the right child of node if is has one, NULL otherwise
static heap_node * _right_child(heap_node * node) {
    heap_node * child = node->lchild;
    if (child != node) {
        heap_node * rsib = RSIB(child);
        if (rsib != node) {
            return rsib;
        }
        else if (IS_RIGHT_CHILD(child)) {
            return child;
        }
    }
    return NULL;
}


static void _make_root(heap_t * h, heap_node * root) {
    h->root = root;
    root->rsib = (heap_node *) (((uint64_t) h) | LEFT_CHILD);
}



int heap_init(heap_t *h) {
    // root is an alias for lchild, so when empty, we set it to itself
    h->root = (heap_node *) h;
    return 0;
}


void heap_destroy(heap_t *h) {
}




/*
 * add rchild as a right child of node, assuming node does not currently have
 * a right child
 */
static void __add_right_child(heap_node * node, heap_node * rchild) {
    heap_node * lchild = node->lchild;

    rchild->rsib = node;
    if (lchild == node) {
        // node has no children
        node->lchild = rchild;
    }
    else {
        // node has left child lchild and no right child
        HEAP_ASSERT(IS_LEFT_CHILD(lchild));
        HEAP_ASSERT(RSIB(lchild) == node);

        lchild->rsib = (heap_node *) (((uint64_t) rchild) | LEFT_CHILD);
    }
}


/*
 * add new_child to the chain of right-child nodes starting from the left
 * (any only) child of node
 *
 * i.e.
 *
 *                    5           8                    5
 *                  /           /                   /
 *  _add_child(   10      ,   12     ) =          8
 *               /  \        /  \              /     \
 *              20   15     14  16           12      10
 *                                          /  \    /  \
 *                                         14  16  20  15
 */
static void _add_child(heap_node * node, heap_node * new_child) {
    heap_node * lchild = node->lchild;

    if (lchild == node) {
        // if the node has no children, just add new_child
        node->lchild = new_child;

        // new_child is 
        new_child->rsib = (heap_node *) (((uint64_t) node) | LEFT_CHILD);
    }
    else {
        // node can only have a left child, no right child
        HEAP_ASSERT(IS_LEFT_CHILD(lchild));
        HEAP_ASSERT(RSIB(lchild) == node);

        __add_right_child(new_child, lchild);

        // now just add new_child as a left child of node
        node->lchild = new_child;
        new_child->rsib = (heap_node *) (((uint64_t) node) | LEFT_CHILD);
    }
}




/*
 * links the two subtrees rooted at n1 and n2 by making the node with smaller
 * key the new root, and making the other tree the leftmost child of the new
 * root. Returns the root of the resultant tree
 */
static heap_node * _link(heap_node * n1, heap_node * n2) {

    HEAP_ASSERT(n1 != NULL);
    HEAP_ASSERT(n2 != NULL);

    if (n2->key < n1->key) {
        heap_node * tmp = n1;
        n1 = n2;
        n2 = tmp;
    }

    // n1 is tree with smaller root

    _add_child(n1, n2);

    // return new tree
    return n1;
}



/*
 * removes the right child from the tree rooted at node and returns that
 * right child (i.e. partially removes the tree rootet at node from the
 * right-child chain of trees it's in)
 */
static heap_node * __unlink_from_rchild(heap_node * node) {
    heap_node * lchild = node->lchild;

    if (lchild != node) {
        // this node has at least one child

        // l_rsib is rsib of lchild
        heap_node * l_rsib = RSIB(lchild);

        if (IS_RIGHT_CHILD(lchild)) {
            // lchild is right child of node

            // if lchild is the right child, it must be the only child
            HEAP_ASSERT(l_rsib == node);

            // node has no left children, so it's an empty subtree
            node->lchild = node;
            return lchild;
        }
        else if (l_rsib != node) {
            // l_rsib is the right child
            HEAP_ASSERT(IS_RIGHT_CHILD(l_rsib));

            // need to unlink lchild from l_rsib (lchild now an only child, so
            // need to set LEFT_CHILD bit)
            lchild->rsib = (heap_node *) (((uint64_t) node) | LEFT_CHILD);
            return l_rsib;
        }
    }

    // otherwise, this node does not have a right child and no work needs to
    // be done
    return NULL;
}


/*
 * removes node from the heap
 */
static void _unlink(heap_node * node) {

    // even the root has a parent (the base of the heap)
    HEAP_ASSERT(HAS_PARENT(node));

    // cut the edge connecting node to its parent and link that subtree with
    // the rest of the heap

    int node_is_left_child = IS_LEFT_CHILD(node);
    heap_node * rsib = RSIB(node);

    // regardless of where node is in the heap, we have to unlink it from
    // its right child chain
    heap_node * rchild = __unlink_from_rchild(node);

    if (rsib->lchild == node) {
        // node is an only child, so rsib is actually parent
        heap_node * parent = rsib;

        if (rchild != NULL) {
            // if there was a right child, then it replaces node
            parent->lchild = rchild;

            // pass on the LEFT_CHILD bit of node
            rchild->rsib =
                (heap_node *) (((uint64_t) parent) | node_is_left_child);
        }
        else {
            // otherwise, if this node has no right child, then we need
            // to set lchild of parent to self (meaning it has no
            // children)
            parent->lchild = parent;
        }
    }
    else {
        // node has a sibling, so action depends on whether it was a left or
        // right child

        if (node_is_left_child) {

            HEAP_ASSERT(IS_RIGHT_CHILD(rsib));

            // we know LEFT_CHILD bit of rsib is not set, so we can just
            // access rsib directly without masking
            heap_node * parent = rsib->rsib;

            // now replace node with rchild
            if (rchild != NULL) {
                parent->lchild = rchild;
                rchild->rsib = (heap_node *) (((uint64_t) rsib) | LEFT_CHILD);
            }
            else {
                // if there was no replacement for node, then rsib is now
                // the only child
                parent->lchild = rsib;
            }
        }
        else /* IS_RIGHT_CHILD(node) */ {
            // if node is a right child, then rsib is actually the parent
            heap_node * parent = rsib;
            heap_node * lsib = parent->lchild;

            HEAP_ASSERT(IS_LEFT_CHILD(lsib));

            // now replace node with rchild
            if (rchild != NULL) {
                lsib->rsib = (heap_node *) (((uint64_t) rchild) | LEFT_CHILD);
                rchild->rsib = parent;
            }
            else {
                // if there was no replacement for node, then lsib is now
                // the only child
                lsib->rsib = (heap_node *) (((uint64_t) parent) | LEFT_CHILD);
            }
        }
    }
}



/*
 * returns a pointer to the heap node at the top of the heap (having minimum
 * key value of all heap nodes)
 */
heap_node * heap_find_min(heap_t *h) {
    if (h->root == (heap_node *) h) {
        return NULL;
    }
    return h->root;
}





/*
 * deletes node from its subtree, returning a new subtree without node
 *
 * First, we remove the root, then for each of the remaining k subtrees which
 * were children of node, we link them together in pairs, i.e. subtree 1 links
 * with 2, 3 with 4, and so on. If k is odd, then the last tree remains
 * unlinked. Then, starting from the next to last pair and working to the
 * front, we combine that pair into the last subtree. After this process is
 * over, the last subtree is the heap without node
 */
static heap_node * _delete_root(heap_node * node) {
    heap_node * prev_subtree = NULL;
    heap_node * next_subtree;

    heap_node * child = node->lchild;
    if (child == node) {
        // node does not have any children
        return NULL;
    }

    // to track the list of combined subtree pairs, we link them in a list
    // using the rsib field of each, as this field will be unused
    do {
        heap_node * child2 = _right_child(child);
        if (child2 != NULL) {
            next_subtree = _right_child(child2);

            // need to remove both child and child2 from list
            __unlink_from_rchild(child);
            __unlink_from_rchild(child2);

            // combine child and child2 into one subtree
            child = _link(child, child2);
        }
        else {
            next_subtree = NULL;
        }

        // append the combined tree to the list of combined trees
        child->rsib = prev_subtree;
        prev_subtree = child;

        child = next_subtree;
    } while (child != NULL);

    // now go backwards and combine all subtrees produced by the above loop
    // into one large tree

    // the head pointer to list of subtrees is prev_subtree
    child = prev_subtree->rsib;
    while (child != NULL) {
        HEAP_ASSERT((((uint64_t) child) & 0x1) == 0);
        next_subtree = child->rsib;
        prev_subtree = _link(prev_subtree, child);
        child = next_subtree;
    }

    return prev_subtree;
}


/*
 * deletes the min heap node from the heap
 */
void heap_delete_min(heap_t *h) {
    heap_node * root = h->root;
    root = _delete_root(root);

    if (root != NULL) {
        _make_root(h, root);
    }
    else {
        h->root = (heap_node *) h;
    }
}

/*
 * combines the effect of find_min and delete_min, returning the deleted min
 */
heap_node * heap_extract_min(heap_t *h) {
    heap_node * min = heap_find_min(h);
    if (min != NULL) {
        heap_delete_min(h);
    }
    return min;
}




/*
 * inserts a node into the tree. The node must already be initialized, i.e. its
 * key must be set
 *
 * returns 0 on success, nonzero if fails
 */
int heap_insert(heap_t *h, heap_node * node) {

    HEAP_ASSERT(IS_ALIGNED(node));
    // make node a one-node tree, then link it with the current root
    node->lchild = node;

    heap_node * root = h->root;

    if (root != (heap_node *) h) {
        // pass node as second argument, as it is less likely for node < root
        root = _link(root, node);

        _make_root(h, root);
    }
    else {
        // if the heap is empty, we want to set the node we are inserting as
        // the left child of the heap struct (which acts like a node that isn't
        // actually part of the heap)
        _add_child((heap_node *) h, node);
    }

    return 0;
}


/*
 * joins heaps h1 and h2 into one heap, which is stored in the h1 struct. Both
 * heaps h1 and h2 are corrupted by this operation, i.e. they would need to be
 * cloned if you wanted to access them after doing this operation
 */
int heap_meld(heap_t *h1, heap_t *h2) {
    heap_node * r1 = heap_find_min(h1);
    heap_node * r2 = heap_find_min(h2);

    if (r2 == NULL) {
        // do nothing if h2 is empty
    }
    else if (r1 == NULL) {
        _make_root(h1, r1);
    }
    else {
        r1 = _link(r1, r2);
        _make_root(h1, r1);
    }

    return 0;
}



/*
 * decreases key value of node to the new key value
 */
int heap_decrease_key(heap_t *h, heap_node * node, heap_key_t new_key) {

    heap_key_t old_key = node->key;

    // this must strictly decrease the key
    HEAP_ASSERT(old_key > new_key);

    node->key = new_key;

    // remove the subtree rooted at node from the tree, then link it with the
    // root of the tree
    _unlink(node);

    heap_node * root = h->root;
    if (root != (heap_node *) h) {
        root = _link(root, node);

        _make_root(h, root);
    }
    else {
        // if the heap is now empty, then node was the root
        HEAP_ASSERT(IS_LEFT_CHILD(node));
        h->root = node;
    }

    return 0;
}


/*
 * deletes a node from the heap, assuming the node is actually in the heap.
 * returns 0 on success, nonzero if fails
 *
 * note: undefined behavior of node is not in the heap
 */
int heap_delete(heap_t *h, heap_node * node) {
    heap_node * root = h->root;

    if (root == node) {
        // if heap_delete is called on the root, then just perform delete min
        // on the whole heap
        heap_delete_min(h);
    }
    else {
        // otherwise, we have to unlink node from the heap, call delete root
        // on the subtree, then recombine the subtree with the remainder of the
        // heap
        _unlink(node);
        heap_node * new_subtree = _delete_root(node);

        if (new_subtree != NULL) {
            // if node had any children, then new subtree will be nonnull, and
            // we have to reinsert them into the heap
            root = _link(root, new_subtree);
        }

        _make_root(h, root);
    }

    return 0;
}



static void _validate_ptrs(heap_node * node) {
    heap_node * child = node->lchild;

    if (child != node) {
        heap_node * rsib = RSIB(child);
        if (rsib != node) {
            HEAP_ASSERT(IS_LEFT_CHILD(child));
            HEAP_ASSERT(IS_RIGHT_CHILD(rsib));
            HEAP_ASSERT(child != rsib);

            _validate_ptrs(child);
            _validate_ptrs(rsib);
        }
    }
}

/*
 * validates half-ordered bst property of tree rooted at node, where node
 * is no greater than any node in left subtree
 *
 * min is min value allowed in subtree
 */
static void _validate_half_bst(heap_node * node, heap_key_t min) {
    HEAP_ASSERT(node->key >= min);
    
    heap_node * child = node->lchild;

    if (child != node) {
        if (IS_LEFT_CHILD(child)) {
            // check left subtree
            _validate_half_bst(child, node->key);

            heap_node * rchild = RSIB(child);
            if (rchild != node) {
                // check right subtree
                _validate_half_bst(child, min);
            }
        }
        else {
            // check right subtree
            _validate_half_bst(child, min);
        }
    }
}


static void __print_nodes(heap_node * node, int is_left, int depth) {
    
    if (is_left == 2) {
        printf("%*.sRoot (%p)\n", depth, "", node);
    }
    else if (is_left == 1) {
        printf("%*.sL: %lld (%p)\n", depth, "", node->key, node);
    }
    else {
        printf("%*.sR: %lld (%p)\n", depth, "", node->key, node);
    }

    heap_node * child = node->lchild;
    if (child != node) {
        __print_nodes(child, IS_LEFT_CHILD(child), depth + 1);
        child = RSIB(child);
        if (child != node) {
            __print_nodes(child, 0, depth + 1);
        }
    }
}


void print_heap(heap_t *h) {
    __print_nodes((heap_node *) h, 2, 0);
}


/*
 * validates the heap, aborting on failure and returning on success
 */
void heap_validate(heap_t *h) {


    if (h->root != (heap_node *) h) {
        heap_node * root = h->root;
        // root cannot have a right sibling
        HEAP_ASSERT(root->rsib == (heap_node *) (((uint64_t) h) | LEFT_CHILD));

        heap_node *root_child = root->lchild;
        if (root_child != root) {
            HEAP_ASSERT(IS_LEFT_CHILD(root_child));
            HEAP_ASSERT(RSIB(root_child) == root);
        }

        _validate_ptrs(h->root);
        _validate_half_bst(h->root, 0x8000000000000000L);
    }
}


