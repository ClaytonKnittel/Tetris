#ifndef _KEY_EVENT_H
#define _KEY_EVENT_H


// will not register beyond 32 key pressed in one frame (I think this is
// reasonable)
#define QUEUE_CAPACITY 32


// return code if the queue is empty
#define QUEUE_EMPTY 0

typedef struct key_event {

    int key, scancode, action, mods;
} key_event;


/*
 * implementation of a single-producer single-consumer queue for key
 * events
 */
typedef struct key_event_queue {
    key_event __m[QUEUE_CAPACITY];

    uint32_t __attribute__((aligned(8))) front;
    uint32_t __attribute__((aligned(8))) back;
} key_event_queue;



static void key_event_queue_init(key_event_queue *q) {
    q->front = 0;
    q->back  = 0;
}

/*
 * pushes event onto the queue. Can fail silently if the queue is full
 */
static void key_event_queue_push(key_event_queue *q, key_event *e) {
    uint64_t back = q->back;

    if (q->front == (back + 1) % QUEUE_CAPACITY) {
        // queue is full
        return;
    }

    q->__m[back] = *e;
    __atomic_store_n(&q->back, (back + 1) % QUEUE_CAPACITY, __ATOMIC_RELEASE);
}


/*
 * pops an event off the queue, populating e. If the queue was empty,
 * then 0 is returned, otherwise 1 is returned
 */
static int key_event_queue_pop(key_event_queue *q, key_event *e) {
    uint64_t slot = __atomic_load_n(&q->front, __ATOMIC_ACQUIRE);
    if (q->back == slot) {
        // queue is empty
        return 0;
    }

    *e = q->__m[slot];
    q->front = (slot + 1) % QUEUE_CAPACITY;
    return 1;
}


#endif /* _KEY_EVENT_H */
