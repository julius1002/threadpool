#include <sys/queue.h>

struct entry {
	STAILQ_ENTRY(entry) entries;
	void (*cb)(void *);
	void *arg;
};

struct threadpool_arg {
};

void init_threadpool(int num_threads, int queue_cap, void *arg);

void threadpool_enqueue(void *cb, void *arg);

struct entry *threadpool_dequeue();

void threadpool_wait();
