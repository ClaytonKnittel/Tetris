
#include <stdint.h>

#include <key_event.h>


void key_event_queue_init(key_event_queue *q) {
    q->front = 0;
    q->back  = 0;
}

/*
 * pushes event onto the queue. Can fail silently if the queue is full
 */
void key_event_queue_push(key_event_queue *q, key_event *e) {
    uint32_t back = __atomic_load_n(&q->back, __ATOMIC_ACQUIRE);
    uint32_t next_back = (back + 1) % QUEUE_CAPACITY;

    if (q->front == next_back) {
        // queue is full
        return;
    }

    q->__m[back] = *e;
    __atomic_store_n(&q->back, next_back, __ATOMIC_RELEASE);
}


/*
 * pops an event off the queue, populating e. If the queue was empty,
 * then 0 is returned, otherwise 1 is returned
 */
int key_event_queue_pop(key_event_queue *q, key_event *e) {
    uint32_t slot = __atomic_load_n(&q->front, __ATOMIC_ACQUIRE);
    if (q->back == slot) {
        // queue is empty
        return 0;
    }

    *e = q->__m[slot];
    __atomic_store_n(&q->front, (slot + 1) % QUEUE_CAPACITY, __ATOMIC_RELEASE);
    return 1;
}


