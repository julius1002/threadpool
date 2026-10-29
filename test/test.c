#include <stdio.h>
#include <threadpool.h>
#include <pthread.h>

void cb(void *data) {
	printf("test %p\n", pthread_self());
	pthread_exit(NULL);
}

int main() {
	printf("hello world\n");
	init_threadpool(5, 10, NULL);
	threadpool_enqueue(&cb, NULL);
	threadpool_enqueue(&cb, NULL);
	threadpool_enqueue(&cb, NULL);
	threadpool_enqueue(&cb, NULL);
	threadpool_enqueue(&cb, NULL);
	threadpool_enqueue(&cb, NULL);
	threadpool_wait();

	return 0;
}
