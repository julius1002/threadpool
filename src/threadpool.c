#include "threadpool.h"
#include <pthread.h>
#include "sem.h"
#include <stdlib.h>

STAILQ_HEAD(stailhead, entry) head;

void *thread_handler(void *arg) {
	while(1) {
		struct entry *entr = threadpool_dequeue();
		entr->cb(entr->arg);
		free(entr);
	}
}

struct threadpool {
	int num_threads;
	struct queue {
		my_sem_t taken_slots, free_slots;
		struct stailhead *headp;
		pthread_mutex_t mutex;
		struct stailhead head;
	} queue;
	pthread_t *threads;
} tp;

static void init_queue(int cap) {
	pthread_mutex_init(&tp.queue.mutex, NULL);
        STAILQ_INIT(&tp.queue.head);
	my_sem_init(&tp.queue.free_slots, cap);
	my_sem_init(&tp.queue.taken_slots, 0);
	tp.queue.head = (struct stailhead) STAILQ_HEAD_INITIALIZER(head);
}

void init_threadpool(int num_threads, int queue_cap, void *arg) {
	tp.num_threads = num_threads;
	init_queue(queue_cap);
        struct threadpool_arg *tp_arg = (struct threadpool_arg *) arg;
	tp.threads = malloc(sizeof(pthread_t) * num_threads);
	for(int i = 0; i < num_threads; i++) {
		pthread_create(&tp.threads[i], NULL, thread_handler, tp_arg);
	}
}

void threadpool_enqueue(void *cb, void *arg) {
	my_sem_wait(&tp.queue.free_slots);
	struct entry *item = malloc(sizeof(struct entry));
	item->cb = (void *) cb;
	item->arg = arg;
	if(0 > pthread_mutex_lock(&tp.queue.mutex)) {
		perror("pthread_mutex_lock");
	}
	STAILQ_INSERT_HEAD(&tp.queue.head, item, entries);
	if(0 > pthread_mutex_unlock(&tp.queue.mutex)) {
		perror("pthread_mutex_unlock");
	}
	my_sem_post(&tp.queue.taken_slots);
}

struct entry *threadpool_dequeue() {
	my_sem_wait(&tp.queue.taken_slots);
	if(0 > pthread_mutex_lock(&tp.queue.mutex)) {
		perror("pthread_mutex_lock");
	}
	struct entry *entr = STAILQ_FIRST(&tp.queue.head);
	STAILQ_REMOVE_HEAD(&tp.queue.head, entries);
	if(0 > pthread_mutex_unlock(&tp.queue.mutex)) {
		perror("pthread_mutex_unlock");
	}
	my_sem_post(&tp.queue.free_slots);
	return entr;
}

void threadpool_wait() {
	for(int i = 0; i < tp.num_threads; i++) {
		pthread_join(tp.threads[i], NULL);
	}
}
