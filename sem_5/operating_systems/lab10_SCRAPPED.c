#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define PHILO 2
#define DELAY 300000
#define FOOD 50

pthread_mutex_t forks[PHILO];
pthread_t phils[PHILO];
pthread_mutex_t foodlock;

int sleep_seconds = 0;

static int foodLeft = FOOD;

int takeFood(int id) {
	int myfood;

	// printf("locked f00d %d\n", id);
	pthread_mutex_lock(&foodlock);
	// printf("\tfood decr %d\n", id);
	if (foodLeft > 0) {
		foodLeft--;
	}
	myfood = foodLeft;
	int ok = pthread_mutex_unlock(&foodlock);
	// printf("(unlocked f00d) %d %d\n", ok, id);

	return myfood;
}

/*
int observe_food(int id) {
	// quantum food: it changes until you observe it
	printf("locked f00d2 %d\n", id);
	pthread_mutex_lock(&foodlock);
	int howmuch = foodLeft;
	int ok = pthread_mutex_unlock(&foodlock);
	printf("(unlocked f00d 2) %d %d\n", ok, id);

	return howmuch;
}
*/

void get_fork (int phil, int fork, char *hand) {
	pthread_mutex_lock(&forks[fork]);
}

void down_forks (int phil, int f1, int f2) {
	pthread_mutex_unlock(&forks[f1]);
	pthread_mutex_unlock(&forks[f2]);
}

void* philosopher (void *num) {
	int id;
	int left_fork, right_fork, f;

	id = (int)num;
	printf ("Philosopher %d sitting down to dinner.\n", id);
	bool flip = id % 2 == 1;
	right_fork = flip ? id + 1 : id;
	left_fork = flip ? id : id + 1;

	/* Wrap around the forks. */
	if (left_fork == PHILO)
		left_fork = 0;

	while ((f = takeFood(id))) {
		printf ("Philosopher %d: get dish %d.\n", id, f);
		get_fork (id, right_fork, "right");
		get_fork (id, left_fork, "left ");

		printf ("Philosopher %d: eating. Food left: %d\n", id, f);
		usleep (DELAY);
		down_forks (id, left_fork, right_fork);
	}

	printf ("Philosopher %d is done eating.\n", id);
	return (NULL);
}

int main(int argn, char **argv) {
	if (argn == 2)
		sleep_seconds = atoi (argv[1]);

	pthread_mutex_init (&foodlock, NULL);

	for (int i = 0; i < PHILO; i++)
		pthread_mutex_init (&forks[i], NULL);

	for (int i = 0; i < PHILO; i++)
		pthread_create (&phils[i], NULL, philosopher, (void *)i);

	for (int i = 0; i < PHILO; i++)
		pthread_join (phils[i], NULL);

	return 0;
}
