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



void key_event_queue_init(key_event_queue *q);

/*
 * pushes event onto the queue. Can fail silently if the queue is full
 */
void key_event_queue_push(key_event_queue *q, key_event *e);


/*
 * pops an event off the queue, populating e. If the queue was empty,
 * then 0 is returned, otherwise 1 is returned
 */
int key_event_queue_pop(key_event_queue *q, key_event *e);


#endif /* _KEY_EVENT_H */
