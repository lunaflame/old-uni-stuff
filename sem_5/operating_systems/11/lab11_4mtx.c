#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#define TIMES 10

pthread_mutex_t loop_mtx;
pthread_mutex_t childlimit_mtx;
pthread_mutex_t parentlimit_mtx;
pthread_mutex_t sync_mtx;


bool child_ran = false;

void child_op() {
	printf("Child print\n");
}

void parent_op() {
	printf("Parent print\n");
}

void* child(void* blah) {
	for (int i = 0; i < TIMES; i++) {
		pthread_mutex_lock(&parentlimit_mtx);
		if (i > 0) {
			pthread_mutex_unlock(&childlimit_mtx);
		}

		child_ran = true;
		pthread_mutex_unlock(&loop_mtx);
		pthread_mutex_lock(&sync_mtx);

		child_op();

		// pthread_mutex_lock(&childlimit_mtx);

		// Unblock parent
		pthread_mutex_unlock(&parentlimit_mtx);
		pthread_mutex_unlock(&sync_mtx);

		pthread_mutex_lock(&loop_mtx);
		pthread_mutex_lock(&childlimit_mtx);
		// Locked: childlimit
		// Parent: sync, parent
	}

	return NULL;
}

int main(int argc, char** argv) {
	pthread_t thread_id;

	pthread_mutex_init(&loop_mtx, NULL);
	pthread_mutex_init(&childlimit_mtx, NULL);
	pthread_mutex_init(&parentlimit_mtx, NULL);
	pthread_mutex_init(&sync_mtx, NULL);

	pthread_mutex_lock(&sync_mtx);

	int ok = pthread_create(&thread_id, NULL, &child, NULL);
	if (ok != 0) {
		perror("Failed to create a thread. ");
		return -1;
	}

	// lame but whatever
	while (!child_ran) {
		usleep(50);
	}

	// Locked: sync_mtx
	// Child has parentLimit, waits on sync

	for (int i = 0; i < TIMES; i++) {
		parent_op();

		pthread_mutex_lock(&loop_mtx);
		pthread_mutex_unlock(&loop_mtx);

		// Allow the child to print *once*
		pthread_mutex_lock(&childlimit_mtx);
		pthread_mutex_unlock(&sync_mtx);

		// Wait till the child prints and waits on childlimit_mtx
		pthread_mutex_lock(&parentlimit_mtx);
		pthread_mutex_unlock(&parentlimit_mtx);

		// Claim next print iteration and unlock child
		pthread_mutex_lock(&sync_mtx);
		pthread_mutex_unlock(&childlimit_mtx);

		// Locked: sync_mtx
		// Child may have no locks; this is why we have loop_mtx
	}

	pthread_mutex_unlock(&loop_mtx);
	pthread_exit(NULL);
}